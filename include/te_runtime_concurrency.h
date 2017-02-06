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
#include <cassert>
#include <algorithm>
#include "te_compiler_warning_rollback.h"
#include "te_compiler.h"


/**
   \brief Choosing the number of threads at runtime 

   One feature of the C++ Standard Library that helps here is std::thread::hardware_concurrency(). 
   This function returns an indication of the number of threads that can TRULY run concurrently for a given execution of a program. 
   On a multicore system it might be the number of CPU cores, for example. 
   This is only a hint, and the function might return 0 if this information is not available,
   but it can be a useful guide for splitting a task among threads.

   \remark "C++ Concurrency in Action", Anthony Williams, chapter 2.4, page 28
*/

namespace thread_ex
{

/**
   The number of threads to run is the minimum of calculated maximum (total_num/min_num) and
   the number of hardware threads. You don’t want to run more threads than the
   hardware can support (which is called oversubscription), because the context switching
   will mean that more threads will decrease the performance

   @param[in] total_num volume of work in number of elements that should be processed.
   @param[in] min_num   minimum number of elements per thread in order to avoid the overhead of too many threads.

    \remark 
   "Oversubscription: a Classic Parallel Performance Problem"
   http://blogs.msdn.com/b/visualizeparallel/archive/2009/12/01/oversubscription-a-classic-parallel-performance-problem.aspx
    
   \note
   Using runtime_concurrency() directly requires care;
   your code doesn’t take into account any of the other threads that are running on the system unless you explicitly share that information.
   In the worst case, if multiple threads call a function that uses runtime_concurrency() for scaling at the same time,there will be huge oversubscription. 
   std::async() avoids this problem because the library is aware of all calls and can schedule appropriately. 
   Careful use of thread pools can also avoid this problem.
*/
inline size_t runtime_concurrency(const size_t total_num, const size_t min_num)
{
   assert(min_num);

   const size_t max_num       = (total_num+min_num-1)/min_num;
   const size_t hardware_num  = std::thread::hardware_concurrency();
   assert(hardware_num);

   return std::min(hardware_num?hardware_num:1,max_num);
}

} // namespace thread_ex

#endif //_THREAD_EX_RUNTIME_CONCURRENCY_INCLUDED_

