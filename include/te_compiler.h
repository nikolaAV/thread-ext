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
#include <tuple>
#include "te_compiler_warning_rollback.h"

/**
   \brief  various C++ compiler implementations provide specific ways to ...

   - thread_local - storage duration specifier (since C++11)
   - std::make_unique - since C++14
   - std::remove_reference_t - since C++14

   namespace 'workaround' must be removed in case of conflict detection 
   with std::analogues deployed with C++ compiler of newer version  

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
      using std::rbegin;
      using std::rend;
   }

   #define thread_local __declspec(thread)

#endif // _MSC_VER


#ifdef __GNUG__

   namespace workaround
   {
      template <typename T> using remove_reference_t = typename std::remove_reference<T>::type;

      using std::make_unique;

      template <typename C>
      auto rbegin(C& c) -> decltype(c.rbegin())
      {
         return c.rbegin();
      }
      template <typename C>
      auto rend(C& c) -> decltype(c.rend())
      {
         return c.rend();
      }

   }

   #define thread_local __thread

#endif // __GNUG__


   namespace workaround
   {
      /**
         idea http://aherrmann.github.io/programming/2016/02/28/unpacking-tuples-in-cpp14/ 
      */

      template <typename F, size_t... Is>
      constexpr 
      decltype(auto) 
      index_apply_impl(F&& f, std::index_sequence<Is...>) 
      {
          return std::forward<F>(f)(std::integral_constant<size_t, Is> {}...);
      }

      template <size_t N, typename F>
         // where F is Callable (http://en.cppreference.com/w/cpp/concept/Callable) 
         // with a sequence of integral constants as variadic input. Signatute Ret (int<0>, int<1>, int<2>, ...)
      constexpr 
      decltype(auto) 
      index_apply(F&& f) 
      {
          return index_apply_impl(std::forward<F>(f), std::make_index_sequence<N>{});
      }

      /**
         \brief takes a callable and a tuple and calls the callable with the el­e­ments of the tuple as ar­gu­ments.
         http://en.cppreference.com/w/cpp/utility/apply
      */
      template <typename F, typename Tuple>
      constexpr 
      decltype(auto)
      apply(F&& f, Tuple&& t)
      {
         return index_apply<std::tuple_size<std::decay_t<Tuple>>::value>([&](auto... Is) {
            return std::forward<F>(f)(std::get<Is>(std::forward<Tuple>(t))...);
         });
      }

   }  // <- end of workaround  

namespace thread_ex
{

/**

 */

   template <typename T> using remove_reference_t = typename workaround::remove_reference_t<T>;
   using workaround::make_unique;
   using workaround::rbegin;
   using workaround::rend;
   using workaround::apply;

} // namespace thread_ex

#endif //_THREAD_EX_COMPILER_INCLUDED_

