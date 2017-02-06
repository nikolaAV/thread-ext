#ifndef _THREAD_EX_UNIQUE_PAIR_INCLUDED_
#define _THREAD_EX_UNIQUE_PAIR_INCLUDED_

/**
	\file 		te_unique_pair.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2012-02-10
	\copyright 	GNU Public License.
*/


#include "te_compiler_warning_suppress.h"
#include <utility>
#include <type_traits>
#include "te_compiler_warning_rollback.h"
#include "te_compiler.h"

/**
   \brief

   \remark 
*/

namespace thread_ex
{

/**
   \brief specific variation of std::pair restricted with move semantins only

   For class unique_pair<T1,T2> the move constructor is NOT an optimization, 
   it provides only a move constructor to emphasize that copy constructor doesn’t make sense. 
   The whole point of unique_pair<> is that each 'non-null' pair-instance is the one and only owner to its first and second sub-objects,
   so a copy constructor makes no sense. However, a move constructor allows ownership of the sub-objects (first,second) to be
   transferred between instances and permits unique_pair<> to be used as a function return value — the internals are moved rather than copied.

   \example test_unique_pair.cpp
 */

template <typename T1, typename T2>
struct unique_pair : private std::pair<T1,T2>
{
   typedef std::pair<T1,T2>   base_t;
   typedef unique_pair<T1,T2> this_t;

   using base_t::first;
   using base_t::second;

   unique_pair()                          : base_t() {}
   unique_pair(const T1& f, const T2& s)  : base_t(f,s) {}
   unique_pair(T1&& f, T2&& s)            : base_t(std::move(f),std::move(s)) {}
    
   unique_pair(const this_t&)             = delete;
   unique_pair(this_t&& rhd)              : base_t(std::move(static_cast<base_t&>(rhd))) {}

   unique_pair& operator=(const this_t&)  = delete;
   unique_pair& operator=(this_t&& rhd)
   { 
      static_cast<base_t&>(*this) = std::move(static_cast<base_t&>(rhd));
      return *this; 
   }

   void swap(unique_pair<T1,T2>& other)
   {
      static_cast<base_t&>(*this).swap(static_cast<base_t&>(other)); 
   }
};

template<typename T1, typename T2>
void swap(unique_pair<T1,T2>& left, unique_pair<T1,T2>& right)
{
   left.swap(right);
}

template <typename T1, typename T2>
// decltype(auto) make_unique_pair(T1&& first, T2&& second) <-- not all compiler are so modern to accept 'decltype(auto)'
unique_pair<remove_reference_t<T1>, remove_reference_t<T2>> make_unique_pair(T1&& first, T2&& second)
{
   return unique_pair<remove_reference_t<T1>, remove_reference_t<T2>>(std::forward<T1>(first), std::forward<T2>(second));
}


} // namespace thread_ex

#endif //_THREAD_EX_UNIQUE_PAIR_INCLUDED_

