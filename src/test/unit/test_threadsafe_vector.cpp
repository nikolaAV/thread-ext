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
      auto out = pns.find_if([](const record& r){ 
                        return 1==r.pk;
                    });
      ensure(out.first && "Alex"==out.second.name);
      out = pns.find_if([](const record& r){
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
      out = pns.find_if([](const record& r){
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
         return call_under_lock(std::is_sorted<ForwardIt>,cbegin(container_),cend(container_));
      }
      void sort()
      {
         using RandomIt = typename container_type::iterator; 
         call_under_lock(std::sort<RandomIt>,begin(container_),end(container_));
      }

      template <typename UnaryFun>
      void unsafe_transform(UnaryFun f)
      {
         std::transform(cbegin(container_),cend(container_),begin(container_),f);
      }

      int unsafe_sum() const
      {
         return accumulate(cbegin(container_),cend(container_),0);
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
      ensure(10000==v.size());
      ensure(!v.is_sorted());

      v.sort();
      ensure(v.is_sorted());

      auto increment = [](const int& i) { return i+3; };
      auto decrement = [](const int& i) { return i-2; };
      auto f3 = thread_ex::call_async(
          [&v](decltype(increment) f) { v.unsafe_transform(f); }
         ,increment
      );
      auto f4 = thread_ex::call_async(
          [&v](decltype(decrement) f) { v.unsafe_transform(f); }
         ,decrement
      );

      f3.get(),f4.get();
      ensure(10000==v.size());
      ensure(v.is_sorted());

      ensure(10000*(10000+1)/2==v.unsafe_sum());

   }

} // namespace 'tut'
