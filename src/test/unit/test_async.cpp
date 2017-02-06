#include <te_async.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <functional>
#include "tut.h"

namespace
{
   struct data
   {
   };
   using test_group     = tut::test_group<data>;
   using test_instance   = test_group::object;
   test_group tg("async");

} // end of anonymous namespace


namespace tut
{
   using namespace thread_ex;

   template<>
   template<>
   void test_instance::test<1>()
   {
      set_test_name("async really");
      using namespace std::chrono;

      std::atomic<size_t> counter{0};

      std::future<int> f1 = call_async([&counter](int a1)                  { ++counter; return ++a1; },1);
      std::future<int> f2 = call_async([&counter](int a1, int a2)          { ++counter; return a1+a2+1; }, 1,1);
      std::future<int> f3 = call_async([&counter](int a1, int a2, int a3)  { ++counter; return a1+a2+a3+1; }, 1,1,1);

      std::this_thread::sleep_for(seconds(1));

      ensure(3 == counter);
      ensure(2 == f1.get());
      ensure(3 == f2.get());
      ensure(4 == f3.get());
   }

   template<>
   template<>
   void test_instance::test<2>()
   {
      set_test_name("deferred call");
      using namespace std::chrono;

      size_t counter{ 0 };

      std::future<int> f1 = call_deferred([&counter](int a1) { ++counter; return ++a1; }, 1);
      std::future<int> f2 = call_deferred([&counter](int a1, int a2) { ++counter; return a1 + a2 + 1; }, 1, 1);
      std::future<int> f3 = call_deferred([&counter](int a1, int a2, int a3) { ++counter; return a1 + a2 + a3 + 1; }, 1, 1, 1);

      std::this_thread::sleep_for(seconds(1));

      ensure(0 == counter);
      ensure(2 == f1.get());
      ensure(1 == counter);
      ensure(3 == f2.get());
      ensure(2 == counter);
      ensure(4 == f3.get());
      ensure(3 == counter);
   }

   template<>
   template<>
   void test_instance::test<3>()
   {
      set_test_name("async, argument passed as reference");
 
      int counter{0};
      std::future<int> f1 = call_async([](int& a1) { return ++a1; }, std::ref(counter));
      ensure(counter == f1.get());
   }


   template<>
   template<>
   void test_instance::test<4>()
   {
      set_test_name("async, argument passed as rvalue");

      struct r_value
      {
         int val = 0;
         r_value(int a) : val(a) {}
         r_value(const r_value&)             = delete;
         r_value& operator=(const r_value&)  = delete;
         r_value(r_value&&)                  = default;
         r_value& operator=(r_value&&)       = default;
      };

#ifdef __GNUG__
      std::future<int> f1 = call_async([](r_value&& a1) { return a1.val+1; }, r_value(25));
      ensure(26 == f1.get());
#elif _MSC_VER
   #define STRING2(x) #x
   #define STRING(x) STRING2(x)
    #pragma message (__FILE__ "[" STRING(__LINE__) "]: VS2015 defect: a rvalue cannot be passed into std::async. Check in late version")
#else
   #error unsupported compiler
#endif

   }

} // namespace tut

