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
   class sequence_wrap : private mutex_wrap<VALUE_T,STL_CONTAINER_T,MUTEX_T>
   {
   protected:
      using base_type         = mutex_wrap<VALUE_T, STL_CONTAINER_T, MUTEX_T>;
      using mutex_type        = typename base_type::mutex_type;
      using this_type         = sequence_wrap<VALUE_T, STL_CONTAINER_T, MUTEX_T>;
      using lock_guard_type   = typename base_type::lock_guard_type;

   public:
      using  container_type   = typename base_type::container_type;
      using  size_type        = typename container_type::size_type;
      using  value_type       = typename container_type::value_type;
      using  ptr_value_type   = typename base_type::ptr_value_type;
      using  pair_result_type = typename std::pair<bool,value_type>;

      sequence_wrap()                           = default;
      sequence_wrap(const this_type&)           = default;
      sequence_wrap(this_type&&)                = default;
      this_type& operator= (const this_type&)   = default;
      this_type& operator= (this_type&&)        = default;

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
      //    {true,  element copy}
      template <typename KEY>
         // requires
         // bool KEY             ::operator()(const value_type&) - criteria which element is looking for in the container
      pair_result_type    find(KEY);

         // @retval
         //    {false, value_type()==null }        - element was not found
         //    {false, element in the container}   - no modification, EXPECTED returned false
         //    {true,  element in the container}   - data element before modification
      template <typename KEY, typename EXPECTED, typename VAL_EXPR>
         // requires
         // bool KEY             ::operator()(const value_type&) - criteria which element is looking for in the container
         // bool EXPECTED        ::operator()(const value_type&) - criteria what data element is acceptable 
         // value_type VAL_EXPR  ::operator()(const value_type&) - new data for this element
      pair_result_type    find_compare_exchange(KEY, EXPECTED, VAL_EXPR);

   };

   /**
   sequence_wrap function-member implementation
   */

   template <typename V, typename C, typename M>
   inline
   bool
   sequence_wrap<V, C, M>::operator==(const this_type& other) const
   {
      if (this == &other)
         return true;

      auto l = thread_ex::lock_unique_pair(mutex_, other.mutex_);
      return container_ == other.container_;
   }

   template <typename V, typename C, typename M>
   inline
   bool
   sequence_wrap<V, C, M>::operator<(const this_type& other) const
   {
      if (this == &other)
         return false;

      auto l = thread_ex::lock_unique_pair(mutex_, other.mutex_);
      return container_ < other.container_;
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
      block::lock(mutex_, [&] {
         container_.push_back(std::move(v));
      });
   }

   template <typename V, typename C, typename M>
   inline
   void
   sequence_wrap<V, C, M>::push_back(const value_type& v)
   {
      block::lock(mutex_, [&] {
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
   template <typename KEY, typename EXPECTED, typename VAL_EXPR>
   inline
   typename sequence_wrap<V, C, M>::pair_result_type    
   sequence_wrap<V, C, M>::find_compare_exchange(KEY key, EXPECTED what, VAL_EXPR how)
   {
      auto res = std::make_pair(false, value_type{});
      block::lock(mutex_, [&] {
        auto i = std::find_if(container_.begin(), container_.end(), key);
        if(i== container_.end())
           return;

        res.first = true; res.second = *i;
        if(res.first)
         *i = how(*i);
      });
      return res;
   }

   template <typename V, typename C, typename M>
   template <typename KEY>
   inline
   typename sequence_wrap<V, C, M>::pair_result_type
   sequence_wrap<V, C, M>::find(KEY key)
   {
      return find_compare_exchange(
         [&](const value_type& v) { return key(v); }
         ,[](const value_type&)    { return true; }
         ,[](const value_type& v)  { return v; }
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

