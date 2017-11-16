#include <te_sequence.h>
#include "te_async.h"
#include "te_compiler_warning_suppress.h"
#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <iterator>
#include "te_compiler_warning_rollback.h"
#include "tut.h"


namespace
{
   using threadsafe_vector = thread_ex::threadsafe_vector<int>;
   using empty_stack_error  = thread_ex::empty_error;

   struct record 
   {
      size_t         pk;
      std::string    name;
      size_t         age;
      unsigned char  sex; // 'f' or 'm';
   };

   using persons_type = thread_ex::threadsafe_vector<record>;

   struct data {};
   using test_group = tut::test_group<data>;
   using test_intance = test_group::object;
   test_group tg("threadsafe_vector");
} // end of anonymous namespace

using namespace std;

namespace tut
{
   template<>
   template<>
   void test_intance::test<1>()
   {
      threadsafe_vector v1;
      v1.push_back(1);
      v1.push_back(2);
      int val{3};
      v1.push_back(val);

      ensure(3==v1.size());
      ensure(!v1.empty());

      v1.pop_back();
      ensure(2 == v1.size());
      v1.pop_back(val);
      ensure(1 == v1.size());
      ensure(2 == val);

      v1.pop_back(val);
      const auto res = v1.pop_back(nothrow,val);
      ensure(!res); // <-- must be false

   }

   template<>
   template<>
   void test_intance::test<2>()
   {
      vector<int> src = {0,1,2,3,4,5,6,7,8,9};
      threadsafe_vector v;
      v.swap(src);

      ensure(10 == v.size());
      ensure(!v.empty());
      ensure(src.empty());


      threadsafe_vector v2;
      v2.swap(v);
      v2.swap(src);

      ensure(v.empty());
      ensure(v2.empty());
      ensure(!src.empty());

      ensure(src == vector<int>{0,1,2,3,4,5,6,7,8,9});
   }

   template<>
   template<>
   void test_intance::test<3>()
   {
      vector<int> src = { 0,1,2,3,4,5,6,7,8,9 };
      threadsafe_vector v1;
      v1.swap(src);
      ensure(10 == v1.size());

      threadsafe_vector v2 (v1);

      using namespace rel_ops; // based on '==' and '<' we can get rest (4) of them: '<=', '>=', "!=" and '>'

      ensure(v2 == v1);
      ensure(v2 >= v1);
      ensure(v2 <= v1);

      auto v = v1.pop_back();
      ensure(v2 > v1);
      ensure(v1 < v2);
      ensure(v2 != v1);
   }

   template<>
   template<>
   void test_intance::test<4>()
   {
      persons_type pns;
      {
         record r = {1,"Alex",49,'m'};
         pns.push_back(r);
      }
      {
         record r = { 2,"Irina",42,'f' };
         pns.push_back(r);
      }
      {
         record r = { 3,"Anastasya",14,'f' };
         pns.push_back(r);
      }
      {
         record r = { 4,"Stas",10,'m' };
         pns.push_back(r);
      }

      ensure(4==pns.size());
      auto out = pns.find_first([](const record& r){ 
                        return 1==r.pk;
                    });
      ensure(out.first && "Alex"==out.second.name);
      out = pns.find_first([](const record& r){
                        return 4==r.pk;
                    });
      ensure(out.first && "Stas"==out.second.name);

      out = pns.compare_exchange(
            [](const record& r){
                  return "Irina"==r.name; 
            },
            [](record& r){
                  return ++r.age; 
            }
      );
      ensure(out.first && 42==out.second.age );
      out = pns.find_first([](const record& r){
                        return 2==r.pk;
                    });
      ensure(out.first && "Irina"==out.second.name && 43==out.second.age);

      out = pns.compare_exchange(
            [](const record& r){
                  return "Anastasya"==r.name && 'm'==r.sex; 
            },
            [](record& r){
                  return ++r.age; 
            }
      );
      ensure(!out.first);

   }


   struct my_vector : threadsafe_vector
   {
      bool is_sorted() const
      {
         using ForwardIt = typename container_type::const_iterator; 
         return call_under_lock(std::is_sorted<ForwardIt>,begin(container_),end(container_));
      }
      void sort()
      {
         using RandomIt = typename container_type::iterator; 
         call_under_lock(std::sort<RandomIt>,begin(container_),end(container_));
      }

      template <typename UnaryFun>
      void unsafe_transform(UnaryFun f)
      {
         std::transform(begin(container_),end(container_),begin(container_),f);
      }

      int unsafe_sum() const
      {
         return accumulate(begin(container_),end(container_),0);
      }
   };



   template<>
   template<>
   void test_intance::test<5>()
   {
      my_vector v;

      auto f1 = thread_ex::call_async([&v](){
            for(int i=0; i<10000; i+=2)
               v.push_back(i);
      });
      auto f2 = thread_ex::call_async([&v](){
            for(int i=1; i<10000; i+=2)
               v.push_back(i);
      });

      f1.get(),f2.get();
      ensure("10000==v.size()", 10000==v.size());
      ensure("!v.is_sorted()", !v.is_sorted());

      v.sort();
      ensure("v.is_sorted()", v.is_sorted());

      const auto increment = [](const int& i) { return i+3; };
      const auto decrement = [](const int& i) { return i-2; };
      auto f3 = thread_ex::call_async(
          [&v](decltype(increment) f) { v.unsafe_transform(f); }
         ,increment
      );
      auto f4 = thread_ex::call_async(
          [&v](decltype(decrement) f) { v.unsafe_transform(f); }
         ,decrement
      );

      f3.get(),f4.get();
      ensure("after decrement/increment: 10000==v.size()", 10000==v.size());
      ensure("after decrement/increment: v.is_sorted()", v.is_sorted());

      ensure("10000*(10000+1)/2==v.unsafe_sum()", 10000*(10000+1)/2==v.unsafe_sum());
   }

