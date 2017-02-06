#ifndef _THREAD_EX_RUNTIME_CONCURRENCY_INCLUDED_
#define _THREAD_EX_RUNTIME_CONCURRENCY_INCLUDED_

/**
	\file 		te_runtime_concurrency.h
	\brief  	some usefull thread primitives (extensions) which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2012-02-10
	\copyright 	GNU Public License.
*/

#include "te_compiler_warning_suppress.h"
#include <thread>
#include <utility>
#include "te_compiler_warning_rollback.h"
#include "te_compiler.h"



namespace thread_ex
{
   /**
   \brief RAII object which makes std::thread unjoinable all paths
   \remark "Effective Modern C++" by Scott Meyers, item:37, page 250

   Every std::thread object is in one of two states: joinable or unjoinable. A joinable
   std::thread corresponds to an underlying asynchronous thread of execution that is
   or could be running. A std::thread corresponding to an underlying thread that’s
   blocked or waiting to be scheduled is joinable, for example. std::thread objects corresponding
   to underlying threads that have run to completion are also considered
   joinable. An unjoinable std::thread is what you’d expect: a std::thread that’s not joinable.
   Unjoinable std::thread objects include:
   -  Default-constructed std::threads.
   -  std::thread objects that have been moved from.
   -  std::threads that have been joined (after join() invokation)
   -  std::threads that have been detached (after detach() invokation)

   One reason a std::thread’s joinability is important is that if the destructor for a
   joinable thread is invoked, execution of the program is terminated
   */

namespace details_
{
   struct join_policy
   {
      void operator()(std::thread& t) { t.join(); }
   };
   struct detach_policy
   {
      void operator()(std::thread& t) { t.detach(); }
   };
}  // details_

template < typename POLICY >
class thread_unjoinable
{
   std::thread thread_;

public:
   thread_unjoinable(std::thread&& t) : thread_(std::move(t)) {}
   thread_unjoinable(thread_unjoinable&&) = default;
   thread_unjoinable& operator=(thread_unjoinable&&) = default;
   ~thread_unjoinable()
   {
      if(thread_.joinable())
         POLICY()(thread_);
   }
   std::thread& get() { return thread_; }
};

using joined_thread     = thread_unjoinable<details_::join_policy>;
using detached_thread   = thread_unjoinable<details_::detach_policy>;

} // namespace thread_ex

#endif //_THREAD_EX_RUNTIME_CONCURRENCY_INCLUDED_

