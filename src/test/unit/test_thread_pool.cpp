#include <te_thread_pool.h>
#include "tut.h"

namespace
{
   struct data
   {
   };
   using test_group     = tut::test_group<data>;
   using test_instance   = test_group::object;
   test_group tg("thread pool");

   using thread_ex::thread_pool;

} // end of anonymous namespace


namespace tut
{
   using namespace thread_ex;

   template<>
   template<>
   void test_instance::test<1>()
   {
      thread_pool tp;   
      tp.async([](){});
      ensure( std::thread::hardware_concurrency()==tp.thread_count());
   }


} // namespace tut

