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

template<typename STDCONTAINERADAPTER>
inline
bool
top_pop(STDCONTAINERADAPTER& c, typename STDCONTAINERADAPTER::value_type& out, const std::nothrow_t)  // returns 'false' if the container is empty
{
   if (c.empty()) return false;
   out = first::get(c);
   c.pop();
   return true;
}

template<typename STDCONTAINERADAPTER, typename MUTEX>
inline
void
top_pop(STDCONTAINERADAPTER& c, typename STDCONTAINERADAPTER::value_type& out, MUTEX& m) // throw (empty_error)
{
   block::lock(m, [&]{
      if(!top_pop(c,out,std::nothrow))
         throw empty_error{};
   });
}

template<typename STDCONTAINERADAPTER, typename MUTEX>
inline
bool
top_pop(const std::nothrow_t n, STDCONTAINERADAPTER& c, typename STDCONTAINERADAPTER::value_type& out, MUTEX& m)
{
   bool res {false};
   block::lock(m, [&] {
      res = top_pop(c, out, n);
   });
   return res;
}

template<typename STDCONTAINERADAPTER, typename MUTEX>
inline
std::unique_ptr<typename STDCONTAINERADAPTER::value_type>
top_pop(STDCONTAINERADAPTER& c, MUTEX& m)                             // throw (empty_error)
{
   using value_type = typename STDCONTAINERADAPTER::value_type;
   auto out = make_unique<value_type>(value_type{});
   top_pop(c,*out,m);
   return out;
}

template<typename STDCONTAINERADAPTER, typename MUTEX>
inline
std::unique_ptr<typename STDCONTAINERADAPTER::value_type>
top_pop(std::nothrow_t n, STDCONTAINERADAPTER& c, MUTEX& m)
{
   using value_type = typename STDCONTAINERADAPTER::value_type;
   auto out = make_unique<value_type>(value_type{});
   if(top_pop(n, c, *out, m))
      return out;
   return {nullptr};
}


} // namespace thread_ex

#endif //_THREAD_EX_TOP_POP_INCLUDED_

