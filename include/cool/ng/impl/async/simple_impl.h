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

// ---- -----------------------------------------------------------------------
// ----
// ---- Static task information
// ----
// ---- -----------------------------------------------------------------------

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
    auto aux = create_context(nullptr, self_, boost::any(i_));
    kickstart(dynamic_cast<context_stack*>(aux));
  }

  template <typename T = InputT>
  typename std::enable_if<std::is_same<T, void>::value, void>::type run(const std::shared_ptr<this_type>& self_)
  {
    auto aux = create_context(nullptr, self_, boost::any());
    kickstart(dynamic_cast<context_stack*>(aux));
  }

  inline context* create_context(
      context_stack* stack_
    , const std::shared_ptr<task>& self_
    , const boost::any& input_) const override
  {
    return context_type::create(stack_, self_, m_user_func, input_);
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


// ---- -----------------------------------------------------------------------
// ----
// ---- Runtime task context
// ----
// ---- -----------------------------------------------------------------------

template <typename RunnerT, typename InputT, typename ResultT>
class task_context<tag::simple, RunnerT, InputT, ResultT>
  : public task_context_base
  , public context_stack // context stack interface
{
 public:
  using task_type  = taskinfo<tag::simple, RunnerT, InputT, ResultT>;
  using this_type  = task_context;
  using base       = task_context_base;

 private:
  inline task_context(context_stack* st_, const std::shared_ptr<task>& t_, const typename task_type::function_type& f_)
      : base(st_, t_), m_user_func(f_)
  {
    //    REP("++++++ context::simple::context void");
    /* noop */
  }

 public:
  inline static this_type* create(
      context_stack* stack_
    , const std::shared_ptr<task>& task_
    , const typename task_type::function_type& f_
    , const boost::any& i_)
  {
    auto aux = new this_type(stack_, task_, f_);
    aux->set_input(i_);
    if (stack_ != nullptr)
      stack_->push(aux);
    return aux;
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
  const typename task_type::function_type&  m_user_func;
};


template <typename InputT, typename ResultT>
struct invoker
{
  template<typename EntryPointT, typename RunnerT, typename ReporterT>
  static void invoke(const EntryPointT& ep_,
              const std::shared_ptr<RunnerT>& r_,
              const boost::any& i_,
              const ReporterT& rep_)
  {
    boost::any res = ep_(r_, boost::any_cast<InputT>(i_));
    if (rep_)
      rep_(res);
  }
};
template <typename ResultT>
struct invoker<void, ResultT>
{
  template<typename EntryPointT, typename RunnerT, typename ReporterT>
  static void invoke(const EntryPointT& ep_,
              const std::shared_ptr<RunnerT>& r_,
              const boost::any& i_,
              const ReporterT& rep_)
  {
    boost::any res = ep_(r_);
    if (rep_)
      rep_(res);
  }
};
template <typename InputT>
struct invoker<InputT, void>
{
  template<typename EntryPointT, typename RunnerT, typename ReporterT>
  static void invoke(const EntryPointT& ep_,
                     const std::shared_ptr<RunnerT>& r_,
                     const boost::any& i_,
                     const ReporterT& rep_)
  {
    ep_(r_, boost::any_cast<InputT>(i_));
    if (rep_)
      rep_(boost::any());
  }
};
template <>
struct invoker<void, void>
{
  template<typename EntryPointT, typename RunnerT, typename ReporterT>
  static void invoke(const EntryPointT& ep_,
                     const std::shared_ptr<RunnerT>& r_,
                     const boost::any& i_,
                     const ReporterT& rep_)
  {
    ep_(r_);
    if (rep_)
      rep_(boost::any());
  }
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

    invoker<InputT, ResultT>::invoke(m_user_func, r, m_input, m_res_reporter);
  }
  catch (...)
  {
    if (m_exc_reporter)
      m_exc_reporter(std::current_exception());
  }

  // NOTE: null stack indicates standalone simple task which will get deleted as
  //       context stack in runner::task_executor. Do not delete it here!
  if (m_stack != nullptr)
    delete this;  // now suicide is in order
}


