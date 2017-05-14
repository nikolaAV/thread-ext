#include <te_container.h>
#include "te_async.h"
#include "te_compiler_warning_suppress.h"
#include <array>
#include <vector>
#include <numeric>
#include <algorithm>
#include <thread>
#include <chrono>
#include "te_compiler_warning_rollback.h"
#include "tut.h"


namespace
{
   using threadsafe_stack     = thread_ex::threadsafe_stack<int>;
   using empty_stack_error    = thread_ex::empty_error;

   struct data {};
   using test_group = tut::test_group<data>;
   using test_intance = test_group::object;
   test_group tg("threadsafe_stack");
} // end of anonymous namespace

using namespace std;

namespace tut
{

   template<>
   template<>
   void test_intance::test<1>()
   {
      threadsafe_stack s1;
      s1.push(1);
      s1.push(2);
      s1.push(3);

      ensure(3==s1.size());
      threadsafe_stack s2 (s1);
 
      using namespace rel_ops; // based on '==' and '<' we can get rest (4) of them: '<=', '>=', "!=" and '>' 

      ensure(s2 == s1);
      ensure(s2 >= s1);
      ensure(s2 <= s1);

      auto v = s1.pop();
      ensure(s2 > s1);
      ensure(s1 < s2);
      ensure(s2 != s1);
   }

   template<>
   template<>
   void test_intance::test<2>()
   {
      threadsafe_stack s;

      ensure("empty", s.empty());
      auto v = s.pop();
      ensure(v==nullptr);

//      auto v1 = s.pop(nothrow);
//      ensure("<null pointer>", !v1);

      int v2 = 77;
      s.pop(nothrow,v2);
      ensure("output must be not changed", 77==v2);

      s.push(1);
      ensure("not empty", !s.empty());
   }


   template<>
   template<>
   void test_intance::test<3>()
   {
      threadsafe_stack s;
      for(int i=0; i<100; ++i)
         s.push(i);

      ensure(100==s.size());
      threadsafe_stack s2;
      swap(s2,s);
      ensure(s.empty());
      ensure(100 == s2.size());
      ensure(99 == *s2.pop());
      ensure(98 == *s2.pop());
      ensure(97 == *s2.pop());
      ensure(96 == *s2.pop());
      ensure(95 == *s2.pop());
  }


   template<>
   template<>
   void test_intance::test<4>()
   {
      thread_ex::threadsafe_stack<int> stack;
      array<int,1000>                  input;
      vector<int>                      output;


      promise<void>         ready_to_read, ready_to_write, ready_to_start; 
      shared_future<void>   start(ready_to_start.get_future());

      auto writer = [&]() {
         ready_to_write.set_value();
         start.wait();
         for(const auto i : input ) 
            stack.push(i); 
      };

      auto reader = [&]() {
         ready_to_read.set_value();
         start.wait();
         while (input.size() != output.size())
            if(auto i = stack.pop())
               output.push_back(*i);
            else
               this_thread::yield();
      };

      shared_future<void> writer_done = thread_ex::call_async(writer);
      shared_future<void> reader_done = thread_ex::call_async(reader);

      iota(input.begin(),input.end(),0);
      ensure(is_sorted(input.begin(), input.end()));
      ensure(1000==input.size());

      ready_to_read.get_future().wait();
      ready_to_write.get_future().wait();
      ready_to_start.set_value();

      writer_done.get();
      reader_done.get();

      ensure("stack.empty()", stack.empty());
      ensure("size==1000", input.size()==output.size());
      // ensure("is output sorted?", !std::is_sorted(output.begin(), output.end()));
      sort(output.begin(), output.end());
      ensure(includes(input.begin(), input.end() ,output.begin(), output.end()));
   }

   template<>
   template<>
   void test_intance::test<5>()
   {
      thread_ex::threadsafe_stack<int> s;
      for (int i = 0; i<10; ++i)
         s.push(i);

      ensure(10 == s.size());

      //std::stack<int> out;
      typename decltype(s)::container_type out;
      s.pop(out);
      ensure(s.empty());
      ensure(out == stack<int>({ 0,1,2,3,4,5,6,7,8,9 }));
   }

   template<>
   template<>
   void test_intance::test<6>()
   {

      using notify_stack_t = thread_ex::condition_wrap<int,stack<int>>; 

      notify_stack_t    stack_shared; // empty!
      vector<int>       local_input(10000);
      iota(local_input.begin(), local_input.end(), 0);

      auto reader = thread_ex::call_async([&stack_shared]
         { 
               vector<int>  local;
               int value;
               do
               {  stack_shared.wait_pop(value);
                  local.push_back(value);
               }
               while(10000 != local.size());
               return local;
         });

      // writer
      for (const auto i : local_input)
      {
         if(i%2)
         //   this_thread::sleep_for(chrono::milliseconds(1));
            this_thread::yield();
         stack_shared.push(i);
      }
     
      vector<int> local_output = reader.get();
      ensure(10000== local_output.size());
      ensure(!is_sorted(local_output.begin(), local_output.end()));
      sort(local_output.begin(), local_output.end());
      ensure(local_input == local_output);

   }

   template<>
   template<>
   void test_intance::test<7>()
   {
      // please compare with test<6>
      // thread_ex::condition_wrap<int, stack<int>> vs. thread_ex::condition_wrap<int, queue<int>>

      using notify_queue_t = thread_ex::condition_wrap<int, queue<int>>;

      notify_queue_t    queue_shared; // empty!
      vector<int>       local_input(10000);
      iota(local_input.begin(), local_input.end(), 0);

      auto reader = thread_ex::call_async([&queue_shared]
      {
         vector<int>  local;
         int value;
         do
         {
            queue_shared.wait_pop(value);
            local.push_back(value);
         } while (10000 != local.size());
         return local;
      });

      // writer
      for (const auto i : local_input)
      {
         if (i % 2)
            //   this_thread::sleep_for(chrono::milliseconds(1));
            this_thread::yield();
         queue_shared.push(i);
      }

      vector<int> local_output = reader.get();
      ensure(10000 == local_output.size());
//       just commented from test<6> for stack    
//      ensure(!is_sorted(local_output.begin(), local_output.end()));
//      sort(local_output.begin(), local_output.end());
      ensure(local_input == local_output);

   }


} // namespace 'tut'
