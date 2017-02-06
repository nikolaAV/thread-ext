#ifndef _THREAD_EX_FIRST_ELEMENT_INCLUDED_
#define _THREAD_EX_FIRST_ELEMENT_INCLUDED_

/**
	\file 		te_first_element.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2012-02-10
	\copyright 	GNU Public License.
*/


#include "te_compiler_warning_suppress.h"
#include <stack>
#include <queue>
#include "te_compiler_warning_rollback.h"


namespace thread_ex
{

namespace first
{
   template <typename VALUE_TYPE, typename CONTAINER_TYPE>
   inline
   typename std::queue<VALUE_TYPE, CONTAINER_TYPE>::reference 
   get(std::queue<VALUE_TYPE, CONTAINER_TYPE>& q)
   {
      return q.front();
   }

   template <typename VALUE_TYPE, typename CONTAINER_TYPE>
   inline
   typename std::queue<VALUE_TYPE, CONTAINER_TYPE>::const_reference 
   get(const std::queue<VALUE_TYPE, CONTAINER_TYPE>& q)
   {
      return q.front();
   }

   template <typename VALUE_TYPE, typename CONTAINER_TYPE>
   inline
   typename std::stack<VALUE_TYPE, CONTAINER_TYPE>::reference 
   get(std::stack<VALUE_TYPE, CONTAINER_TYPE>& s)
   {
      return s.top();
   }

   template <typename VALUE_TYPE, typename CONTAINER_TYPE>
   inline
   typename std::stack<VALUE_TYPE, CONTAINER_TYPE>::const_reference 
   get(const std::stack<VALUE_TYPE, CONTAINER_TYPE>& s)
   {
      return s.top();
   }

} // end of namespace 'first'

} // namespace thread_ex

#endif //_THREAD_EX_FIRST_ELEMENT_INCLUDED_

