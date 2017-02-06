#include <te_thread_unjoinable.h>
#include "te_compiler_warning_suppress.h"
#include <chrono>
#include "te_compiler_warning_rollback.h"
#include "tut.h"

namespace
{

   struct data {};
   using test_group = tut::test_group<data>;
   using test_intance = test_group::object;
   test_group tg("thread_unjoinable");

} // end of anonymous namespace


namespace tut
{

   using namespace thread_ex;
   using namespace std;

   template<>
   template<>
   void test_intance::test<1>()
   {
      set_test_name("joined thread");

      size_t counter {0};
      {
         joined_thread t = std::thread([&counter]{
            counter = 1;
            this_thread::sleep_for(chrono::milliseconds(100));
            counter = 2;
         });
         this_thread::sleep_for(chrono::milliseconds(10));
         ensure(counter==1);
      }
      ensure(counter == 2);
   }

   template<>
   template<>
   void test_intance::test<2>()
   {
      set_test_name("detached thread");

      size_t counter{ 0 };
      {
         detached_thread t = std::thread([&counter] {
            counter = 1;
            this_thread::sleep_for(chrono::milliseconds(100));
            counter = 2;
         });
         this_thread::sleep_for(chrono::milliseconds(10));
         ensure(counter == 1);
      }
      ensure(counter == 1);
   }

} // namespace tut

