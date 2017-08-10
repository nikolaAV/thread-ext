#ifndef _THREAD_EX_THREAD_POOL_INCLUDED_
#define _THREAD_EX_THREAD_POOL_INCLUDED_

/**
	\file 	te_thread_pool.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2017-08-03
	\copyright 	GNU Public License.
*/


#include "te_compiler_warning_suppress.h"
#include <atomic>
#include <utility>
#include <memory>
#include <vector>
#include <thread>
#include <future>
#include <cassert>
#include <type_traits>
#include <tuple>
#include "te_compiler_warning_rollback.h"
#include "te_compiler.h"
#include "te_container.h"
#include "te_thread_unjoinable.h"

/**
   \brief a thread pool is a fixed number of worker threads (typically the same number as the value returned by std::thread::hardware_concurrency()) that process work.

   On most systems, it’s impractical to have a separate thread for every task that can potentially be done in parallel with other tasks, 
   but there is  still need to take advantage of the available concurrency where possible. 
   A thread pool allows you to accomplish this; 
   tasks that can be executed concurrently are submitted to the pool, 
   which puts them on a queue of pending work. 
   Each task is then taken from the queue by one of the worker threads, 
   which executes the task before looping back to take another from the queue.

   \remark "C++ Concurrency in Action", Anthony Williams, chapter 9.1.2, page 277
   \example unit/test_thread_pool.cpp
*/

namespace thread_ex
{

namespace tpis // thread_pool_internals
{
   class movable_function_body
   {
      struct void_signature
      {
         virtual void call() = 0;
         virtual bool exit_marker() const noexcept = 0;
         virtual ~void_signature() {}
      };

      template <typename Callable>
         // where Callable is an instance of class template std::packaged_task<...>
      struct void_signature_impl : void_signature
      {
         bool exit_marker() const noexcept override { return false; }
         void call() override { f_(); }

         void_signature_impl(Callable&& f) : f_(std::move(f)) {}
         void_signature_impl& operator=(Callable&& f) { f_(std::move(f)); return *this; }
         void_signature_impl(const void_signature_impl&)             = delete;
         void_signature_impl& operator=(const void_signature_impl&)  = delete;

      private:
         Callable f_;
      };

      struct exit_signature_impl : void_signature
      {
         bool exit_marker() const noexcept override { return true; }
         void call() override {}
      };

   public:
      template <typename Callable>
      using package_task_type = void_signature_impl<Callable>;
      using exit_task_type    = exit_signature_impl;

      /**
         \retval
            'true'  - function completed successfully. The current thread is still in active state & continuous listening incoming tasks 
            'false' - that was the last function call. The listening thread must be terminated.
      */
      bool operator()() { f_->call(); return !f_->exit_marker(); }
      template <typename Function>
      movable_function_body(Function&& f)       : f_{ new package_task_type<Function>(std::move(f)) } {}
      movable_function_body(exit_task_type&&)   : f_{ new exit_task_type{} } {}

         // movable only
      movable_function_body()                                         = default;
      movable_function_body(movable_function_body&&)                  = default;
      movable_function_body& operator=(movable_function_body&&)       = default;
      movable_function_body(const movable_function_body&)             = delete;
      movable_function_body& operator=(const movable_function_body&)  = delete;

   private:
      std::unique_ptr<void_signature> f_;
   };
}  // end of 'thread_pool_internals'

/**
   The implementation below allows you be in waiting state to ensure the overall submitted task was complete before returning to the caller.
   By moving std::future-driven technique into the thread_pool itself, you can wait for the task directly.
   You can have the submit() function return a task handle of some description that you can then use to wait for the task to complete. 
   This task handle would wrap the use of condition variables or futures, thus simplifying the code that uses the thread pool.
   Any task you want to submit is 'f' - a function-delegate or object of 'Callable' concept with 'args...' arbitrary parameters to pass to 'f'.  
*/

class thread_pool
{
   using movable_function_body   = tpis::movable_function_body;
   using task_queue_type         = threadsafe_queue<movable_function_body>;
   using thread_container_type   = std::vector<joined_thread>;
   using exit_task_type          = typename movable_function_body::exit_task_type;

public:
   const struct deferred_start_type {}    deferred_start{};

