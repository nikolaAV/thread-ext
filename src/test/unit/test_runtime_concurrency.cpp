#include <te_runtime_concurrency.h>
#include "tut.h"


namespace
{

using thread_ex::runtime_concurrency;

struct data
{
   data() = default;
   data(const data&) = delete;
   data& operator=(const data&) = delete;

   const size_t hardware_processors = std::thread::hardware_concurrency();

};
using test_group = tut::test_group<data>;
using test_intance = test_group::object;
test_group tg("runtime_concurrency");

} // end of anonymous namespace


namespace tut
{

   template<>
   template<>
   void test_intance::test<1>()
   {
      size_t thread_n = runtime_concurrency(1000,1);
      ensure(thread_n== data::hardware_processors);

      thread_n = runtime_concurrency(1000, 1000);
      ensure(thread_n == 1);
   }

   template<>
   template<>
   void test_intance::test<2>()
   {
      const size_t total_work      = 100;
      const size_t work_per_thread = 33;
      const size_t calculated_thread_no = runtime_concurrency(total_work, work_per_thread);

      switch (data::hardware_processors)
      {
         case 1 : ensure(1==calculated_thread_no); // [t1] 33 + 33 + 33 + 1
                  break;
         case 2 : ensure(2==calculated_thread_no); // [t1] 33 + 33
                  break;                           // [t2] 33 + 1
  
         case 3 : ensure(3==calculated_thread_no); // [t1] 33 + 1
                  break;                           // [t2] 33
                                                   // [t3] 33

         case 4 : ensure(4==calculated_thread_no); // [t1] 33
                  break;                           // [t2] 33
                                                   // [t3] 33
                                                   // [t4] 1

         case 8: ensure(4==calculated_thread_no);  // [t1] 33
                  break;                           // [t2] 33
                                                   // [t3] 33
                                                   // [t4] 1

         default: break;   
      }
   }

} // namespace 'tut'
