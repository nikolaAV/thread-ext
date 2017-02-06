#include <te_hierarchical_mutex.h>
#include "tut.h"


namespace
{
   struct data 
   {
      data()                        = default;
      data(const data&)             = delete;
      data& operator=(const data&)  = delete;

      using hierarchical_mutex   = thread_ex::hierarchical_mutex;
      using lock_guard           = std::lock_guard<hierarchical_mutex>;

      hierarchical_mutex m1000   {1000};
      hierarchical_mutex m100    {100};
      hierarchical_mutex m10     {10};

      void low_level()
      {
         lock_guard l(m10);
         // ...
      }

      void middle_level()
      {
         lock_guard l(m100);
         // ...
         low_level();   // <-- OK, higher layer calls lower one
      }

      void high_level()
      {
         lock_guard l(m1000);
         // ...
         middle_level();   // <-- OK, higher layer calls lower one
      }

      void rude_level() // <-- bad specified by design
      {
         lock_guard l(m100);
         // ...
         high_level();   // <-- not allowed, lower layer calls higher one
      }

   };
   using test_group = tut::test_group<data>;
   using test_intance = test_group::object;
   test_group tg("hierarchical_mutex");

} // end of anonymous namespace

namespace tut
{

template<>
template<>
void test_intance::test<1>()
{
   data::high_level();
}

template<>
template<>
void test_intance::test<2>()
{
   try
   {  
       data::rude_level();
      ensure("in right way this code is not reachable", false);
   }
   catch (const thread_ex::hierarchical_lock_error& e)
   {
      ensure_equals(e.current_level(),    100U);
      ensure_equals(e.requested_level(),  1000U);
   }
}

} // namespace 'tut'
