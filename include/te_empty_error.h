#ifndef _THREAD_EX_EMPTY_ERROR_INCLUDED_
#define _THREAD_EX_EMPTY_ERROR_INCLUDED_

/**
	\file 		te_empty_error.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2012-02-10
	\copyright 	GNU Public License.
*/


#include "te_compiler_warning_suppress.h"
#include <stdexcept>
#include "te_compiler_warning_rollback.h"
#include "te_compiler.h"


/**
	\brief  	exception 'empty container', thown if a clent tries to pop element from empty queue/statck 
*/


namespace thread_ex
{

/**

*/

   struct empty_error : std::logic_error
   {
               empty_error()                       : std::logic_error("empty error") {}
      explicit empty_error(const char* msg)        : std::logic_error(msg) {}
      explicit empty_error(const std::string& msg) : std::logic_error(msg) {}
   };

} // namespace thread_ex

#endif //_THREAD_EX_EMPTY_ERROR_INCLUDED_

