/**
	\file 		te_ compiler_warning_suppress.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2012-02-10
	\copyright 	GNU Public License.
*/

/**
    if you compile your solution in the 'pedantic' mode (/W4, higest warnong level)
    you will receive some messages from the Standard Library
    in order to skip these unwiling warnings from the Standard Code (we trust it, doesn't we :-))
    we can just suppress them ...  

    in your application code, follow the basic construct

    #include "te_compiler_warning_suppress.h"
    ...
    #include <any standard headed>
    ...
    #include "te_compiler_warning_rollback.h"
*/


#ifdef  _THREAD_EX_COMPILER_WARNING_SUPRESS_INCLUDED_ 
   #error te_compiler_warning_suppress.h cannot be sequentially included twice or more. Use also te_compiler_warning_rollback.h 
#endif

#define _THREAD_EX_COMPILER_WARNING_SUPRESS_INCLUDED_

#ifdef _MSC_VER
#pragma warning( push )
   #pragma warning( disable: 4265 ) // '<class_name>': class has virtual functions, but destructor is not virtual
   #pragma warning( disable: 4355 ) // 'this': used in base member initializer list
   #pragma warning( disable: 4571 ) // Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
   #pragma warning( disable: 4625 ) // '<class_name>' : copy constructor was implicitly defined as deleted
   #pragma warning( disable: 4626 ) // '<class_name>' : assignment operator was implicitly defined as deleted
   #pragma warning( disable: 4711 ) // function '<name>' selected for automatic inline expansion
   #pragma warning( disable: 5026 ) // '<class_name>': move constructor was implicitly defined as deleted
   #pragma warning( disable: 5027 ) // '<class_name>': move assignment operator was implicitly defined as deleted
   #pragma warning( disable: 5031 ) // #pragma warning(pop) : likely mismatch, popping warning state pushed in different file
#endif

#ifdef __GNUG__
#pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wunused-function"
// #pragma GCC diagnostic ignored "-Weffc++"
// #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
   #pragma GCC diagnostic ignored "-Wunused-variable"
#endif
