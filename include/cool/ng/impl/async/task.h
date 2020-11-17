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

#if !defined(cool_ng_f36abcb0_dda1_42a1_b25a_943f5951523a)
#define      cool_ng_f36abcb0_dda1_42a1_b25a_943f5951523a

#include <iostream> // TODO: remove
#include <memory>
#include <functional>
#include <type_traits>
#include <vector>
#include <stack>
#include <typeinfo>
#include <stdexcept>

#include "cool/ng/async/runner.h"
#include "context.h"
#include "task_traits.h"

namespace cool { namespace ng { namespace async { namespace detail {

// ---- Task types
namespace tag
{

  struct simple      { }; // simple task
  struct sequential  { }; // compound task with sequential execution
  struct parallel    { }; // compound task with concurrent execution
  struct conditional { }; // compount task with conditional execution
  struct oneof       { }; // oneof compound task
  struct loop        { }; // compound task that iterates the subtask
  struct repeat      { }; // compound task that repeats the subtask n times
  struct intercept   { }; // compound task with exception catchers

} // namespace

using default_runner_type = cool::ng::async::runner;

// ---- ----
// ---- Helper class templates to help keep implementations generic without
// ---- needless specializations for void result types
// ---- ----

// ---- Value container that can hold value of any type, including void
template <typename T>
struct any_value
{
  inline any_value() { /* noop */ }
  inline any_value(const T& r_) : value(r_) { /* noop */ }
  T value;
};

template <>
struct any_value<void> { };

template <typename T>
inline any_value<T> get_value_container(const T& v_)
{
  return any_value<T>(v_);
}
inline any_value<void> get_value_container()
{
  return any_value<void>();
}


// --- Result reporter that can will call user Callable with or without
// --- input parameter, depending on the user Callable declaration and will
// --- report the return value to the provided reporter function
template <typename InputT, typename ResultT>
struct report
{
  // SFINAE overloads for calls to user Callable with and without parameter
  template <typename RunnerT, typename TaskPtrT, typename ReporterT, typename T = InputT>
  inline static void run_report(
      const RunnerT& r_
    , const TaskPtrT& p_
    , const any_value<typename std::enable_if<!std::is_same<T, void>::value, void>::type>& i_
    , const ReporterT& rep_)
  {
    ResultT aux = p_->user_callable()(r_, i_.value);
    if (rep_)
      rep_(aux);
  }
  template <typename RunnerT, typename TaskPtrT, typename ReporterT, typename T = InputT>
  inline static void run_report(
      const RunnerT& r_
    , const TaskPtrT& p_
    , const any_value<typename std::enable_if<std::is_same<T, void>::value, void>::type>& i_
    , const ReporterT& rep_)
  {
    ResultT aux = p_->user_callable()(r_);
    if (rep_)
      rep_(aux);
  }
};

// --- Specialization for the result reporter for user Callable that do not return
// --- value. In this case the call to the provided reporter function only serves
// --- as completion signal.
template <typename InputT>
struct report<InputT, void>
{
  // SFINAE overloads for calls to user Callable with and without parameter
  template <typename RunnerT, typename TaskPtrT, typename ReporterT, typename T = InputT>
  inline static void run_report(
      const RunnerT& r_
    , const TaskPtrT& p_
    , const any_value<typename std::enable_if<!std::is_same<T, void>::value, void>::type>& i_
    , const ReporterT& rep_)
  {
    p_->user_callable()(r_, i_.value);
    if (rep_)
      rep_();
  }
  template <typename RunnerT, typename TaskPtrT, typename ReporterT, typename T = InputT>
  inline static void run_report(
      const RunnerT& r_
    , const TaskPtrT& p_
    , const any_value<typename std::enable_if<std::is_same<T, void>::value, void>::type>& i_
    , const ReporterT& rep_)
  {
    p_->user_callable()(r_);
    if (rep_)
      rep_();
  }
};
// ---- task implementation interface
class task
{
 public:
  virtual ~task() { /* noop */ }
  // Return runner for this task - makes sense only for tag::simple tasks,
  // compound tasks should never have their entry point executed anyway
  virtual std::weak_ptr<runner> get_runner() const = 0;
  // Returns the subtask at index - makes sense only for compound tasks
  virtual std::shared_ptr<task> get_subtask(std::size_t) const
  {
    return std::shared_ptr<task>();
  }
  // Returns number of subtasks
  virtual std::size_t get_subtask_count() const
  {
    return 0;
  }
  virtual context* create_context(
        context_stack* stack_
      , const std::shared_ptr<task>& self_
      , const any& input_) const = 0;
};

// ==== =====
// ====
// ==== Static task information
// ====
// ==== ====

template <typename TagT, typename RunnerT, typename ResultT, typename... InputT>
class x_taskinfo
{ /* noop default template */ };

// ---- task static information
template <typename TagT, typename RunnerT, typename InputT, typename ResultT, typename... TaskT>
class taskinfo { };

// ---- task runtime information
template <typename TagT, typename RunnerT, typename InputT, typename ResultT, typename... TaskT>
class task_context : public context { };

// ---- Task execution kick-starter
dlldecl void kickstart(context_stack*);

// ---- Default implementation of task stack
class default_task_stack : public context_stack
{
public:
  ~default_task_stack()
  {
    while (!empty())
      delete pop();
  }
  void push(context* arg_) override  { m_stack.push(arg_); }
  context* pop() override            { auto aux = m_stack.top(); m_stack.pop(); return aux; }
  context* top() const override      { return m_stack.top(); }
  bool empty() const override        { return m_stack.empty(); }

private:
  std::stack<context*> m_stack;
};



namespace base {

template <typename InputT, typename ResultT>
class taskinfo : public detail::task
{
 public:
  using this_type     = taskinfo;
  using result_type   = ResultT;
  using input_type    = InputT;

  template <typename T = InputT>
  inline void run(
      const std::shared_ptr<this_type>& self_
    , const typename std::decay<typename std::enable_if<
          !std::is_same<T, void>::value && !std::is_rvalue_reference<T>::value
        , T>::type>::type& i_)
  {
    any input = i_;
    auto stack = new default_task_stack();
    create_context(stack, self_, input);
    kickstart(stack);
  }

  // rvalue reference argument
  template <typename T = InputT>
  inline void run(
      const std::shared_ptr<this_type>& self_
    , typename std::enable_if<
        !std::is_same<T, void>::value && std::is_rvalue_reference<T>::value
      , T>::type i_)
  {
    any input(std::move(i_));
    auto stack = new default_task_stack();
    create_context(stack, self_, input);
    kickstart(stack);
  }

  template <typename T = InputT>
  typename std::enable_if<std::is_same<T, void>::value, void>::type run(const std::shared_ptr<this_type>& self_)
  {
    auto stack = new default_task_stack();
    create_context(stack, self_, any());
    kickstart(stack);
  }
};

} // namespace


#define __COOL_INCLUDE_TASK_IMPL_FILES__

#include "simple_impl.h"
#if 0
#include "sequential_impl.h"
#include "intercept_impl.h"
#include "conditional_impl.h"
#include "repeat_impl.h"
#include "loop_impl.h"
#endif
#undef __COOL_INCLUDE_TASK_IMPL_FILES__


} } } }




#endif

