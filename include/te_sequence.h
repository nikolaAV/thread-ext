#ifndef _THREAD_EX_TREADSAFE_SEQUENCE_INCLUDED_
#define _THREAD_EX_TREADSAFE_SEQUENCE_INCLUDED_

/**
	\file 	te_sequence.h
	\brief  	some usefull thread primitives which are not included into std (since C++11) 
	\author 	Alexander Nikolayenko
	\date		2017-05-13
	\copyright 	GNU Public License.
*/


#include "te_compiler_warning_suppress.h"
#include <vector>
#include <list>
#include <algorithm>
#include <iterator>
#include "te_compiler_warning_rollback.h"
#include "te_container.h"
#include "te_compiler.h"

/**
   \brief  a generic sequential container interface with no race conditions

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
      , typename STL_CONTAINER_T
      , typename MUTEX_T = std::mutex
      >
   class sequence_wrap : mutex_wrap<VALUE_T,STL_CONTAINER_T,MUTEX_T>
   {
   protected:
      using base_type         = mutex_wrap<VALUE_T, STL_CONTAINER_T, MUTEX_T>;
      using mutex_type        = typename base_type::mutex_type;
      using this_type         = sequence_wrap<VALUE_T, STL_CONTAINER_T, MUTEX_T>;
      using lock_guard_type   = typename base_type::lock_guard_type;
      using base_type::mutex_;
      using base_type::container_;

   public:
      using  container_type   = typename base_type::container_type;
      using  size_type        = typename container_type::size_type;
      using  value_type       = typename container_type::value_type;
      using  ptr_value_type   = typename base_type::ptr_value_type;
      using  pair_result_type = typename std::pair<bool,value_type>;

      sequence_wrap()                           = default;
      sequence_wrap(const this_type&)           = default;
      sequence_wrap(this_type&&)                = default;
      sequence_wrap(container_type&&);
      this_type& operator= (const this_type&)   = default;
      this_type& operator= (this_type&&)        = default;
      this_type& operator= (container_type&&);

      void              push_back(value_type&&);
      void              push_back(const value_type&);

      ptr_value_type    pop_back();                                    // <null> returned if the container is empty;
      void              pop_back(value_type& out);                     // throw (empty_error);
      bool              pop_back(std::nothrow_t, value_type& out);     // false returned if the container is empty

      void              swap(this_type& other);
      void              swap(container_type& other);
      bool              empty() const;
      size_type         size() const;

      bool  operator==(const this_type& other) const;
      bool  operator <(const this_type& other) const;

         // @retval
         //    {false, value_type()==null }        - element was not found
         //    {true,  element copy}               - the first element in the sequence that satisfies specific criteria defined by UnaryPredicate 
      template <typename UnaryPredicate>
         // UnaryPredicate - lambda or functor with the signature: bool(const value_type&)
      pair_result_type find_first(UnaryPredicate) const;
      template <typename UnaryPredicate>
      pair_result_type find_last(UnaryPredicate) const;

         // Copies the elements in the sequence, to 'other' container
         // returns the number of elements that have just been copied
      template <typename UnaryPredicate>
      size_t copy(container_type& other, UnaryPredicate) const;

         // Applies the given UnaryFunction to the every element in the sequence, in order.
         // returns function object of UnaryFunction type
      template <typename UnaryFunction> // void fun(const Type &a);
      UnaryFunction for_each(UnaryFunction) const;

         // non-const version of 'for_each'
         // the every element = UnaryFunction(...);
      template <typename UnaryFunction> // Ret fun(const Type &a);
      void transform(UnaryFunction);

         // Removes the elements in the sequence, for every of which, a criteria ('UnaryPredicate') is true
         // returns the number of removed elements
      template <typename UnaryPredicate>
      size_t erase(UnaryPredicate);
         // Removes all elements from the sequence and returns the number of removed elements
      size_t clear();

        // @retval
         //    {false, value_type()==null }        - element was not found
         //    {true,  element copy}               - copy of the element before modification 
      template <typename UnaryPredicateExpected, typename UnaryFuncDesired>
         // UnaryPredicateExpected  - lambda or functor with the signature: bool(const value_type&)
         // UnaryFuncDesired        - lambda or functor with the signature: void(value_type& 'in_out'), returns new value_type in 'in_out'
      pair_result_type compare_exchange(UnaryPredicateExpected, UnaryFuncDesired);


protected:
      template <typename Function, typename... Args>
      inline 
      typename std::result_of<Function(Args...)>::type // any function, any parameters
      call_under_lock(Function&& f, Args... args);

      template <typename Function, typename... Args>
      inline 
      typename std::result_of<Function(Args...)>::type
      call_under_lock(Function&& f, Args... args) const;
   };

   /**
   sequence_wrap function-member implementation
   */

   template <typename V, typename C, typename M>
   inline
   sequence_wrap<V, C, M>::sequence_wrap(container_type&& other)
      : base_type(std::move(other))
   {
   }

   template <typename V, typename C, typename M>
   inline
   sequence_wrap<V, C, M>&
   sequence_wrap<V, C, M>::operator=(container_type&& other)
   {
      static_cast<base_type&>(*this) = std::move(other);
      return *this;
   }

   template <typename V, typename C, typename M>
   inline
   bool
   sequence_wrap<V, C, M>::operator==(const this_type& other) const
   {
      return base_type::operator==(other);
   }

   template <typename V, typename C, typename M>
   inline
   bool
   sequence_wrap<V, C, M>::operator<(const this_type& other) const
   {
      return base_type::operator<(other);
   }

   template <typename V, typename C, typename M>
   inline
   typename sequence_wrap<V, C, M>::ptr_value_type
   sequence_wrap<V, C, M>::pop_back()
   {
      return thread_ex::pop<last_element>(container_, mutex_);
   }

   template <typename V, typename C, typename M>
   inline
   void
   sequence_wrap<V, C, M>::pop_back(value_type& out)
   {
      return thread_ex::pop<last_element>(container_, out, mutex_);
   }

   template <typename V, typename C, typename M>
   inline
   bool
   sequence_wrap<V, C, M>::pop_back(std::nothrow_t n, value_type& out)
   {
      return thread_ex::pop<last_element>(n, container_, out, mutex_);
   }

   template <typename V, typename C, typename M>
   inline
   void
   sequence_wrap<V, C, M>::push_back(value_type&& v)
   {
      call_under_lock([&](){
         container_.push_back(std::move(v));
      });
   }

   template <typename V, typename C, typename M>
   inline
   void
   sequence_wrap<V, C, M>::push_back(const value_type& v)
   {
      call_under_lock([&](){
         container_.push_back(v);
      });     
   }

   template <typename V, typename C, typename M>
   inline
   bool
   sequence_wrap<V, C, M>::empty() const
   {
      return base_type::empty();
   }

   template <typename V, typename C, typename M>
   inline
   typename sequence_wrap<V, C, M>::size_type
   sequence_wrap<V, C, M>::size() const
   {
      return base_type::size();
   }

   template <typename V, typename C, typename M>
   inline
   void
   sequence_wrap<V, C, M>::swap(this_type& other)
   {
      return base_type::swap(static_cast<base_type&>(other));
   }

   template <typename V, typename C, typename M>
   inline
   void
   sequence_wrap<V, C, M>::swap(container_type& other)
   {
      return base_type::swap(other);
   }

   template <typename V, typename C, typename M>
   template <typename Function, typename... Args>
   inline 
   typename std::result_of<Function(Args...)>::type
   sequence_wrap<V, C, M>::call_under_lock(Function&& f, Args... args)
   {
      std::lock_guard<mutex_type> hold{ mutex_ };
      return f(std::forward<Args>(args)...);
   }    

   template <typename V, typename C, typename M>
   template <typename Function, typename... Args>
   inline 
   typename std::result_of<Function(Args...)>::type
   sequence_wrap<V, C, M>::call_under_lock(Function&& f, Args... args) const
   {
      std::lock_guard<mutex_type> hold{ mutex_ };
      return f(std::forward<Args>(args)...);
   }    

   template <typename V, typename C, typename M>
   template <typename UnaryPredicate> // UnaryPredicate - lambda or functor with the signature: bool(const value_type&)
   inline
   typename sequence_wrap<V, C, M>::pair_result_type 
   sequence_wrap<V, C, M>::find_first(UnaryPredicate p) const
   {
      using namespace std;
      return call_under_lock([&]{
         auto i = find_if(begin(container_), end(container_),p);
         return (i!= end(container_))? make_pair(true,*i) : make_pair(false,value_type());
      });        
   }

   template <typename V, typename C, typename M>
   template <typename UnaryPredicate> // UnaryPredicate - lambda or functor with the signature: bool(const value_type&)
   inline
   typename sequence_wrap<V, C, M>::pair_result_type
   sequence_wrap<V, C, M>::find_last(UnaryPredicate p) const
   {
      using namespace std;
      return call_under_lock([&] {
         auto i = find_if(rbegin(container_), rend(container_), p);
         return (i != rend(container_)) ? make_pair(true, *i) : make_pair(false, value_type());
      });
   }

   template <typename V, typename C, typename M>
   template <typename UnaryPredicate>
   inline
   size_t
   sequence_wrap<V, C, M>::copy(container_type& other, UnaryPredicate f) const
   {
      using namespace std;
      using InputIt  = decltype(begin(container_));
      using OutputIt = decltype(back_inserter(other));

      call_under_lock(
         std::copy_if<InputIt, OutputIt, UnaryPredicate>,
         begin(container_),end(container_),back_inserter(other),f
      );
      return other.size();
   }

   template <typename V, typename C, typename M>
   template <typename UnaryPredicate>
   inline
   size_t
   sequence_wrap<V, C, M>::erase(UnaryPredicate f)
   {
      using namespace std;

      return
      call_under_lock([&](){
         auto pos = remove_if(begin(container_), end(container_),f);
         auto n = distance(pos, end(container_));
         container_.erase(pos, end(container_));
         return static_cast<size_t>(n);
      });
   }

   template <typename V, typename C, typename M>
   inline
   size_t
   sequence_wrap<V, C, M>::clear()
   {
      container_type tmp;
      swap(tmp);
      return tmp.size();
   }

   template <typename V, typename C, typename M>
   template <typename UnaryPredicateExpected, typename UnaryFuncDesired>
   inline
   typename sequence_wrap<V, C, M>::pair_result_type 
   sequence_wrap<V, C, M>::compare_exchange(UnaryPredicateExpected e, UnaryFuncDesired d)
   {
      return call_under_lock([&]{
         auto res = std::make_pair(false,value_type());
         auto i = std::find_if(begin(container_),end(container_),e);
         if(i==end(container_))
            return res; 
         res.first = true;
         res.second = *i; 
         d(*i);
         return res;
      });        
   }

   template <typename V, typename C, typename M>
   template <typename UnaryFunction>
   inline
   UnaryFunction 
   sequence_wrap<V, C, M>::for_each(UnaryFunction f) const
   {
      using InputIt = decltype(std::begin(container_));
      return
         call_under_lock(
             std::for_each<InputIt,UnaryFunction>
            ,std::begin(container_),std::end(container_),f
         );
   }

   template <typename V, typename C, typename M>
   template <typename UnaryFunction>
   inline
   void 
   sequence_wrap<V, C, M>::transform(UnaryFunction f)
   {
      using Iterator = decltype(std::begin(container_));
      call_under_lock(
            std::transform<Iterator,Iterator,UnaryFunction>
         ,std::begin(container_),std::end(container_),std::begin(container_),f
      );
   }

/**
   \brief  a vector with (1) no race conditions in the interface and (2) each public method is atomic
   \example test_threadsafe_vector.cpp & test_threadsafe_list.cpp
*/

template <typename VALUE_T, typename MUTEX_T = std::mutex>
using threadsafe_vector = sequence_wrap<VALUE_T, std::vector<VALUE_T>, MUTEX_T>;

/**
   \brief  a list with (1) no race conditions in the interface and (2) each public method is atomic
   \example test_threadsafe_vector.cpp & test_threadsafe_list.cpp
*/

template <typename VALUE_T, typename MUTEX_T = std::mutex>
using threadsafe_list = sequence_wrap<VALUE_T, std::list<VALUE_T>, MUTEX_T>;


} // namespace thread_ex

#endif //_THREAD_EX_TREADSAFE_SEQUENCE_INCLUDED_

