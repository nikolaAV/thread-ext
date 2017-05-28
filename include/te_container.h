#ifndef _THREAD_EX_TREADSAFE_CONTAINER_INCLUDED_
#define _THREAD_EX_TREADSAFE_CONTAINER_INCLUDED_

/**
	\file 		te_container.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2012-02-10
	\copyright 	GNU Public License.
*/


#include "te_compiler_warning_suppress.h"
#include <queue>
#include <stack>
#include <mutex>
#include <condition_variable>
#include <utility>
#include "te_compiler_warning_rollback.h"
#include "te_compiler.h"
#include "te_lock_unique_pair.h"
#include "te_pop.h"
#include "te_block_lock.h"


/**
   \brief  a generic container interface with no race conditions

   It also implements options:
   - customized by concreate containet type in 2d template parameter 
   - passing a reference to a variable in which you wish to receive the
   popped value as an argument in the call to 'pop'
   - return a pointer to the popped item rather than return the item by value.
   The advantage here is that pointers can be freely copied without throwing an
   exception, so you’ve avoided Cargill’s exception problem
   http://ptgmedia.pearsoncmg.com/images/020163371x/supplements/Exception_Handling_Article.html
*/

namespace thread_ex
{

template 
<
    typename VALUE_T 
   ,typename STL_CONTAINER_T 
   ,typename MUTEX_T     = std::mutex
>
class mutex_wrap
{
protected:
   STL_CONTAINER_T         container_;
   mutable MUTEX_T         mutex_;

   using mutex_type        = MUTEX_T;
   using this_type         = mutex_wrap<VALUE_T, STL_CONTAINER_T, MUTEX_T>;
   using lock_guard_type   = std::lock_guard<mutex_type>;
   using unique_lock_type  = std::unique_lock<mutex_type>;

public:
   using  container_type   = STL_CONTAINER_T;
   using  size_type        = typename container_type::size_type;
   using  value_type       = typename container_type::value_type;
   using  ptr_value_type   = std::unique_ptr<value_type>;

   mutex_wrap();
   mutex_wrap(const this_type&);
   mutex_wrap(this_type&&);
   mutex_wrap(container_type&&);
   this_type& operator= (const this_type&);
   this_type& operator= (this_type&&);
   this_type& operator= (container_type&&);

   void              push(value_type&&);
   void              push(const value_type&);

   ptr_value_type    pop();                                    // <null> returned if the container is empty
   void              pop(value_type& out);                     // throw (empty_error);
   void              pop(container_type& out);                 // throw (empty_error);
   bool              pop(std::nothrow_t, value_type& out);     // false returned if the container is empty
   bool              pop(std::nothrow_t, container_type& out); // all elements are moved into out and 'true' returned, 'false' if empty

   void              swap(this_type& other);
   void              swap(container_type& other);
   bool              empty() const;
   size_type         size() const;

   bool  operator==(const this_type& other) const;
   bool  operator <(const this_type& other) const;

}; // class mutex_wrap


template
<
    typename VALUE_T
   ,typename STL_CONTAINER_T
   ,typename MUTEX_T = std::mutex
>
class condition_wrap : mutex_wrap<VALUE_T, STL_CONTAINER_T, MUTEX_T>
{
   using base_type         = mutex_wrap<VALUE_T, STL_CONTAINER_T, MUTEX_T>;
   using this_type         = condition_wrap<VALUE_T, STL_CONTAINER_T, MUTEX_T>;
   using condition_type    = std::condition_variable;
   using unique_lock_type  = typename base_type::unique_lock_type;

   condition_type cond_;

public:
   using container_type    = typename base_type::container_type;
   using value_type        = typename base_type::value_type;
   using ptr_value_type    = typename base_type::ptr_value_type;
   using size_type         = typename base_type::size_type;

public:
   condition_wrap ();
   condition_wrap (const this_type&);
   condition_wrap (this_type&&);
   condition_wrap(container_type&&);
   this_type& operator= (const this_type&);
   this_type& operator= (this_type&&);
   this_type& operator= (container_type&&);

   void              push(value_type&&);
   void              push(const value_type&);

   ptr_value_type    try_pop();                                   // <null> returned if the container is empty
   void              try_pop(value_type& out);                    // throw (empty_error);
   void              try_pop(container_type& out);                // throw (empty_error);
   bool              try_pop(std::nothrow_t, value_type& out);    // false returned if the container is empty
   bool              try_pop(std::nothrow_t, container_type& out);// all elements are moved into out and 'true' returned, 'false' if empty

   ptr_value_type    wait_pop();                                  // if the container is empty, it waits until an element is pushed by other thread 
   void              wait_pop(value_type& out);                   // waits (if needed) and pops one element
   void              wait_pop(container_type& out);               // waits (if needed) and pops all elements

   void              swap(this_type& other);
   using             base_type::empty;
   using             base_type::size;
   using             base_type::swap;

