#ifndef _THREAD_EX_BLOCK_LOCK_INCLUDED_
#define _THREAD_EX_BLOCK_LOCK_INCLUDED_

/**
	\file 		te_block_lock.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2012-02-10
	\copyright 	GNU Public License.
*/


#include <mutex>
#include "te_lock_unique_pair.h"

/**
   \brief lock (mutex) {...} block constuction like a language feature  

   The lock fubction call marks a statement block (see lock statement in C#) as a critical section by obtaining the mutual-exclusion lock for a given object, 
   executing a statement, and then releasing the lock.

   \remark https://herbsutter.com/elements-of-modern-c-style/
           https://msdn.microsoft.com/en-us/library/c5kehkcz.aspx
*/

namespace thread_ex
{

/**
   Example of usage:

   // C#
               lock( mut_x ) {
               ... use x ...
               }

   // C++11 without lambdas: already nice, and more flexible (e.g., can use timeouts, other options)
               {
               lock_guard<mutex> hold { mut_x };
               ... use x ...
               }

   // C++11 with lambdas, and a helper algorithm: C# syntax in C++
   // 'lock' definition see below in 'block' section/namespace
               lock( mut_x, [&]{
               ... use x ...
               });

*/

#include "te_compiler_warning_suppress.h"

namespace block
{

   template <typename MUTEX, typename F>
   inline void lock(MUTEX& m, F f)
   {
      std::lock_guard<MUTEX> hold{ m };
      f();
   }

   template <typename MUTEX1, typename MUTEX2, typename F>
   inline void lock(MUTEX1& m1, MUTEX2& m2, F f)
   {
      auto hold { thread_ex::lock_unique_pair(m1, m2) };
      f();
   }

}  // namespace block

#include "te_compiler_warning_rollback.h"

} // namespace thread_ex

#endif // _THREAD_EX_BLOCK_LOCK_INCLUDED_

