#include <te_container.h>
#include <te_move.h>
#include <te_async.h>
#include "te_compiler_warning_suppress.h"
#include <vector>
#include <algorithm>
#include <iterator>
#include "te_compiler_warning_rollback.h"
#include "tut.h"


namespace
{
   struct ConstructionCounter
   {
      ConstructionCounter() { reset(); }

      ConstructionCounter(const ConstructionCounter&) { ++copying_copy; }
      ConstructionCounter& operator=(const ConstructionCounter&) { ++copying_copy; return *this; }

      ConstructionCounter(ConstructionCounter&&) { ++moving_copy; }
      ConstructionCounter& operator=(ConstructionCounter&&) { ++moving_copy; return *this; }

      static void reset() { copying_copy = moving_copy = 0; }
      static size_t copying_copy;
      static size_t moving_copy;
   };
   size_t ConstructionCounter::copying_copy = 0;
   size_t ConstructionCounter::moving_copy = 0;

   bool operator==(const ConstructionCounter&, const ConstructionCounter&)
   {
      return true;
   }

   template <typename T> 
   using threadsafe_queue = thread_ex::threadsafe_queue<T, std::mutex>;
   using thread_ex::call_async;

   struct data {};
   using test_group = tut::test_group<data>;
   using test_intance = test_group::object;
   test_group tg("threadsafe_queue");
} // end of anonymous namespace


namespace tut
{

   template<>
   template<>
   void test_intance::test<1>()
   {
      typedef threadsafe_queue<ConstructionCounter> queue_t;

      queue_t q;
      ensure(q.empty());
      ensure(0==q.size());
      ensure(0==ConstructionCounter::copying_copy);
      ensure(0==ConstructionCounter::moving_copy);

      q.push(ConstructionCounter{});
      ensure(!q.empty());
      ensure(1 == q.size());
      ensure(0 == ConstructionCounter::copying_copy);
      ensure(1 == ConstructionCounter::moving_copy);

      queue_t q2(std::move(q));
      ensure(!q2.empty());
      ensure(1 == q2.size());
      ensure(0 == ConstructionCounter::copying_copy);
      ensure(1 == ConstructionCounter::moving_copy);

      queue_t q3;
      q3 = std::move(q2);
      ensure(!q3.empty());
      ensure(1 == q3.size());
      ensure(0 == ConstructionCounter::copying_copy);
      ensure(1 == ConstructionCounter::moving_copy);

      queue_t q4;
      q4 = q3;
      ensure(q3==q4);
      ensure(1 == ConstructionCounter::copying_copy);
      ensure(1 == ConstructionCounter::moving_copy);

      swap(q3,q4);
      ensure(q3 == q4);
      ensure(1 == ConstructionCounter::copying_copy);
      ensure(1 == ConstructionCounter::moving_copy);
   }


   template<>
   template<>
   void test_intance::test<2>()
   {
      typedef threadsafe_queue<int> queue_t;

      queue_t q1;
      q1.push(1);
      q1.push(2);
      q1.push(3);
      ensure(3 == q1.size());

      queue_t q2(q1);
      ensure(q2 == q1);

      using namespace std::rel_ops; // based on '==' and '<' we can get rest (4) of them: '<=', '>=', "!=" and '>' 
 
      ensure(q2 == q1);
      ensure(q2 >= q1);
      ensure(q2 <= q1);

      q2.push(4);

      ensure(q1 < q2);
      ensure(q2 > q1);

      swap(q1,q2);
      ensure(q1 > q2);
      ensure(q2 < q1);
   }


   template<>
   template<>
   void test_intance::test<3>()
   {
      typedef threadsafe_queue<int> queue_t;

      queue_t q;
      for (int i = 0; i<100; ++i)
         q.push(i);

      ensure(100 == q.size());
      ensure(0 == *q.try_pop());
      ensure(1 == *q.try_pop());
      ensure(2 == *q.try_pop());
      ensure(3 == *q.try_pop());
      ensure(4 == *q.try_pop());
      int value{0};
      q.try_pop(value);
      ensure(5 == value);
      ensure(94 == q.size());

      queue_t other;
      swap(other,q);
      ensure(0 == q.size());
      ensure(94 == other.size());

      ensure(!q.try_pop());
      value = 1234;
      ensure(!q.try_pop(std::nothrow,value));
      ensure(1234==value);
   }

