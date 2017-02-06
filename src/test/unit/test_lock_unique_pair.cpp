#include <te_lock_unique_pair.h>
#include <cassert>
#include "tut.h"

namespace
{
   struct mutex_stub
   {
      mutex_stub() : count_(0) {}

      void lock()       { ++count_; }
      void unlock()     { if(0 < count_) --count_; }

      bool try_lock()
      {
         if (0 == count_)
         {
            ++count_;
            return true;
         }
        return false;
      }

      size_t lock_count() const noexcept { return count_; }

   private:
      size_t count_;
   };

   using thread_ex::lock_unique_pair;
   using thread_ex::unique_pair;
   using unique_lock_stub        = std::unique_lock<mutex_stub>;
   using unique_lock_pair_stub   = unique_pair<unique_lock_stub,unique_lock_stub>;

   void do_copy(unique_lock_pair_stub p)
   {
      // ....
      p.first.unlock();
      p.second.unlock();
   }

   void take_ownership(unique_lock_pair_stub&& p)
   {
      // ....
      p.first.unlock();
      p.first.lock();
   }


   struct data{};
   using test_group = tut::test_group<data>;
   using test_intance = test_group::object;
   test_group tg("lock_unique_pair");

} // end of anonymous namespace

namespace tut
{

   template<>
   template<>
   void test_intance::test<1>()
   {
      mutex_stub m1, m2;
      ensure(0== m1.lock_count() && 0 == m2.lock_count());
      {
         auto l = lock_unique_pair(m1,m2);
         ensure(1 == m1.lock_count() && 1 == m2.lock_count());
         l.first.unlock();
         ensure(0 == m1.lock_count() && 1 == m2.lock_count());
      }
      ensure(0 == m1.lock_count() && 0 == m2.lock_count());

   }

//   template<>
//   template<>
//   void test_intance::test<2>()
//   {
//      mutex_stub m1, m2;
//      auto l = lock_unique_pair(m1, m2);
//      do_copy(l); // compiler error: l is not copyable 
//   }

   template<>
   template<>
   void test_intance::test<3>()
   {
      mutex_stub m1, m2;
      take_ownership(lock_unique_pair(m1, m2));
      ensure(0 == m1.lock_count() && 0 == m2.lock_count());
   }

} // namespace: tut 


