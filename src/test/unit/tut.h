#ifndef _THREAD_EX_TUT_INCLUDED_
#define _THREAD_EX_TUT_INCLUDED_

#ifdef _MSC_VER
   #pragma warning( push )
   #pragma warning( disable: 4061 ) // enumerator 'tut::test_result::skipped' in switch of enum 'tut::test_result::result_type' is not explicitly handled by a case label
   #pragma warning( disable: 4365 ) // 'return': conversion from '...' to '...', signed/unsigned mismatch
   #pragma warning( disable: 4571 ) // semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
   #pragma warning( disable: 4623 ) // '<type_name>': default constructor was implicitly defined as deleted
   #pragma warning( disable: 4625 ) // '<class_name>' : copy constructor was implicitly defined as deleted
   #pragma warning( disable: 4626 ) // 'tut::no_such_group': assignment operator was implicitly defined as deleted
   #pragma warning( disable: 4711 ) // function '<name>' selected for automatic inline expansion
   #pragma warning( disable: 4774 ) // '<function_name>' : format string expected in argument <num> is not a string literal
   #pragma warning( disable: 4987 ) // nonstandard extension used: 'throw (...)'
   #pragma warning( disable: 5026 ) // '<type_name>': move constructor was implicitly defined as deleted
   #pragma warning( disable: 5027 ) // '<type_name>': move assignment operator was implicitly defined as deleted
#endif

#ifdef __GNUG__
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wunused-function"
#endif


#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>

#ifdef __GNUG__
   #pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
   #pragma warning( pop )
#endif

#endif //_THREAD_EX_TUT_INCLUDED_
