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

// ---- -----------------------------------------------------------------------
// ----
// ---- Static task information
// ----
// ---- -----------------------------------------------------------------------

#if !defined(__COOL_INCLUDE_TASK_IMPL_FILES__)
#error "This header file cannot be directly included in the application code."
#endif



template <typename RunnerT, typename ResultT, typename... InputT>
class x_taskinfo<tag::simple, RunnerT, ResultT, InputT...> // : public base::taskinfo<InputT, ResultT>
{
 public:
  using function_type = std::function<ResultT(const std::shared_ptr<RunnerT>&, const typename std::decay<InputT>::type&...)>;
};

#if 0
template <typename RunnerT, typename InputT, typename ResultT>
class taskinfo<tag::simple, RunnerT, InputT, ResultT> : public base::taskinfo<InputT, ResultT>
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


  inline context* create_context(
      context_stack* stack_
    , const std::shared_ptr<task>& self_
    , const any& input_) const override
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
#endif

// ---- -----------------------------------------------------------------------
// ----
// ---- Runtime task context
// ----
// ---- -----------------------------------------------------------------------

namespace helpers {

template<typename ResultT> struct get_result_reporter
{
  using type = std::function<void(context*, const ResultT&)>;
};
template<> struct get_result_reporter<void>
{
  using type = std::function<void(context*)>;
};

} // namespace helpers

template <typename RunnerT, typename ResultT, typename... InputT>
class context_impl<tag::simple, RunnerT, ResultT, InputT...>
  : public task_context_base
  , public param_store<InputT...>
{
 public:
  using task_type  = x_taskinfo<tag::simple, RunnerT, ResultT, InputT...>;
  using this_type  = context_impl;
  using store_type = param_store<InputT...>;
  using result_reporter_type = typename helpers::get_result_reporter<ResultT>::type;

  inline static this_type* create(
      context_stack* stack_
    , const std::shared_ptr<task>& task_
    , const typename task_type::function_type& function_
    , const result_reporter_type& res_reporter_
    , const InputT&... input_)
  {
    auto aux = new this_type(stack_, task_, function_, res_reporter_, input_...);
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

  private:
   inline context_impl(
         context_stack* st_
       , const std::shared_ptr<task>& t_
       , const typename task_type::function_type& f_
       , const result_reporter_type& res_reporter_
       , const InputT&... input_)
     : task_context_base(st_, t_)
     , param_store<InputT...>(input_...)
     , m_user_func(f_)
     , m_result_reporter(res_reporter_)
   { /* noop */ }

  private:
   const typename task_type::function_type&  m_user_func;
   const result_reporter_type& m_result_reporter;
};


template <typename ResultT, typename TupleT>
struct callable_invoker
{
  template<typename EntryPointT, typename RunnerT, typename ReporterT>
  static void invoke(
      const EntryPointT& ep_
    , context* rep_ctx_
    , const std::shared_ptr<RunnerT>& r_
    , const ReporterT& rep_
    , const TupleT& input_)
  {
    if (rep_)
      rep_(rep_ctx_, helpers::invoke_callable<ResultT>(r_, ep_, input_));
    else
      helpers::invoke_callable<ResultT>(r_, ep_, input_);
  }
};

template <typename TupleT>
struct callable_invoker<void, TupleT>
{
  template<typename EntryPointT, typename RunnerT, typename ReporterT>
  static void invoke(
      const EntryPointT& ep_
    , context* rep_ctx_
    , const std::shared_ptr<RunnerT>& r_
    , const ReporterT& rep_
    , const TupleT& input_)
  {
    helpers::invoke_callable<void>(r_, ep_, input_);
    if (rep_)
      rep_(rep_ctx_);
  }
};
// ----
// ---- Entry point for simple task
// ----
template <typename RunnerT, typename ResultT, typename... InputT>
void context_impl<tag::simple, RunnerT, ResultT, InputT...>::entry_point(
    const std::shared_ptr<async::runner>& r_, context* ctx_)
{
  try
  {
    auto runner = std::dynamic_pointer_cast<RunnerT>(r_);
    if (!runner)
      throw exception::bad_runner_cast();

    callable_invoker<ResultT, decltype(store_type::m_arguments)>::invoke(m_user_func, nullptr, runner, m_result_reporter, store_type::m_arguments);
  }
  catch (...)
  {

  }
}







#if 0

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
    , const any& i_)
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
              const any& i_,
              const ReporterT& rep_)
  {
    any res = ep_(r_, any_cast<InputT>(i_));
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
              const any& i_,
              const ReporterT& rep_)
  {
    any res = ep_(r_);
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
                     const any& i_,
                     const ReporterT& rep_)
  {
    ep_(r_, any_cast<InputT>(i_));
    if (rep_)
      rep_(any());
  }
};
template <>
struct invoker<void, void>
{
  template<typename EntryPointT, typename RunnerT, typename ReporterT>
  static void invoke(const EntryPointT& ep_,
                     const std::shared_ptr<RunnerT>& r_,
                     const any& i_,
                     const ReporterT& rep_)
  {
    ep_(r_);
    if (rep_)
      rep_(any());
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
  //       context stack in runner::task_executor. Hence do not delete it here!
  if (m_stack != nullptr)
    delete this;  // now suicide is in order
}

#endif
