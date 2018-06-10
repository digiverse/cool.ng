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

#if !defined(__COOL_INCLUDE_TASK_IMPL_FILES__)
#error "This header file cannot be directly included in the application code."
#endif

// ---- -----------------------------------------------------------------------
// ----
// ---- Static task information
// ----
// ---- -----------------------------------------------------------------------

template <typename InputT, typename ResultT>
class taskinfo<tag::loop, default_runner_type, InputT, ResultT> : public base::taskinfo<InputT, ResultT>
{
 public:
  using tag           = tag::loop;
  using this_type     = taskinfo;
  using runner_type   = default_runner_type;
  using result_type   = ResultT;
  using input_type    = InputT;
  using context_type  = task_context<tag, runner_type, input_type, result_type>;

 public:
  template <typename PredicateT, typename BodyT>
  explicit inline taskinfo(const std::shared_ptr<PredicateT>& p_, const std::shared_ptr<BodyT>& body_)
      : m_predicate(p_), m_body(body_)
  { /* noop */ }
  
  template <typename PredicateT>
  explicit inline taskinfo(const std::shared_ptr<PredicateT>& p_)
      : m_predicate(p_)
  { /* noop */ }

  inline context* create_context(
      context_stack* stack_
    , const std::shared_ptr<task>& self_
    , const any& input_) const override
  {
    auto aux = context_type::create(stack_, self_, input_);
    return aux;
  }

  inline std::weak_ptr<runner> get_runner() const override
  {
    return m_predicate->get_runner();
  }

  inline std::size_t get_subtask_count() const override
  {
    return 0;
  }

  inline std::shared_ptr<task> get_subtask(std::size_t index) const override
  {
    switch (index)
    {
      case 0:
        return m_predicate;
      case 1:
        return m_body;
      default:
        break;
    }
    return nullptr;
  }

 private:
  std::shared_ptr<task> m_predicate;
  std::shared_ptr<task> m_body;
};

// ---- -----------------------------------------------------------------------
// ----
// ---- Runtime task context
// ----
// ---- -----------------------------------------------------------------------
template <typename InputT, typename ResultT>
class task_context<tag::loop, default_runner_type, InputT, ResultT>
  : public task_context_base
{
 public:
  using this_type  = task_context;
  using base       = task_context_base;

 private:
  inline task_context(
      context_stack* st_
    , const std::shared_ptr<task>& self_)
        : base(st_, self_), m_predicate_result(true)
  { /* noop */ }

 public:
  inline static this_type* create(
      context_stack* stack_
    , const std::shared_ptr<task>& task_
    , const any& input_)
  {
    auto aux = new this_type(stack_, task_);
    stack_->push(aux);

    aux->set_input(input_);
    aux->prepare_predicate_task();

    return aux;
  }

  // context interface
  inline std::weak_ptr<async::runner> get_runner() const override
  {
    return m_task->get_runner();
  }
  const char* name() const override
  {
    return "context::loop";
  }
  bool will_execute() const override
  {
    return true;
  }

  void predicate_result_report(const any& res_)
  {
    m_predicate_result = any_cast<bool>(res_);
    if (!m_predicate_result)   // predicate evaluated to false, terminate loop
    {
      m_stack->pop();
      if (m_res_reporter)
        m_res_reporter(m_input);
      delete this;
      return;
    }

    if (!prepare_body_task())  // if no body rerun predicate task
      prepare_predicate_task();
  }

  void body_result_report(const any& r_)
  {
    set_input(r_);
    prepare_predicate_task();
  }

  void exception_report(const std::exception_ptr& e)
  {
    m_stack->pop();
    if (m_exc_reporter)
      m_exc_reporter(e);
    delete this;
  }

  bool prepare_body_task()
  {
    auto t_ = m_task->get_subtask(1);
    if (!t_)
      return false;

    auto ctx = t_->create_context(m_stack, t_, m_input);
    ctx->set_res_reporter(std::bind(&this_type::body_result_report, this, std::placeholders::_1));
    ctx->set_exc_reporter(std::bind(&this_type::exception_report, this, std::placeholders::_1));
    return true;
  }

  void prepare_predicate_task()
  {
    auto t_ = m_task->get_subtask(0);
    auto ctx = t_->create_context(m_stack, t_, m_input);
    ctx->set_res_reporter(std::bind(&this_type::predicate_result_report, this, std::placeholders::_1));
    ctx->set_exc_reporter(std::bind(&this_type::exception_report, this, std::placeholders::_1));
  }

 private:
  std::shared_ptr<task> m_predicate;
  bool                  m_predicate_result;
};

