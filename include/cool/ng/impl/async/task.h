/* Copyright (c) 2017 Digiverse d.o.o.
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

#include <type_traits>

#include "cool/ng/async/runner.h"
#include "context.h"
#include "task_traits.h"

namespace cool { namespace ng { namespace async { namespace detail {

// ---- Task types
namespace tag
{

  struct simple      { }; // simple task
  struct serial      { }; // compound task with sequential execution
  struct parallel    { }; // compound task with concurrent execution
  struct conditional { }; // compount task with conditional execution
  struct oneof       { }; // oneof compound task
  struct loop        { }; // compound task that iterates the subtask
  struct repeat      { }; // compound task that repeats the subtask n times
  struct intercept   { }; // compound task with exception catchers

} // namespace

// ---- exceptions used internally
// exceptions generated internaly by async runtime
namespace exception {

class internal : public cool::ng::exception::runtime_exception
{
 public:
  internal(const std::string& msg_) : runtime_exception(msg_)
  { /* noop */ }
};

class bad_runner_cast : public internal
{
 public:
  bad_runner_cast()
  : internal("dynamic_pointer_cast to RunnerT type unexpectedly failed")
  { /* noop */ }
};

class runner_not_available : public internal
{
 public:
  runner_not_available()
  : internal("the destination runner not available")
  { /* noop */ }
};

class no_context : public internal
{
 public:
  no_context()
  : internal("the task context is not available")
  { /* noop */ }
};

class nothing_to_run : public internal
{
 public:
  nothing_to_run()
  : internal("task will not run owing to predicate resolution")
  { /* noop */ }
};
  
} // namspace exception

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
    , const any_value<typename std::enable_if<!std::is_same<T, void>::value, T>::type>& i_
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
    , const any_value<typename std::enable_if<!std::is_same<T, void>::value, T>::type>& i_
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
  virtual std::weak_ptr<runner> get_runner() const = 0;
};

// ---- task static information
template <typename TagT, typename RunnerT, typename InputT, typename ResultT, typename... TaskT>
class taskinfo { };

// ---- task runtime information
template <typename TagT, typename RunnerT, typename InputT, typename ResultT, typename... TaskT>
class task_context : public context { };

template <typename ResultT> class task_context_base : public context
{
 public:
  using result_reporter = typename traits::result_reporter<ResultT>::type;
  using exception_reporter = std::function<void(const std::exception_ptr&)>;

 public:
  inline task_context_base(context_stack* stack_, const result_reporter& r_rep_, const exception_reporter& e_rep_)
    : m_stack(stack_), m_res_reporter(r_rep_), m_exc_reporter(e_rep_)
  { /* noop */ }
  virtual inline ~task_context_base()
  { /* noop */ }

  void set_res_reporter(const result_reporter& arg_)    { m_res_reporter = arg_; }
  void set_exc_reporter(const exception_reporter& arg_) { m_exc_reporter = arg_; }

 protected:
  context_stack*     m_stack;
  result_reporter    m_res_reporter; // result reporter if set
  exception_reporter m_exc_reporter; // exception reporter if set
};

// ---- Task execution kick-starter
void kickstart(context_stack*);

} } } }

#include "simple_impl.h"



#endif