   template<>
   template<>
   void test_intance::test<6>()
   {
      threadsafe_vector                   v1 (vector<int>{0,1,2,3,4,5,6,7,8,9});
      threadsafe_vector::container_type   v2;
      v1.copy(v2,[](const int& i){ return i >= 5; });

      ensure(5==v2.size());
      ensure(is_sorted(begin(v2),end(v2)));

      v1.erase([](const int& i) { return i >= 5; });
      ensure(5 == v1.size());
      ensure(v1 == vector<int>{0, 1, 2, 3, 4});

      const auto n = v1.clear();
      ensure(0 == v1.size());
      ensure(5 == n);
   }

   template<>
   template<>
   void test_intance::test<7>()
   {
      threadsafe_vector v (vector<int>{0,1,2,3,4,5,6,7,8,9});
      list<int>    odd;
      list<int>    even;

      v.for_each([&odd,&even](const int& v){
         if(v%2) odd.push_back(v);
         else    even.push_back(v);         
      });

      ensure("odd + even = total", odd.size() + even.size() == v.size());
      ensure("even list", even ==list<int>{0,2,4,6,8});
      ensure("odd list",  odd  ==list<int>{1,3,5,7,9});
   }

   template<>
   template<>
   void test_intance::test<8>()
   {
      struct record { 
         size_t t0 = 0;
         size_t t1 = 0;
         size_t t2 = 0;
         size_t t3 = 0;
         long   a  = 0;
         long   b  = 0;
         long   c  = 0;
      };

      using my_container_t = thread_ex::threadsafe_vector<record>;

      my_container_t shared_obj;
      shared_obj.push_back(record{});
      auto r = shared_obj.find_first([](const record&r){ return r.t0==0;}).second;
      ensure(0==r.t0 && 0==r.t1 && 0==r.t2 && 0==r.t3 && 0==r.a);

      const size_t loop_count = 100000;

      auto f1 = thread_ex::call_async([&shared_obj,loop_count](){
         for(size_t i = 0; i < loop_count; ++i)
         {   shared_obj.compare_exchange(
                [](const record&) { return true; }
               ,[](record& r)       { 
                     const auto a_copy = r.a+1;
                     const auto b_copy = r.b-1;
                     ++r.t0; 
                     ++r.t1;
                     r.b = b_copy;
                     r.a = a_copy;
                }
             );   
         }
      });

      auto f2 = thread_ex::call_async([&shared_obj,loop_count](){
         for(size_t i = 0; i < loop_count; ++i)
         {   shared_obj.compare_exchange(
                [](const record&) { return true; }
               ,[](record& r)       { 
                     const auto b_copy = r.b+1;
                     const auto c_copy = r.c-1;
                     ++r.t0; 
                     ++r.t2;
                     r.c = c_copy;
                     r.b = b_copy;
                }
             );   
         }
      });

      auto f3 = thread_ex::call_async([&shared_obj,loop_count](){
         for(size_t i = 0; i < loop_count; ++i)
         {   shared_obj.compare_exchange(
                [](const record&) { return true; }
               ,[](record& r)       { 
                     const auto c_copy = r.c+1;
                     const auto a_copy = r.a-1;
                     ++r.t0; 
                     ++r.t3;
                     r.a = a_copy;
                     r.c = c_copy;
                }
             );   
         }
      });

      vector<bool> evidence_thread_in_parallel;
      ensure(evidence_thread_in_parallel.empty());
      auto f0 = thread_ex::call_async([&shared_obj,loop_count,&evidence_thread_in_parallel](){
         record r{};
         while(loop_count*3!=r.t0)
         {   
            r = shared_obj.find_last([](auto) { return true; }).second;   
            ensure("r.t0 = t1+t2+t3", r.t0 == r.t1 + r.t2 + r.t3);
            if(r.t1 && r.t2 && r.t3)
               evidence_thread_in_parallel.push_back(true);
         }
      });

      f1.get(); f2.get(); f3.get(); f0.get();
      r = shared_obj.find_last([](auto){return true;}).second;
      ensure("loop_count*3==r.t0", loop_count*3==r.t0);
      ensure("r.t1==r.t2==r.t3", loop_count==r.t1 && loop_count==r.t2 &&loop_count==r.t3);
      ensure("a,b,c==0", 0==r.a && 0==r.b && 0==r.c);
      ensure("evidence_thread_in_parallel", !evidence_thread_in_parallel.empty());
   }

   template<>
   template<>
   void test_intance::test<9>()
   {
      threadsafe_vector v(vector<int>{0,1,2,3,4,5,6,7,8,9});
      v.transform([](const int& i){
         return i%2? i+10:i;
      });
      ensure(v==vector<int>{0,11,2,13,4,15,6,17,8,19});
   }

   template<>
   template<>
   void test_intance::test<10>()
   {
      enum Colours { Black, Red, White };

      struct Node
      {
         explicit Node(Colours c) : colour(c) {}
         Colours colour;

         Node(const Node&)             = delete;
         Node& operator=(const Node&)  = delete;
         Node(Node&&)                  = default;
         Node& operator=(Node&&)       = default;
      };

      thread_ex::threadsafe_vector<Node> source;
      for(size_t i = 0; i < 100; ++i)
      {
         source.push_back(Node{Black});   
         source.push_back(Node{Red});   
         source.push_back(Node{White});   
      }
      ensure(300==source.size());

      std::vector<Node> destination;
      source.move(destination,[](const Node& n){ return Red==n.colour; });

      ensure(200==source.size());
      ensure(100==destination.size());
   }

} // namespace 'tut'