   template<>
   template<>
   void test_intance::test<4>()
   {
      threadsafe_queue<int> q;
      for (int i = 0; i<10; ++i)
         q.push(i);

      ensure(10 == q.size());

      //std::queue<int> out;
      typename decltype(q)::container_type out;
      q.try_pop(out);
      ensure(q.empty());
      ensure(out == std::queue<int>({0,1,2,3,4,5,6,7,8,9}));
   }

   template<>
   template<>
   void test_intance::test<5>()
   {
      threadsafe_queue<std::string> q;
      ensure(q.empty());

      std::future<std::string> result = call_async([&q]{
            std::string out;
            q.wait_pop(out);
            return out;
         } 
      ); 

      q.push("first");
      q.push("second");
      q.push("third");

      ensure(result.get()=="first");
      ensure(2==q.size());
   }


   template<>
   template<>
   void test_intance::test<6>()
   {
      threadsafe_queue<std::string> q;
      ensure(q.empty());

      std::future<std::string> result = call_async([&q] {
            return *q.wait_pop();
         }
      );

      q.push("first");
      q.push("second");
      q.push("third");

      ensure(result.get() == "first");
      ensure(2 == q.size());
   }

   template<>
   template<>
   void test_intance::test<7>()
   {
      set_test_name("one writer - one reader");

      typedef size_t value_type;
      threadsafe_queue<value_type>  q_shared;

      std::future<std::queue<value_type>> result = call_async([&q_shared]{
         value_type v{ 0 };
         std::queue<value_type> out;
         for(q_shared.wait_pop(v);1000!=v; q_shared.wait_pop(v))
            out.push(v);
         return out;
      });

      for(size_t i = 0; i <=1000; ++i)
         q_shared.push(i);

      auto q = result.get();
      ensure(1000 == q.size());
      ensure(0 == q.front()); q.pop();
      ensure(1 == q.front()); q.pop();
      ensure(2 == q.front()); q.pop();
      ensure(3 == q.front()); q.pop();
   }

   template<>
   template<>
   void test_intance::test<8>()
   {
      set_test_name("one writer - two readers");

      typedef size_t value_type;
      threadsafe_queue<value_type>  q_shared;
      std::promise<void>            do_fill;
      std::shared_future<void>      filled_done = do_fill.get_future(); 

      auto reader = [&q_shared,&filled_done]{
         std::vector<value_type> out;
         while(std::future_status::ready!=filled_done.wait_for(std::chrono::microseconds(1)))
            out.push_back(*q_shared.wait_pop());
         for(auto i = q_shared.try_pop(); i ; i = q_shared.try_pop())
            out.push_back(*i);
         return out;
      };

      std::future<std::vector<value_type>> result1 = call_async(reader);
      std::future<std::vector<value_type>> result2 = call_async(reader);

      for (size_t i = 0; i < 1000; ++i)
         q_shared.push(i);
      do_fill.set_value();

      auto v1 = result1.get();
      auto v2 = result2.get();

      ensure("unexpected size of the output", 1000 == v1.size() + v2.size());
      ensure("reader1 not sorted", std::is_sorted(v1.begin(), v1.end()));
      ensure("reader2 not sorted", std::is_sorted(v2.begin(), v2.end()));
      
      std::vector<value_type> v3;
      std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), std::back_inserter(v3));
      ensure("duplication in the output is not allowed", v3.empty() );
      std::set_intersection(v2.begin(), v2.end(), v1.begin(), v1.end(), std::back_inserter(v3));
      ensure("duplication in the output is not allowed", v3.empty());
   }

   template<>
   template<>
   void test_intance::test<9>()
   {
      set_test_name("two writers - one reader");

      typedef size_t value_type;
      threadsafe_queue<value_type>  q_shared;

      auto writer = [&q_shared] (value_type const start_from) {
         for(value_type i = start_from; i < start_from + 1000; ++i)
            q_shared.push(i);
      };

      std::future<void> w1 = call_async(writer, 0);
      std::future<void> w2 = call_async(writer, 2000);

      auto writer_working = [&w1, &w2]{
         return   std::future_status::ready != w1.wait_for(std::chrono::seconds(0)) 
               || std::future_status::ready != w2.wait_for(std::chrono::seconds(0))
         ;
      };

      std::vector<value_type>             output;
      decltype(q_shared)::container_type  portion;
      do
      {
         q_shared.wait_pop(portion);
         thread_ex::move(portion,std::back_inserter(output));
      } 
      while(writer_working());

      while(!q_shared.empty())
      { 
         q_shared.try_pop(portion);
         thread_ex::move(portion, std::back_inserter(output));
      }

      ensure(2000 == output.size());
      ensure(!std::is_sorted(output.begin(), output.end()));
   }

} // namespace 'tut'
