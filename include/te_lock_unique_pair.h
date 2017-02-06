#ifndef _THREAD_EX_LOCK_UNIQUE_PAIR_INCLUDED_
#define _THREAD_EX_LOCK_UNIQUE_PAIR_INCLUDED_

/**
	\file 		te_lock_unique_pair.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2012-02-10
	\copyright 	GNU Public License.
*/


#include "te_compiler_warning_suppress.h"
#include <mutex>
#include "te_compiler_warning_rollback.h"
#include "te_compiler.h"
#include "te_unique_pair.h"

/**
   \brief  

   The common advice for avoiding deadlock is to always lock the two mutexes in the same order:
   if you always lock mutex A before mutex B, then you’ll never deadlock.
   Sometimes this is straightforward, because the mutexes are serving different purposes,
   but other times it’s not so simple, such as when the mutexes are each protecting a separate instance of the same class.
   Consider, for example, an operation exchanges data (initiated by different threads) between two instances of the same class; 
   in order to ensure that the data is exchanged correctly, without being affected by concurrent modifications,
   the mutexes on both instances must be locked.
   However, if a fixed order is chosen (for example, the mutex for the instance supplied as the first parameter, then the
   mutex for the instance supplied as the second parameter), this can backfire: 
   all it takes is for two threads to try to exchange data between the same two instances with the parameters swapped, and you have deadlock!
   Thankfully, the C++ Standard Library has a cure for this in the form of std::lock —
   a function that can lock two or more mutexes at once without risk of deadlock.

   \remark "C++ Concurrency in Action", Anthony Williams, chapter 3.2.4, page 47
*/



namespace thread_ex
{

/**
   \brief wrapper of std::lock to simplify client code invocation 

   calling of thread_ex::lock_unique_pair() locks the two mutexes, and two std::unique_lock instances are constructed, one for each mutex.
   These two instances are packed up (by means 'move' semantics) into std::pair instance and returned to the client 

   \param[in]  reference to the first mutex of type A 
   \param[in]  reference to the second mutex of type B 
   \note       mutexes might be of the same type (type A == type B)
   \return     instance of unique_pair<unique_lock<mutex A>, unique_lock<mutex B>>

   \example test_lock_unique_pair.cpp
 */

template <typename MUTEX_A_T, typename MUTEX_B_T>
unique_pair<std::unique_lock<MUTEX_A_T>, std::unique_lock<MUTEX_B_T> > lock_unique_pair(MUTEX_A_T& first, MUTEX_B_T& second)
{
   typedef std::unique_lock<MUTEX_A_T>                unique_lock_a;
   typedef std::unique_lock<MUTEX_B_T>                unique_lock_b;
   typedef unique_pair<unique_lock_a,unique_lock_b>   unique_pair;
 
   unique_pair pair(unique_lock_a(first,std::defer_lock),unique_lock_b(second,std::defer_lock));
   std::lock(pair.first,pair.second);
   return pair;
}


} // namespace thread_ex

#endif //_THREAD_EX_LOCK_UNIQUE_PAIR_INCLUDED_

