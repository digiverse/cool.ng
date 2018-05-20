/*
 * Copyright (c) 2017 Leon Mlakar.
 * Copyright (c) 2017 Digiverse d.o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. The
 * license should be included in the source distribution of the Software;
 * if not, you may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * The above copyright notice and licensing terms shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#if !defined(cool_ng_41352af7_f2d7_4732_8200_beef75dc84b2)
#define      cool_ng_41352af7_f2d7_4732_8200_beef75dc84b2

#include <cstddef>
#include <memory>
#include <functional>
#include <type_traits>

// Visual Studio has broken std::is_copy_constructible trait
#if _MSC_VER == 1800
#include "boost/type_traits.hpp"
#endif

namespace cool { namespace ng {  namespace async {

class runner;

namespace detail {

// ---- ----
// ----  Helper class that is a replacement for boost::any - the latter
// ----  is not usable owing to its requirement for the value type to be
// ----  CopyConstructible, which was not working for tasks whose parameter
// ----  was MoveConstructible but not CopyConstructible.
// ----
// ----- Hence own implementation with relaxed requirements for the value type.
// ---- ----

class any
{
 private:
  class valueholder
  {
   public:
    virtual ~valueholder() { /* noop */ }
    virtual const std::type_info& type() const = 0;
    virtual std::unique_ptr<valueholder> clone() = 0;
  };

  template <typename ValueType>
  class value : public valueholder
  {
   public:
    value(const ValueType& v_) : m_v(v_)       { /* noop */ }
    value(ValueType&& v_) : m_v(std::move(v_)) { /* noop */ }
    const std::type_info& type() const override
    { return typeid(ValueType); }
    std::unique_ptr<valueholder> clone() override
    {
      return std::unique_ptr<value>(new value(std::move(m_v)));
    }
   private:
    template <typename T>
    friend T* any_cast(any*);
    ValueType m_v;
  };

 public:
  // -- ctors and assignments
  any()
  { /* noop */ }
  any(any&& other_) : m_value(std::move(other_.m_value))
  { /* noop */ }
  template <typename ValueT> any(const ValueT& value_)
    : m_value(new value<typename std::decay<ValueT>::type>(value_))
  { /* noop */ }
  template <typename ValueT> any(ValueT&& value_
    , typename std::enable_if<!std::is_const<ValueT>::value>::type* = 0)
        : m_value(new value<typename std::decay<ValueT>::type>(std::move(value_)))
  { /* noop */ }
  any& operator =(any&& other_)
  { m_value = std::move(other_.m_value); return *this; }
  template <typename ValueT>
  any& operator =(ValueT&& value_)
  {
    m_value.reset(new value<typename std::decay<ValueT>::type>(std::move(value_)));
    return *this;
  }
  any(const any& other_) : m_value( other_.m_value ? other_.m_value->clone() : nullptr)
  { /* noop */ }
  any& operator =(const any& other_)
  {
    any(other_).swap(*this);
    return *this;
  };

  // -- observers
  bool empty() const
  { return static_cast<bool>(m_value); }

  // -- modifiers
  any& swap(any& other_)
  { std::swap(m_value, other_.m_value); return *this; }
  void clear()
  { m_value.reset(); }

 private:
  template <typename T>
  friend T* any_cast(any*);
  std::unique_ptr<valueholder> m_value;
};

class bad_any_cast : public std::bad_cast
{
 public:
  const char* what() const NOEXCEPT_ override
  { return "bad_any_cast: failed any_cast conversion"; }
};

template <typename T> inline T* any_cast(any* v_)
{
  return v_ != nullptr && v_->m_value->type() == typeid(T)
    ? &static_cast<any::value<typename std::remove_cv<T>::type>*>(v_->m_value.get())->m_v
    : 0;
}

template <typename T> inline const T* any_cast(const any* v_)
{
  return any_cast<T>(const_cast<any*>(v_));
}

