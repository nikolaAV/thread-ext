#ifndef _THREAD_EX_COMPILER_INCLUDED_
#define _THREAD_EX_COMPILER_INCLUDED_

/**
	\file 		te_ compiler.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2012-02-10
	\copyright 	GNU Public License.
*/


#include "te_compiler_warning_suppress.h"
#include <type_traits>
#include <memory>
#include <utility>
#include "te_compiler_warning_rollback.h"

/**
   \brief  various C++ compiler implementations provide specific ways to ...

   - thread_local - storage duration specifier (since C++11)
   - std::make_unique - since C++14
   - std::remove_reference_t - since C++14

   \remark 
*/

#ifdef _MSC_VER
   #if _MSC_VER < 1700 
   #error Visual C++ 2012 (RTM version number: 17.00.xxxxx.x) or greater is required
   #endif

   namespace workaround
   {
      template <typename T> using remove_reference_t = typename std::remove_reference_t<T>;

      using std::make_unique;
   }

   #define thread_local __declspec(thread)

#endif // _MSC_VER


#ifdef __GNUG__

   namespace workaround
   {
      template <typename T> using remove_reference_t = typename std::remove_reference<T>::type;

		template< typename T, class... Args >
		std::unique_ptr<T> make_unique(Args&&... args)
		{
			return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
		}

   }

   #define thread_local __thread

#endif // __GNUG__


namespace thread_ex
{

/**

 */

   template <typename T> using remove_reference_t = typename workaround::remove_reference_t<T>;
   using workaround::make_unique;

} // namespace thread_ex

#endif //_THREAD_EX_COMPILER_INCLUDED_

