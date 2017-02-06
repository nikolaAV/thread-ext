#ifndef _THREAD_EX_MOVE_INCLUDED_
#define _THREAD_EX_MOVE_INCLUDED_

/**
	\file 		te_move.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2012-02-10
	\copyright 	GNU Public License.
*/


#include "te_compiler_warning_suppress.h"
#include <utility>
#include <stack>
#include <queue>
#include "te_compiler_warning_rollback.h"
#include "te_compiler.h"
#include "te_first_element.h"

namespace thread_ex
{

/**
   \brief Moves the elements in the container into the range beginning at 'out' of Output iterator.

   \param[in,out] 'c' - the container of elements to move
   \param[out]    'out' - the beginning of the destination range
   \return        Output iterator to the element past the last element moved
*/

   template <typename STDCONTAINER, typename OutputIterator>
   inline
   OutputIterator
   move(STDCONTAINER& c, OutputIterator out)
   {
      return  std::move(c.begin(), c.end(), out);
   }

   namespace container_adapter
   { // std::queue<>, std::stack<>

      template <typename STDADAPTER, typename OutputIterator>
      inline
      OutputIterator
      move(STDADAPTER& c, OutputIterator out)
      {
         for (; !c.empty(); c.pop())
         {
            *out = std::move(first::get(c));
            ++out;
         }
         return out;
      }

   } // namespace container_adapter

   template <typename T, typename T2, typename OutputIterator>
   inline
   OutputIterator
   move(std::stack<T, T2>& c, OutputIterator out)
   {
      return container_adapter::move(c,out);
   }

   template <typename T, typename T2, typename OutputIterator>
   inline
      OutputIterator
      move(std::queue<T, T2>& c, OutputIterator out)
   {
      return container_adapter::move(c, out);
   }

} // namespace thread_ex

#endif // _THREAD_EX_MOVE_INCLUDED_