   bool  operator==(const this_type& other) const;
   bool  operator <(const this_type& other) const;

}; // class condition_wrap

/**
      mutex_wrap function-member implementation
*/
template <typename V, typename C,typename M>
inline
mutex_wrap<V,C,M>::mutex_wrap()
{
}

template <typename V, typename C, typename M>
inline
mutex_wrap<V,C,M>::mutex_wrap(const this_type& other)
{
   block::lock(other.mutex_,[&]{
      container_ = other.container_;
   });
}

template <typename V, typename C, typename M>
inline
mutex_wrap<V,C,M>::mutex_wrap(this_type&& other)
   : container_(std::move(other.container_))
{
}

template <typename V, typename C, typename M>
inline
mutex_wrap<V, C, M>::mutex_wrap(container_type&& other)
   : container_(std::move(other))
{
}

template <typename V, typename C, typename M>
inline
mutex_wrap<V,C,M>& 
mutex_wrap<V,C,M>::operator= (const this_type& other)
{
   if (this == &other)
      return *this;

   block::lock(mutex_,other.mutex_,[&]{
      container_ = other.container_;
   });
   return *this;
}

template <typename V, typename C, typename M>
inline
mutex_wrap<V,C,M>&
mutex_wrap<V,C,M>::operator= (this_type&& other)
{
   block::lock(mutex_,[&]{
      container_ = std::move(other.container_);
   });
   return *this;
}

template <typename V, typename C, typename M>
inline
mutex_wrap<V, C, M>&
mutex_wrap<V, C, M>::operator= (container_type&& other)
{
   block::lock(mutex_, [&] {
      container_ = std::move(other);
   });
   return *this;
}

template <typename V, typename C, typename M>
inline
bool
mutex_wrap<V,C,M>::empty() const
{
   lock_guard_type l(mutex_);
   return container_.empty();
}

template <typename V, typename C, typename M>
inline
typename mutex_wrap<V,C,M>::size_type
mutex_wrap<V,C,M>::size() const
{
   lock_guard_type l(mutex_);
   return container_.size();
}


template <typename V, typename C, typename M>
inline
void
mutex_wrap<V,C,M>::push(value_type&& v)
{
   block::lock(mutex_,[&]{
      container_.push(std::move(v));
   });
}

template <typename V, typename C, typename M>
inline
void
mutex_wrap<V,C,M>::push(const value_type& v)
{
   block::lock(mutex_,[&]{
      container_.push(v);
   });
}

template <typename V, typename C, typename M>
inline
void
mutex_wrap<V,C,M>::pop(value_type& out)
{
   thread_ex::pop<first_element>(container_, out, mutex_);
}

template <typename V, typename C, typename M>
inline
void
mutex_wrap<V,C,M>::pop(container_type& out)
{
   container_type zero;
   {  lock_guard_type l(mutex_);
      if (container_.empty()) throw empty_error();
      container_.swap(zero);
   }
   out.swap(zero);
}

template <typename V, typename C, typename M>
inline
typename mutex_wrap<V,C,M>::ptr_value_type  
mutex_wrap<V,C,M>::pop()
{
   return thread_ex::pop<first_element>(container_, mutex_);
}

template <typename V, typename C, typename M>
inline
bool
mutex_wrap<V,C,M>::pop(std::nothrow_t n, value_type& out)
{
   return thread_ex::pop<first_element>(n, container_, out, mutex_);
}

template <typename V, typename C, typename M>
inline
bool
mutex_wrap<V,C,M>::pop(std::nothrow_t, container_type& out)
{
   container_type zero;
   {  lock_guard_type l(mutex_);
      if (container_.empty()) return false;
      container_.swap(zero);
   }
   out.swap(zero);
   return true;
}

template <typename V, typename C, typename M>
inline
void
mutex_wrap<V,C,M>::swap(this_type& other)
{
   if (this == &other)
      return;

   auto l = thread_ex::lock_unique_pair(mutex_, other.mutex_);
   container_.swap(other.container_);
}

template <typename V, typename C, typename M>
inline
void
mutex_wrap<V,C,M>::swap(container_type& other)
{
   unique_lock_type l(mutex_);
   container_.swap(other);
}

template <typename V, typename C, typename M>
inline
bool  
mutex_wrap<V,C,M>::operator==(const this_type& other) const
{
   if (this == &other)
      return true;

   auto l = thread_ex::lock_unique_pair(mutex_, other.mutex_);
   return container_ == other.container_;
}

template <typename V, typename C, typename M>
inline
bool  
mutex_wrap<V,C,M>::operator<(const this_type& other) const
{
   if (this == &other)
      return false;

   auto l = thread_ex::lock_unique_pair(mutex_, other.mutex_);
   return container_ < other.container_;
}

/**
   condition_wrap function-member implementation
*/
template <typename V, typename C, typename M>
inline
condition_wrap<V,C,M>::condition_wrap() : base_type()
{
}

template <typename V, typename C, typename M>
inline
condition_wrap<V,C,M>::condition_wrap(const this_type& other) : base_type(other)
{
}

template <typename V, typename C, typename M>
inline
condition_wrap<V,C,M>::condition_wrap(this_type&& other) : base_type(std::move(other))
{
}

template <typename V, typename C, typename M>
inline
condition_wrap<V, C, M>::condition_wrap(container_type&& other) : base_type(std::move(other))
{
}

template <typename V, typename C, typename M>
inline
condition_wrap<V,C,M>&
condition_wrap<V,C,M>::operator= (const this_type& other)
{
   static_cast<base_type&>(*this) = other;
   return *this;
}

template <typename V, typename C, typename M>
inline
condition_wrap<V,C,M>&
condition_wrap<V,C,M>::operator= (this_type&& other)
{
   static_cast<base_type&>(*this) = std::move(other);
   return *this;
}

template <typename V, typename C, typename M>
inline
condition_wrap<V, C, M>&
condition_wrap<V, C, M>::operator= (container_type&& other)
{
   static_cast<base_type&>(*this) = std::move(other);
   return *this;
}

template <typename V, typename C, typename M>
inline
void
condition_wrap<V,C,M>::push(value_type&& v)
{
   base_type::push(std::move(v));
   cond_.notify_one();
}

template <typename V, typename C, typename M>
inline
void
condition_wrap<V,C,M>::push(const value_type& v)
{
   base_type::push(v);
   cond_.notify_one();
}

template <typename V, typename C, typename M>
inline
void
condition_wrap<V,C,M>::try_pop(value_type& out)
{
   base_type::pop(out);
}

template <typename V, typename C, typename M>
inline
void
condition_wrap<V,C,M>::try_pop(container_type& out)
{
   base_type::pop(out);
}

template <typename V, typename C, typename M>
inline
typename condition_wrap<V,C,M>::ptr_value_type
condition_wrap<V,C,M>::try_pop()
{
   return base_type::pop();
}

template <typename V, typename C, typename M>
inline
bool
condition_wrap<V,C,M>::try_pop(std::nothrow_t n, value_type& out)
{
   return base_type::pop(n,out);
}

template <typename V, typename C, typename M>
inline
bool
condition_wrap<V,C,M>::try_pop(std::nothrow_t n, container_type& out)
{
   return base_type::pop(n, out);
}

template <typename V, typename C, typename M>
inline
void
condition_wrap<V,C,M>::wait_pop(value_type& out)
{
   unique_lock_type l(base_type::mutex_);
   container_type& cont =  base_type::container_;
   if (cont.empty())
      cond_.wait(l, [&cont] { return !cont.empty(); });
   thread_ex::pop<first_element>(cont,out);
}


template <typename V, typename C, typename M>
inline
typename condition_wrap<V,C,M>::ptr_value_type
condition_wrap<V,C,M>::wait_pop()
{
   auto out = make_unique<value_type>(value_type{});
   wait_pop(*out);
   return out;
}

template <typename V, typename C, typename M>
inline
void
condition_wrap<V,C,M>::wait_pop(container_type& out)
{
   unique_lock_type l(base_type::mutex_);
   container_type& cont =  base_type::container_;
   if (cont.empty())
      cond_.wait(l, [&cont] { return !cont.empty(); });
   out.swap(cont);
}

template <typename V, typename C, typename M>
inline
void
condition_wrap<V,C,M>::swap(this_type& other)
{
   base_type::swap(other);
}

template <typename V, typename C, typename M>
inline
bool
condition_wrap<V,C,M>::operator==(const this_type& other) const
{
   return base_type::operator==(other);
}

template <typename V, typename C, typename M>
inline
bool
condition_wrap<V,C,M>::operator<(const this_type& other) const
{
   return base_type::operator<(other);
}


/**
   \brief  a stack with (1) no race conditions in the interface and (2) each public method is atomic
   \remark "C++ Concurrency in Action", Anthony Williams, chapter 3.2.3, page 45
   \example test_threadsafe_stack.cpp & test_threadsafe_stack.cpp
*/

template <typename VALUE_T, typename MUTEX_T = std::mutex>
using threadsafe_stack = mutex_wrap<VALUE_T, std::stack<VALUE_T>, MUTEX_T>;

/**
   \brief  a queue with (1) no race conditions in the interface, (2) each public method is atomic and (3) notification mechaism integrated 
   \remark "C++ Concurrency in Action", Anthony Williams, chapter 4.1.2 page 71
   \example test_threadsafe_queue.cpp & test_threadsafe_stack.cpp
*/

template <typename VALUE_T, typename MUTEX_T = std::mutex>
using threadsafe_queue = condition_wrap<VALUE_T, std::queue<VALUE_T>, MUTEX_T>;


} // namespace thread_ex

#endif //_THREAD_EX_TREADSAFE_CONTAINER_INCLUDED_

