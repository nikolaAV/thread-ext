#ifndef _THREAD_EX_LAST_ELEMENT_INCLUDED_
#define _THREAD_EX_LAST_ELEMENT_INCLUDED_

/**
	\file 	te_last_element.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2017-05-14
	\copyright 	GNU Public License.
*/


#include "te_compiler_warning_suppress.h"
#include <stack>
#include <queue>
#include <vector>
#include <list>
#include "te_compiler_warning_rollback.h"

namespace thread_ex
{

namespace last
{
   template <typename VALUE_TYPE, typename CONTAINER_TYPE>
   inline
   typename std::queue<VALUE_TYPE, CONTAINER_TYPE>::reference 
   get(std::queue<VALUE_TYPE, CONTAINER_TYPE>& q)
   {
      return q.back();
   }

   template <typename VALUE_TYPE, typename CONTAINER_TYPE>
   inline
   typename std::queue<VALUE_TYPE, CONTAINER_TYPE>::const_reference 
   get(const std::queue<VALUE_TYPE, CONTAINER_TYPE>& q)
   {
      return q.back();
   }

   template <typename VALUE_TYPE, typename CONTAINER_TYPE>
   inline
   typename std::vector<VALUE_TYPE, CONTAINER_TYPE>::reference
   get(std::vector<VALUE_TYPE, CONTAINER_TYPE>& v)
   {
      return v.back();
   }

   template <typename VALUE_TYPE, typename CONTAINER_TYPE>
   inline
   typename std::vector<VALUE_TYPE, CONTAINER_TYPE>::const_reference
   get(const std::vector<VALUE_TYPE, CONTAINER_TYPE>& v)
   {
      return v.back();
   }

   template <typename VALUE_TYPE, typename CONTAINER_TYPE>
   inline
   typename std::list<VALUE_TYPE, CONTAINER_TYPE>::reference
   get(std::list<VALUE_TYPE, CONTAINER_TYPE>& l)
   {
      return l.back();
   }

   template <typename VALUE_TYPE, typename CONTAINER_TYPE>
   inline
   typename std::list<VALUE_TYPE, CONTAINER_TYPE>::const_reference
   get(const std::list<VALUE_TYPE, CONTAINER_TYPE>& l)
   {
      return l.back();
   }

} // end of namespace 'last'

} // namespace thread_ex

#endif //_THREAD_EX_LAST_ELEMENT_INCLUDED_

