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
#include "te_compiler_warning_rollback.h"
#include "te_compiler.h"

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


   std::atomic_bool done_;   
};


} // namespace thread_ex

#endif //_THREAD_EX_THREAD_POOL_INCLUDED_

