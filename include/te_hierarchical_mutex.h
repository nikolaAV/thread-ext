#ifndef _THREAD_EX_HIERARCHICAL_MUTEX_INCLUDED_
#define _THREAD_EX_HIERARCHICAL_MUTEX_INCLUDED_

/**
	\file 		te_hierarchical_mutex.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2012-02-10
	\copyright 	GNU Public License.
*/


#include "te_compiler_warning_suppress.h"
#include <mutex>
#include <cassert>
#include <stdexcept>
#include <limits>
#include "te_compiler_warning_rollback.h"
#include "te_compiler.h"

/**
   \brief Lock hierarchy to prevent deadlock

   The guidelines for avoiding deadlock all boil down to one idea:
   don’t wait for another thread if there’s a chance it’s waiting for you.
   The individual guidelines provide ways of identifying and eliminating the possibility that the other thread is waiting for you.
   - avoid nested locks
   - avoid calling user-supplied code while holding a lock
   - acquire locks in a fixed order

   \remark "C++ Concurrency in Action", Anthony Williams, chapter 3.2.5, page 51
   \example test_hierarchical_mutex.cpp
*/

namespace thread_ex
{

/**
   Lock hierarchy is really a particular case of defining lock ordering, 
   a lock hierarchy can provide a means of checking that the convention is adhered to at runtime. 
   The idea is that you divide your application into layers and identify all the mutexes
   that may be locked in any given layer.
   When code tries to lock a mutex, it isn’t permitted to lock that mutex if it already holds a lock from a lower layer. 
   You can check this at runtime by assigning layer numbers to each mutex 
   and keeping a record of which mutexes are locked by each thread
 */

class hierarchical_lock_error : public std::logic_error
{
   size_t current_level_;
   size_t requested_level_;
public:
   hierarchical_lock_error(size_t cl, size_t rl) : 
      std::logic_error("mutex hierarchy violation")
      , current_level_(cl)
      , requested_level_(rl)
   {
      assert(current_level_<requested_level_ && "[hierarchical_lock_error] logic_error");
   }
   size_t current_level()     const noexcept { return current_level_; }
   size_t requested_level()   const noexcept { return requested_level_; }
};

struct hierarchical_error_throw
{
   void operator()(size_t cl, size_t rl) const { throw hierarchical_lock_error(cl,rl); }
};

struct hierarchical_error_assert
{
   void operator()(size_t, size_t) const noexcept { assert(!"mutex hierarchy violated"); }
};

template
<  
   typename MUTEX_T                 = std::mutex, 
   typename NOTIFICATION_POLICY_T   = hierarchical_error_throw
>
class hierarchical_mutex_basic
{
public:
   typedef MUTEX_T               mutex_type;         
   typedef NOTIFICATION_POLICY_T notify_type;

public:
   explicit hierarchical_mutex_basic(size_t value) :
       curr_hierarchy_value_(value)
      ,prev_hierarchy_value_(0)
      ,mutex_()
   {}

   template <typename ARG1_T>
   hierarchical_mutex_basic(size_t value, const ARG1_T& arg1) :
       curr_hierarchy_value_(value)
      ,prev_hierarchy_value_(0)
      ,mutex_(arg1)
   {}

   template <typename ARG1_T, typename ARG2_T>
   hierarchical_mutex_basic(size_t value, const ARG1_T& arg1, const ARG2_T& arg2) :
       curr_hierarchy_value_(value)
      ,prev_hierarchy_value_(0)
      ,mutex_(arg1,arg2)
   {}

   hierarchical_mutex_basic(const hierarchical_mutex_basic&) = delete;
   hierarchical_mutex_basic& operator=(const hierarchical_mutex_basic&) = delete;

	void lock()
	{
      check_for_violation();
      mutex_.lock();
      forward();
	}

	bool try_lock()
	{
      check_for_violation();
		if(mutex_.try_lock())
      {
         forward();
         return true;
      }
      return false;
	}

	void unlock()
	{
      backward();
		mutex_.unlock();
	}

	std::mutex::native_handle_type native_handle()
	{
	   return mutex_.native_handle();
	}

private:
   void forward() noexcept
   {
      prev_hierarchy_value_         = the_thread_hierarchy_value_;
      the_thread_hierarchy_value_   = curr_hierarchy_value_;
   }

   void backward() noexcept
   {
      the_thread_hierarchy_value_   = prev_hierarchy_value_;
   }

   void check_for_violation() const
   {
      if(the_thread_hierarchy_value_ <= curr_hierarchy_value_)
         notify_(the_thread_hierarchy_value_,curr_hierarchy_value_);
   }

private:
   static thread_local size_t             the_thread_hierarchy_value_;
   size_t const                           curr_hierarchy_value_;
   size_t                                 prev_hierarchy_value_;
   notify_type                            notify_;
   mutex_type                             mutex_;
};

template <typename MUTEX_T, typename NOTIFICATION_T>
thread_local size_t hierarchical_mutex_basic<MUTEX_T,NOTIFICATION_T>::the_thread_hierarchy_value_(std::numeric_limits<size_t>::max());


typedef hierarchical_mutex_basic<> hierarchical_mutex;

} // namespace thread_ex



#endif //_THREAD_EX_HIERARCHICAL_MUTEX_INCLUDED_

