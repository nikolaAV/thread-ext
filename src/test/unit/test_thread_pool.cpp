#include "tut.h"
#include <te_thread_pool.h>
#include <te_block_lock.h>
#include <chrono>
#include <map>

namespace
{
   struct data
   {
   };
   using test_group     = tut::test_group<data>;
   using test_instance   = test_group::object;
   test_group tg("thread pool");

   using thread_ex::thread_pool;
   using namespace thread_ex::block;
   using namespace std;
   using namespace std::chrono_literals;

} // end of anonymous namespace


namespace tut
{
   template<>
   template<>
   void test_instance::test<1>()
   {
      set_test_name ("graceful exit by default");
      mutex m;
      map<thread::id,size_t> ids;

      auto l = [&m,&ids]() { lock(m,[&]{
         ++(ids[this_thread::get_id()]);
         this_thread::yield();
      });};

      {  thread_pool tp;   
         ensure( std::thread::hardware_concurrency()==tp.thread_count());

         for(size_t i=0; i < 1000; ++i)
            tp.submit(std::ref(l));
      }  // <-- behalf of destructor, stop() will be invoked

      size_t total_task_completed = 0;
      for(const auto& i : ids)
      {
//         cout << i.first << " -> " << i.second << endl;
         total_task_completed += i.second;
      }

      ensure( 1000==total_task_completed);
   }


   template<>
   template<>
   void test_instance::test<2>()
   {
      set_test_name ("termination");
      mutex m;
      map<thread::id,size_t> ids;

      auto l = [&m,&ids]() { lock(m,[&]{
         ++(ids[this_thread::get_id()]);
         this_thread::yield();
      });};

      {  thread_pool tp;   
         ensure( std::thread::hardware_concurrency()==tp.thread_count());

         for(size_t i=0; i < 1000; ++i)
            tp.submit(std::ref(l));
         tp.terminate();
      }

      size_t total_task_completed = 0;
      for(const auto& i : ids)
      {
//         cout << i.first << " -> " << i.second << endl;
         total_task_completed += i.second;
      }
//      cout << "-----------"<< endl << "total: " << total_task_completed << endl;

      ensure( 1000>=total_task_completed);
   }


   template<>
   template<>
   void test_instance::test<3>()
   {
      set_test_name ("exception handling");

      auto lambda = [] (size_t v) {
         if(v%2)
            throw invalid_argument("odd number is not allowed");
         return v+10;
      };

      thread_pool tp{1};
      auto f1 = tp.submit(lambda,1);
      auto f2 = tp.submit(lambda,2);
      auto f3 = tp.submit(lambda,3);
      auto f4 = tp.submit(lambda,4);

      ensure(1==tp.thread_count());
      ensure(12==f2.get());
      ensure(14==f4.get());

      try
      {
         f1.get();
         ensure(!"this line is not reachable");
      }
      catch(const invalid_argument&)
      {
      }

      try
      {
         f3.get();
         ensure(!"this line is not reachable");
      }
      catch(const invalid_argument& e)
      {
         ensure(string("odd number is not allowed")==e.what());   
      }

   }

} // namespace tut

