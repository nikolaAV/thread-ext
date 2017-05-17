#ifndef _THREAD_EX_TOP_POP_INCLUDED_
#define _THREAD_EX_TOP_POP_INCLUDED_

/**
	\file 		te_top_pop.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2012-02-10
	\copyright 	GNU Public License.
*/


#include "te_compiler_warning_suppress.h"
#include <memory>
#include <mutex>
#include <queue>
#include <stack>
#include "te_compiler_warning_rollback.h"
#include "te_compiler.h"

#include "te_empty_error.h"
#include "te_first_element.h"
#include "te_last_element.h"
#include "te_block_lock.h"


/**
	\brief  	combines two operations (top/front & pop) of std::container adapter (stack/queue) as singular action 

   Original std::stack, std::queue interfaces contain two separate methods to work with the first element of the top
      - top returns a reference to the top element in the stack/queue. 
      - pop/front removes the element on top/beginning of the stack/queue
   Obviously it was designed in such manner to solve Cargill’s exception problem
   http://ptgmedia.pearsoncmg.com/images/020163371x/supplements/Exception_Handling_Article.html

   This problem was dealt with fairly comprehensively from an exception-safety point of view by Herb Sutter,
   http://www.gotw.ca/gotw/008.htm
   but the potential for race conditions brings something new to the mix.
   This calls for a more radical change to the interface, one that combines the calls to
   top/front() and pop() under the protection of the mutex

   \remark "C++ Concurrency in Action", Anthony Williams, chapter 3.2.3, page 43 

*/


namespace thread_ex
{

/**
   @param STDCONTAINERADAPTER is type one of std::stack and std::queue 
*/

   struct first_element
   {
      template<typename STDCONTAINERADAPTER>
      static
      bool
      pop(STDCONTAINERADAPTER& c, typename STDCONTAINERADAPTER::value_type& out)  // returns 'false' if the container is empty
      {
         if (c.empty()) return false;
         out = first::get(c);
         c.pop();
         return true;
      }
   };

   struct last_element
   {
      template<typename STDCONTAINERADAPTER>
      static
      inline
      bool
      pop(STDCONTAINERADAPTER& c, typename STDCONTAINERADAPTER::value_type& out)  // returns 'false' if the container is empty
      {
         if (c.empty()) return false;
         out = last::get(c);
         c.pop_back();
         return true;
      }
   };

template<typename ELEMENT_T, typename STDCONTAINERADAPTER>
inline
bool
pop(STDCONTAINERADAPTER& c, typename STDCONTAINERADAPTER::value_type& out)  // returns 'false' if the container is empty
{
   return ELEMENT_T::pop(c,out);
}

template<typename ELEMENT_T, typename STDCONTAINERADAPTER, typename MUTEX>
inline
bool
pop(const std::nothrow_t, STDCONTAINERADAPTER& c, typename STDCONTAINERADAPTER::value_type& out, MUTEX& m)
{
   bool res{ false };
   block::lock(m, [&] {
      res = pop<ELEMENT_T>(c, out);
   });
   return res;
}

template<typename ELEMENT_T, typename STDCONTAINERADAPTER, typename MUTEX>
inline
void
pop(STDCONTAINERADAPTER& c, typename STDCONTAINERADAPTER::value_type& out, MUTEX& m) // throw (empty_error)
{
   if (!pop<ELEMENT_T>(std::nothrow, c, out, m))
      throw empty_error{};
}

template<typename ELEMENT_T, typename STDCONTAINERADAPTER, typename MUTEX>
inline
std::unique_ptr<typename STDCONTAINERADAPTER::value_type>
pop(STDCONTAINERADAPTER& c, MUTEX& m)                             // returns nullptr is a container is empty
{
   using value_type = typename STDCONTAINERADAPTER::value_type;
   auto out = make_unique<value_type>(value_type{});
   if (pop<ELEMENT_T>(std::nothrow, c, *out, m))
      return out;
   return{ nullptr };
}


} // namespace thread_ex

#endif //_THREAD_EX_TOP_POP_INCLUDED_