template <typename T> inline T any_cast(any & v_)
{
    using nonref = typename std::remove_reference<T>::type;


    nonref * result = any_cast<nonref>(&v_);
    if (!result)
        throw bad_any_cast();

    // Attempt to avoid construction of a temporary object in cases when
    // `T` is not a reference. Example:
    // `static_cast<std::string>(*result);`
    // which is equal to `std::string(*result);`
    using ref_type = typename std::conditional<
        std::is_reference<T>::value
      , T
      , typename std::add_lvalue_reference<T>::type
    >::type ;

    return static_cast<ref_type>(*result);
}

template <typename T> inline T any_cast(const any& v_
  , typename std::enable_if<!std::is_rvalue_reference<T>::value
// Visual Studio has broken std::is_copy_constructible trait
#if _MSC_VER == 1800
    && boost::is_copy_constructible<typename std::decay<T>::type>::value, void>::type * = 0)
#else
    && std::is_copy_constructible<typename std::decay<T>::type>::value, void>::type * = 0)
#endif
{
  using nonref = typename std::remove_reference<T>::type;
//  return any_cast<const nonref&>(const_cast<any&>(v_));
  return  any_cast<const nonref&>(const_cast<any&>(v_));
}

template <typename T> inline T any_cast(const any& v_
  , typename std::enable_if<std::is_rvalue_reference<T>::value
// Visual Studio has broken std::is_copy_constructible trait
#if _MSC_VER == 1800
    || !boost::is_copy_constructible<typename std::decay<T>::type>::value, void>::type * = 0)
#else
    || !std::is_copy_constructible<typename std::decay<T>::type>::value, void>::type * = 0)
#endif
{
  using nonref = typename std::remove_reference<T>::type;
//  return any_cast<const nonref&>(const_cast<any&>(v_));
  return std::move(*any_cast<nonref>(&const_cast<any&>(v_)));
}

template <typename T> inline T any_cast(any&& v_)
{
  return any_cast<T>(v_);
}


enum class work_type {
  task_work, event_work, cleanup_work
};

class work
{
 public:
  virtual ~work() { /* noop */ }
  virtual work_type type() const = 0;
};

class event_context : public work
{
 public:
  work_type type() const override
  {
    return work_type::event_work;
  }
  virtual void entry_point() = 0;
  virtual void* environment() = 0;
};

class cleanup_context : public event_context
{
public:
  work_type type() const override
  {
    return work_type::cleanup_work;
  }
};

// ---- execution context interface
class context
{
public:
  using result_reporter    = std::function<void(const any&)>;
  using exception_reporter = std::function<void(const std::exception_ptr&)>;

public:
  virtual ~context() { /* noop */ }

  // returns pointer to runner that is supposed to execute this context
  virtual std::weak_ptr<async::runner> get_runner() const = 0;
  // entry point to this context; to enter when context starts execute
  virtual void entry_point(const std::shared_ptr<async::runner>&, context*) = 0;
  // name of the context (context type name) for debugging purposes
  virtual const char* name() const = 0;
  // returns true if entry point will execute, false otherwise
  virtual bool will_execute() const = 0;
  // sets the input
  virtual void set_input(const any&) = 0;
  virtual void set_res_reporter(const result_reporter& arg_) = 0;
  virtual void set_exc_reporter(const exception_reporter& arg_) = 0;
};

// ---- execution context stack interface
// ---- each run() call creates a new execution context stack which is then
// ---- (re)submitted to task queues as long as there are unfinished contexts
class context_stack : public  work
{
 public:
  work_type type() const override
  {
    return work_type::task_work;
  }
  virtual ~context_stack() { /* noop */ }
  // pushes new context to the top of the stack
  virtual void push(context*) = 0;
  // returns top of the stack
  virtual context* top() const = 0;
  // return the top of the stack and removes it from the stack
  virtual context* pop() = 0;
  // returns true if stack is empty
  virtual bool empty() const = 0;
};




} } } }// namespace

#endif
