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


} // namespace tut

