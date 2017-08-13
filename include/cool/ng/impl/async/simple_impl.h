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

#if !defined(cool_ng_f36abcb0_33a1_42a1_b25a_943f5951523a)
#define      cool_ng_f36abcb0_33a1_42a1_b25a_943f5951523a

namespace cool { namespace ng { namespace async { namespace detail {

// ---- Static task information for tag::simple tasks

template <typename RunnerT, typename InputT, typename ResultT>
class taskinfo<tag::simple, RunnerT, InputT, ResultT> : public detail::task
{
 public:
  using type          = tag::simple;
  using this_type     = taskinfo;
  using runner_type   = RunnerT;
  using result_type   = ResultT;
  using input_type    = InputT;
  using function_type = typename traits::run_signature<runner_type, input_type, result_type>::type;
  using context_type  = task_context<type, runner_type, input_type, result_type>;

 public:
  explicit inline taskinfo(const std::weak_ptr<runner_type>& r_, const function_type& f_)
      : m_runner(r_), m_user_func(f_)
  { /* noop */ }

  template <typename T = InputT>
  inline void run(
      const std::shared_ptr<this_type>& self_
    , const typename std::enable_if<!std::is_same<T, void>::value, T>::type& i_)
  {
    auto aux = create_context(nullptr, self_);
    aux->set_input(get_value_container(i_));
    kickstart(aux);
  }

  inline context_type* create_context(context_stack* stack_, const std::shared_ptr<this_type>& self_)
  {
    auto aux = context_type::create(stack_, self_);
    return aux;
  }

  inline std::weak_ptr<runner> get_runner() const override
  {
    return m_runner;
  }

  inline function_type& user_callable()
  {
    return m_user_func;
  }

 private:
  std::weak_ptr<runner_type> m_runner;
  function_type              m_user_func;    // user Callable
};


// ---- Runtime task context for tag::simple tasks
template <typename RunnerT, typename InputT, typename ResultT>
class task_context<tag::simple, RunnerT, InputT, ResultT>
  : public task_context_base<ResultT>
  , public context_stack // context stack interface
{
 public:
  using task_type  = taskinfo<tag::simple, RunnerT, InputT, ResultT>;
  using this_type  = task_context;
  using base       = task_context_base<ResultT>;

 public:
  inline static this_type* create(
      context_stack* stack_
    , const std::shared_ptr<task_type>& task_)
  {
    auto aux = new this_type(stack_, task_);
    if (stack_ != nullptr)
      stack_->push(aux);
    return aux;
  }

  void set_input(const any_value<InputT>& input_)
  {
    m_input = input_;
  }

  // context interface
  inline std::weak_ptr<async::runner> get_runner() const override
  {
    return m_task->get_runner();
  }
  const char* name() const override
  {
    return "context::simple";
  }
  bool will_execute() const override
  {
    return true;
  }
  void entry_point(const std::shared_ptr<async::runner>& r_, context* ctx_) override;

  // context_stack interface
  void push(context*) override
  { /* noop */ }
  context* top() const override
  {
    return const_cast<this_type*>(this);
  }
  context* pop() override
  {
    return const_cast<this_type*>(this);
  }
  bool empty() const override
  {
    return true;
  }

 private:
  inline task_context(
      context_stack* st_
    , const std::shared_ptr<task_type>& t_
    , const typename base::result_reporter& r_rep_ = typename base::result_reporter()
    , const typename base::exception_reporter& e_rep_ = typename base::exception_reporter())
        : base(st_, r_rep_, e_rep_), m_task(t_)
  {
//    REP("++++++ context::simple::context void");
    /* noop */
  }

private:
  any_value<InputT>          m_input;  // Input to pass to user Callable
  std::shared_ptr<task_type> m_task;   // Reference to static task data
};

// --- entry point implementation
template <typename RunnerT, typename InputT, typename ResultT>
void task_context<tag::simple, RunnerT, InputT, ResultT>::entry_point(
      const std::shared_ptr<async::runner>& r_
    , context* ctx_)
{
//  REP("------ context::simple::entry_point");

  // remove self from the context stack but do not commit suicide just yet
  // NOTE: Removal must be done before reporting result since reporter may
  //       need to push new task on the top!
  if (base::m_stack != nullptr)
    base::m_stack->pop();

  try
  {
    auto r = std::dynamic_pointer_cast<RunnerT>(r_);
    if (!r)
      throw exception::bad_runner_cast();

    report<InputT, ResultT>::run_report(r, m_task, m_input, base::m_res_reporter);
  }
  catch (...)
  {
    if (base::m_exc_reporter)
      base::m_exc_reporter(std::current_exception());
  }

  // NOTE: null stack indicates standalone simple task which will get deleted as
  //       context stack in runner::task_executor. Do not delete it here!
  if (base::m_stack != nullptr)
    delete this;  // now suicide is in order
}


#if 0
template <typename RunnerT, typename InputT, typename ResultT>
class task<tag::simple, RunnerT, InputT, ResultT> : public detail::task
{
 public:
  using tag_type     = tag::simple;
  using this_type    = task;
  using runner_type  = RunnerT;
  using result_type  = ResultT;
  using input_type   = InputT;
//  using runtime_type = runtime::task<tag_type, runner_type, input_type, result_type>;
  using unbound_type = typename traits::unbound_type<runner_type, input_type, result_type>::type;

 public:
  explicit inline task(const std::weak_ptr<runner_type>& r_, const unbound_type& f_)
  : m_runner(r_), m_unbound(f_)
  { /* noop */ }

  // NOTE:
  // Run methods for simple tasks cheat a little - since a simple task by definition
  // has no subtasks they will allocate an execution context which is also a dummy
  // context stack. As its empty() method always return true it will get deleted
  // by task executor immediatelly after the task is done. This optimisation will
  // boost the run() speed for simple tasks to almost double the speed we can get
  // if stack and context were allocated separatelly.
  template <typename T = InputT>
  inline void run(
                  const std::shared_ptr<this_type>& self_
                  , const typename std::enable_if<!std::is_same<T, void>::value, T>::type& i_)
  {
    auto aux = create_runtime(nullptr, self_);
    aux->set_input(helpers::get_container(i_));
    ::cool::async::entrails::kick(aux);
  }

  inline void run(const std::shared_ptr<this_type>& self_)
  {
    ::cool::async::entrails::kick(create_runtime(nullptr, self_));
  }

  inline std::weak_ptr<runner> get_runner() const override { return m_runner; }
  inline unbound_type& user_callable() { return m_unbound; }
  
 private:
  std::weak_ptr<runner_type> m_runner;
  unbound_type               m_unbound;    // user Callable
};

#endif

} } } }

#endif
