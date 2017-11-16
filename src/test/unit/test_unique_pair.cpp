#include <te_unique_pair.h>
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
   size_t ConstructionCounter::copying_copy  = 0;
   size_t ConstructionCounter::moving_copy   = 0;

   using thread_ex::unique_pair;
   using thread_ex::make_unique_pair;

   struct data {};
   using test_group     = tut::test_group<data>;
   using test_intance   = test_group::object;
   test_group tg("unique_pair");

} // end of anonymous namespace


namespace tut
{

   template<>
   template<>
   void test_intance::test<1>()
   {
      unique_pair<int,int> p;
      const auto p2 = make_unique_pair(1, 2);
      ensure(1 == p2.first && 2 == p2.second);

      size_t val = 3;
      const auto p3 = make_unique_pair(val, val);
      ensure(val == p3.first && val == p3.second);
   }

   template<>
   template<>
   void test_intance::test<2>()
   {
      unique_pair<int,int> p;
      unique_pair<int,int> p2(1, 2);

      //p = p2;               // compiler error: deleted function
      //unique_pair_t p3(p);  // compiler error: deleted function
   }

   template<>
   template<>
   void test_intance::test<3>()
   {
      ConstructionCounter cc;
      ensure(0 == cc.copying_copy && 0 == cc.moving_copy);

      ConstructionCounter cc1 = cc;
      ensure(1 == cc1.copying_copy && 0 == cc1.moving_copy);

      ConstructionCounter cc2 = std::move(cc);
      ensure(1 == cc2.copying_copy && 1 == cc2.moving_copy);
   }

   template<>
   template<>
   void test_intance::test<4>()
   {
      ConstructionCounter cc1, cc2;
      ensure(0 == ConstructionCounter::copying_copy && 0 == ConstructionCounter::moving_copy);

      const auto up1 = make_unique_pair(cc1,cc2);
      ensure(2 == ConstructionCounter::copying_copy && 0 == ConstructionCounter::moving_copy);
      ensure(2 == up1.first.copying_copy && 0 == up1.second.moving_copy);
   }

   template<>
   template<>
   void test_intance::test<5>()
   {
      ConstructionCounter cc1, cc2;
      ensure(0 == ConstructionCounter::copying_copy && 0 == ConstructionCounter::moving_copy);

      const auto up1 = make_unique_pair(std::move(cc1), std::move(cc2));
      ensure(0 == ConstructionCounter::copying_copy && 2 == ConstructionCounter::moving_copy);
      ensure(0 == up1.first.copying_copy && 2 == up1.first.moving_copy);
   }

   template<>
   template<>
   void test_intance::test<6>()
   {
      ConstructionCounter cc;
      ensure(0 == ConstructionCounter::copying_copy && 0 == ConstructionCounter::moving_copy);

      const auto up1 = make_unique_pair(1, cc);
      ensure(1 == ConstructionCounter::copying_copy && 0 == ConstructionCounter::moving_copy);
      ensure(1 == up1.second.copying_copy && 0 == up1.second.moving_copy);

      //auto up2 = up1;   // compiler error: deleted function 
      //unique_pair<int, ConstructionCounter> up2;
      //up2 = up1;           // compiler error: deleted function 
   }

   template<>
   template<>
   void test_intance::test<7>()
   {
      unique_pair<int, ConstructionCounter> up3;
      ConstructionCounter cc;
      ensure(0 == ConstructionCounter::copying_copy && 0 == ConstructionCounter::moving_copy);

      auto up1 = make_unique_pair(1, std::move(cc));
      ensure(0 == ConstructionCounter::copying_copy && 1 == ConstructionCounter::moving_copy);

      auto up2 = std::move(up1);
      ensure(0 == ConstructionCounter::copying_copy && 2 == ConstructionCounter::moving_copy);

      up3 = std::move(up2);
      ensure(0 == ConstructionCounter::copying_copy && 3 == ConstructionCounter::moving_copy);
   }

   template<>
   template<>
   void test_intance::test<8>()
   {
      ConstructionCounter cc;

      auto up1 = make_unique_pair(cc,1);
      auto up2 = make_unique_pair(cc,2);
      ensure(2 == ConstructionCounter::copying_copy && 0 == ConstructionCounter::moving_copy);
      ensure(1 == up1.second && 2 == up2.second);

      swap(up1,up2);
      ensure(2 == up1.second && 1 == up2.second);
      ensure(2 == ConstructionCounter::copying_copy && 3 == ConstructionCounter::moving_copy);
 
   }

} // namespace tut

