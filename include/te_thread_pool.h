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
   \brief

   \remark 
*/

namespace thread_ex
{

/**

   \example test_thread_pool.cpp
*/

namespace tpis // thread_pool_internals
{
   class movable_function_body
   {
      struct void_signature
      {
         virtual void call() = 0;
         virtual ~void_signature() {}
      };

      template <typename Callable>
      struct void_signature_impl : void_signature
      {
         void call() override { f_(); }
         void_signature_impl(Callable&& f) : f_(std::move(f)) {}
         void_signature_impl& operator=(Callable&& f) { f_(std::move(f)); return *this; }

      private:
         Callable f_;
      };

   public:
      void operator()() { f_->call(); }
      template <typename Function>
      movable_function_body(Function&& f) : f_{ new void_signature_impl<Function>(std::move(f)) } {}

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

class thread_pool
{
   using movable_function_body   = tpis::movable_function_body;
   using task_queue_type         = threadsafe_queue<movable_function_body>;
   using thread_container_type   = std::vector<joined_thread>;

public:
   const struct deferred_start_type {}    deferred_start{};
   const struct async_execution_type {}   async_exec{};
   const struct sync_execution_type {}    sync_exec{};

   thread_pool();
   explicit thread_pool(size_t);
   explicit thread_pool(const deferred_start_type&);
   ~thread_pool();

   size_t   thread_count() const noexcept;
   void     start(size_t = std::thread::hardware_concurrency());
   void     stop(const async_execution_type&);
   void     stop(const sync_execution_type&);

   template <typename Function, typename... Args>
   std::future<std::result_of_t<std::decay_t<Function>(std::decay_t<Args>...)>>
   async(Function&&,Args&&...);

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
   stop(async_exec);
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
   try
   {
      for(size_t i = 0; i < thread_count_; ++i)
         threads_.push_back(std::thread{&thread_pool::listening_thread,this});
   }
   catch(...)
   {
      done_ = true;
      throw;
   }
   assert(n==threads_.size());
}

inline
void thread_pool::stop(const async_execution_type&)
{
   assert(!done_ && "double stop not allowed");
   done_ = true;
   for(size_t i=0; i<thread_count_; ++i)
      async([](){});      
}

inline
void thread_pool::stop(const sync_execution_type&)
{
   stop(async_exec);
   thread_container_type{}.swap(threads_); 
}

inline
void thread_pool::listening_thread()
{
   while(!done_)
   {
      movable_function_body f;
      tasks_.wait_pop(f);
      f();   
   }
} 

template <typename Function, typename... Args>
inline
std::future<std::result_of_t<std::decay_t<Function>(std::decay_t<Args>...)>>
thread_pool::async(Function&& f,Args&&... args)
{
   using result_type = std::result_of_t<std::decay_t<Function>(std::decay_t<Args>...)>;

   std::packaged_task<result_type(Args&&...)> pack {std::forward<Function>(f)};
   auto future = pack.get_future(); 
   auto lambda = [p=std::move(pack),a=std::make_tuple(std::forward<Args>(args)...)]() mutable { 
      apply(std::move(p),std::move(a)); 
   };
   tasks_.push(std::move(lambda));
   return future;
}


} // namespace thread_ex

#endif //_THREAD_EX_THREAD_POOL_INCLUDED_

