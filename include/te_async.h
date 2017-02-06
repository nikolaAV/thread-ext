#ifndef _THREAD_EX_ASYNC_INCLUDED_
#define _THREAD_EX_ASYNC_INCLUDED_

/**
	\file 		te_async.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2012-02-10
	\copyright 	GNU Public License.
*/


#include "te_compiler_warning_suppress.h"
#include <future>
#include <type_traits>
#include <utility>
#include "te_compiler_warning_rollback.h"
#include "te_compiler.h"


/**
   \brief a function that acts like std::async, but that automatically uses std::launch::async as the launch policy 
   \remark "Effective Modern C++" by Scott Meyers, item:36, page 249
   \note
   http://en.cppreference.com/w/cpp/thread/async
   http://en.cppreference.com/w/cpp/thread/launch
*/

namespace thread_ex
{

template <typename Function, typename... Args>
inline 
std::future<typename std::result_of<Function(Args...)>::type> 
call_async(Function&& f, Args... args)
{
    return std::async(std::launch::async
        ,std::forward<Function>(f)
        ,std::forward<Args>(args)...
    );
}

template <typename Function, typename... Args>
inline
std::future<typename std::result_of<Function(Args...)>::type>
call_deferred(Function&& f, Args... args)
{
   return std::async(std::launch::deferred
      ,std::forward<Function>(f)
      ,std::forward<Args>(args)...
   );
}

} // namespace thread_ex

#endif //_THREAD_EX_ASYNC_INCLUDED_