   thread_pool();
   explicit thread_pool(size_t);
   explicit thread_pool(const deferred_start_type&);
   thread_pool(const thread_pool&)              = delete;
   thread_pool& operator=(const thread_pool&)   = delete;
   ~thread_pool();

   size_t   thread_count() const noexcept;
   void     start(size_t = std::thread::hardware_concurrency());
      // graceful completion. All pending tasks will be completed before the stop
   void     stop();
      // stop working as soon as possible. That means some tasks in the queue might be unprocessed
   void     terminate(); 

   /**
      \brief 'submit' This is very similar to the way that the std::async - based.
      \retval std::future<...> of behaviour which conforms to the return by std::packaged_task 
   */
   template <typename Function, typename... Args>
   std::future<std::result_of_t<std::decay_t<Function>(std::decay_t<Args>...)>>
   submit(Function&&,Args&&...);

private:
   void     listening_thread(); 

private:
   size_t                  thread_count_  {0} ;
   std::atomic_bool        done_          {false};   
   task_queue_type         tasks_;
   thread_container_type   threads_;
};

inline 
thread_pool::thread_pool()
{
   start();
}

inline 
thread_pool::thread_pool(size_t n)
{
   start(n);
}

inline 
thread_pool::thread_pool(const deferred_start_type&)
{
}

inline
thread_pool::~thread_pool()
{
   stop();
}

inline
size_t   thread_pool::thread_count() const noexcept
{
   return thread_count_;
}

inline 
void thread_pool::start(size_t n)
{
   assert(n && "thread count must be greater zero");
   assert(threads_.empty() && "'start' can be called once");

   thread_count_ = n;

#ifdef _MSC_VER
   #pragma warning( push )
   #pragma warning( disable: 4571 ) // Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
#endif
   try
   {
      for(size_t i = 0; i < thread_count_; ++i)
         threads_.push_back(std::thread{&thread_pool::listening_thread,this});
   }
   catch(...)
   {
      terminate();
      throw;
   }
#ifdef _MSC_VER
   #pragma warning( pop )
#endif

   assert(n==threads_.size());
}

inline
void thread_pool::stop()
{
   for(size_t i=0; i<thread_count_; ++i)
      tasks_.push(exit_task_type{});
   thread_container_type{}.swap(threads_); 
}

inline
void thread_pool::terminate()
{
   done_ = true;
   stop();
}

inline
void thread_pool::listening_thread()
{
   while(!done_)
   {
      movable_function_body f;
      tasks_.wait_pop(f);
      if(!f())
         break;   
   }
} 

template <typename Function, typename... Args>
inline
std::future<std::result_of_t<std::decay_t<Function>(std::decay_t<Args>...)>>
thread_pool::submit(Function&& f,Args&&... args)
{
   using result_type = std::result_of_t<std::decay_t<Function>(std::decay_t<Args>...)>;

   std::packaged_task<result_type(Args&&...)> pack {std::forward<Function>(f)};
   auto future = pack.get_future(); 

#ifdef _MSC_VER
   #pragma warning( push )
   #pragma warning( disable: 4625 ) // '<lambda_...>': copy constructor was implicitly defined as deleted
#endif

   auto lambda = [p=std::move(pack),a=std::make_tuple(std::forward<Args>(args)...)]() mutable { 
      apply(std::move(p),std::move(a)); 
   };

#ifdef _MSC_VER
   #pragma warning( pop )
#endif

   tasks_.push(std::move(lambda));
   return future;
}


} // namespace thread_ex

#endif //_THREAD_EX_THREAD_POOL_INCLUDED_

