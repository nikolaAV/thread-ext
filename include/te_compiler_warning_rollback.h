/**
	\file 		te_ compiler_warning_rollback.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2012-02-10
	\copyright 	GNU Public License.
*/


#ifndef  _THREAD_EX_COMPILER_WARNING_SUPRESS_INCLUDED_ 
   #error te_compiler_warning_suppress.h must be included first
#endif

#undef _THREAD_EX_COMPILER_WARNING_SUPRESS_INCLUDED_

#ifdef __GNUG__
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning( pop )
#endif
