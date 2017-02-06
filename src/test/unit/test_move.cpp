#include <te_move.h>
#include "tut.h"

#include <stack>
#include <queue>
#include <deque>
#include <vector>
#include <array>
#include <list>

namespace
{

   struct only_movable
   {
      only_movable() = default;
      only_movable(const only_movable&) = delete;
      only_movable& operator=(const only_movable&) = delete;
      only_movable(only_movable&& other) : count_(other.count_)   { ++count_; }
      only_movable& operator=(only_movable&& other)               { count_=other.count_; ++count_; return *this; }

      size_t count_ = 0;
   };



   using thread_ex::move; 

   struct data {};
   using test_group     = tut::test_group<data>;
   using test_intance   = test_group::object;
   test_group tg("move");
} // end of anonymous namespace


namespace tut
{

   template<>
   template<>
   void test_intance::test<1>()
   {
      set_test_name("original source: queue"); 

      std::queue<int>   q (std::deque<int>{0,1,2,3,4,5,6,7,8,9});
      std::vector<int>  out;
      auto end = move(q,std::back_inserter(out));
      ensure(10==out.size());
      ensure(out== std::vector<int>{0,1,2,3,4,5,6,7,8,9});

      *end = 0;
      ensure(11 == out.size());
      ensure(out == std::vector<int>{0,1,2,3,4,5,6,7,8,9,0});
   }

   template<>
   template<>
   void test_intance::test<2>()
   {
      set_test_name("original source: stack");

      std::stack<int>   s(std::deque<int>{0,1,2,3,4,5,6,7,8,9});
      std::vector<int>  out;
      move(s, std::back_inserter(out));
      ensure("size 10", 10==out.size());
      ensure(out==std::vector<int>{9,8,7,6,5,4,3,2,1,0});
   }

   template<>
   template<>
   void test_intance::test<3>()
   {
      set_test_name("original source: list");

      std::list<int>    l(std::list<int>{0,1,2,3,4,5,6,7,8,9});
      std::vector<int>  out;
      auto end = move(l, std::back_inserter(out));
      ensure(10 == out.size());
      ensure(out == std::vector<int>{0,1,2,3,4,5,6,7,8,9});

      *end = 0;
      ensure(11 == out.size());
      ensure(out == std::vector<int>{0,1,2,3,4,5,6,7,8,9,0});
   }

   template<>
   template<>
   void test_intance::test<4>()
   {
      std::array<only_movable,3> arr;
      arr[0] = only_movable{};
      arr[1] = only_movable{};
      arr[2] = only_movable{};

      ensure(3 == arr.size());
      for(auto const& i : arr)
         ensure(1==i.count_);

      std::list<only_movable> l;
      move(arr,std::back_inserter(l));
      for (auto const& i : l)
         ensure(2 == i.count_);
   }

   template<>
   template<>
   void test_intance::test<5>()
   {
      std::queue<only_movable> q;
      q.push(only_movable{});
      q.push(only_movable{});
      q.push(only_movable{});
      ensure(3 == q.size());

      std::list<only_movable> l;
      move(q, std::back_inserter(l));
      ensure(3 == l.size());
      for (auto const& i : l)
         ensure(2 == i.count_);
   }


} // namespace tut

