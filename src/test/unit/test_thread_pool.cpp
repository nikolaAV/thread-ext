#include "tut.h"
#include <te_thread_pool.h>
#include <te_block_lock.h>
#include <chrono>
#include <map>
#include <numeric>
#include <iterator>

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
      const auto SUBMISSIONS = 10000;

      auto l = [&m,&ids]() { lock(m,[&]{
         ++(ids[this_thread::get_id()]);
         this_thread::yield();
      });};

      {  thread_pool tp;   
         ensure( std::thread::hardware_concurrency()==tp.thread_count());

         for(size_t i=0; i < SUBMISSIONS; ++i)
            tp.submit(std::ref(l));    // returned future is ignored, it's ok . This behavious conforms to std::future<> returned by std::packaged_task
      }  // <-- behalf of destructor, stop() will be invoked

      size_t total_task_completed = 0;
      for(const auto& i : ids)
      {
//         cout << i.first << " -> " << i.second << endl;
         total_task_completed += i.second;
      }

      ensure( SUBMISSIONS==total_task_completed);
   }


   template<>
   template<>
   void test_instance::test<2>()
   {
      set_test_name ("termination");
      mutex m;
      map<thread::id,size_t> ids;
      const auto SUBMISSIONS = 10000;

      auto l = [&m,&ids]() { lock(m,[&]{
         ++(ids[this_thread::get_id()]);
         this_thread::yield();
      });};

      {  thread_pool tp;   
         ensure( std::thread::hardware_concurrency()==tp.thread_count());

         for(size_t i=0; i < SUBMISSIONS; ++i)
            tp.submit(std::ref(l));
         tp.terminate();
      }

      size_t total_task_completed = 0;
      for(const auto& i : ids)
         total_task_completed += i.second;
      ensure( SUBMISSIONS>=total_task_completed);
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

      struct value
      {
         value()  = default;
         value(size_t n)       : n_(n) {}
         value(const value& v) : n_copy_(1+v.n_copy_), n_move_(v.n_move_), n_(v.n_) {}
         value& operator=(const value& v) { n_copy_=1+v.n_copy_; n_move_=v.n_move_; n_=v.n_; return *this; }
         value(value&& v)      : n_copy_(v.n_copy_), n_move_(1+v.n_move_), n_(v.n_) {}
         value& operator=(value&& v)      { n_copy_=v.n_copy_; n_move_=1+v.n_move_; n_=v.n_; return *this; }

         size_t n_copy_ = 0;
         size_t n_move_ = 0;
         size_t n_      = 0;

         void print() const
         {
            cout << "{c:" << n_copy_ << ",m:" << n_move_ << ",n:" << n_ << "}" << endl;
         }
      };


   template<>
   template<>
   void test_instance::test<4>()
   {
      set_test_name ("value, rrvalue & rlvalue");

      thread_pool tp;
      {
         auto  func = [](value v) { v.n_++; return v; };
         value v {1};

         auto res1 = tp.submit( func, v);          // {1,9}  1 copying to pass v into 'submit', then this copy will be moved into 'func'  
         auto res2 = tp.submit( func, value{});    // {0,10} no copying at all
         auto res3 = tp.submit( func, ref(v));     // {1,3}  1 copying to pass v into 'func'

         const auto& v1 = res1.get();
         const auto& v2 = res2.get();
         const auto& v3 = res3.get();

         ensure(1==v1.n_copy_);
         ensure(0==v2.n_copy_);
         ensure(v1.n_copy_ + v1.n_move_==v2.n_copy_ + v2.n_move_);
         ensure(1==v3.n_copy_);
         ensure(1==v.n_);
      }

      {
         auto  func = [](value& v) { v.n_++; return v; };
         value v {1};

      //   auto res1 = tp.submit( func, v);        compiler error
      //   auto res2 = tp.submit( func, value{});  compiler error
         auto res3 = tp.submit( func, ref(v));     // 1 copying to return copy of reference v

         const auto& v3 = res3.get();
         ensure(1==v3.n_copy_);
         ensure(1==v3.n_copy_);
         ensure(2==v.n_);
      }

   }

   template<>
   template<>
   void test_instance::test<5>()
   {
      set_test_name ("natural numbers");  // https://en.wikipedia.org/wiki/1_%2B_2_%2B_3_%2B_4_%2B_%E2%8B%AF

      using value_type     = size_t;
      using container_type = std::vector<value_type>;
      using InputIt        = typename container_type::iterator;
      using future_type    = std::future<value_type>;
      using results_type   = std::vector<future_type>;
      using range_type     = std::pair<InputIt,InputIt>;

      constexpr size_t packet_size     = 1000;
      constexpr size_t packet_number   = 10;
      constexpr size_t N               = packet_number*packet_size;

      container_type v(N);
      std::iota(begin(v), end(v), 1u);

      thread_pool   tp;
      auto accumulate = [&tp](range_type r){
         return tp.submit(std::accumulate<typename range_type::first_type,size_t>,r.first,r.second,0u);
      };

      auto i = begin(v);
      results_type  partial_sums;
      while(i !=end(v))
      {
         const auto range = make_pair(i,next(i,packet_size));
         partial_sums.push_back(accumulate(range));
         i = range.second;
      }

      const auto sum = std::accumulate(begin(partial_sums),end(partial_sums),0u,[](auto& a, auto& b){
            return a+b.get();
      });

      ensure(sum==N*(N+1)/2);
   }

} // namespace tut

