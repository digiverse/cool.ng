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

template <typename InputT, typename ResultT>
class taskinfo<tag::conditional, default_runner_type, InputT, ResultT> : public detail::task
{
 public:
  using tag           = tag::conditional;
  using this_type     = taskinfo;
  using runner_type   = default_runner_type;
  using result_type   = ResultT;
  using input_type    = InputT;
  using context_type  = task_context<tag, runner_type, input_type, result_type>;

 public:
  template <typename PredicateT, typename IfT, typename ElseT>
  explicit inline taskinfo(const std::shared_ptr<PredicateT>& p_, const std::shared_ptr<IfT>& if_, const std::shared_ptr<ElseT>& else_)
      : m_predicate(p_), m_if(if_), m_else(else_)
  { /* noop */ }

  template <typename T = InputT>
  inline void run(
      const std::shared_ptr<this_type>& self_
    , const typename std::enable_if<!std::is_same<T, void>::value, T>::type& i_)
  {
    boost::any input = i_;
    auto stack = new default_task_stack();
    create_context(stack, self_, input);
    kickstart(stack);
  }

  template <typename T = InputT>
  typename std::enable_if<std::is_same<T, void>::value, void>::type run(const std::shared_ptr<this_type>& self_)
  {
    auto stack = new default_task_stack();
    create_context(stack, self_, boost::any());
    kickstart(stack);
  }

  inline context* create_context(
      context_stack* stack_
    , const std::shared_ptr<task>& self_
    , const boost::any& input_) const override
  {
    auto aux = context_type::create(stack_, self_, m_predicate, m_if, m_else, input_);
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
    return nullptr;
  }

 private:
  std::shared_ptr<task> m_predicate;
  std::shared_ptr<task> m_if;
  std::shared_ptr<task> m_else;
};

// ---- -----------------------------------------------------------------------
// ----
// ---- Runtime task context
// ----
// ---- -----------------------------------------------------------------------
template <typename RunnerT, typename InputT, typename ResultT>
class task_context<tag::conditional, RunnerT, InputT, ResultT>
  : public task_context_base
{
 public:
  using this_type  = task_context;
  using base       = task_context_base;

 private:
  inline task_context(
      context_stack* st_
    , const std::shared_ptr<task>& task_
    , const std::shared_ptr<task>& if_
    , const std::shared_ptr<task>& else_)
        : base(st_, task_), m_predicate_result(true), m_if(if_), m_else(else_)
  { /* noop */ }

 public:
  inline static this_type* create(
      context_stack* stack_
    , const std::shared_ptr<task>& task_
    , const std::shared_ptr<task>& predicate_
    , const std::shared_ptr<task>& if_
    , const std::shared_ptr<task>& else_
    , const boost::any& input_)
  {
    auto aux = new this_type(stack_, task_, if_, else_);
    stack_->push(aux);

    aux->set_input(input_);
    aux->prepare_next_task(predicate_);

    return aux;
  }

  // context interface
  inline std::weak_ptr<async::runner> get_runner() const override
  {
    return m_task->get_runner();
  }
  const char* name() const override
  {
    return "context::sequential";
  }
  bool will_execute() const override
  {
    return true;
  }

  void result_report(const boost::any& res_)
  {
    if (!m_predicate_result)
    {
      // result of one of branches
      m_stack->pop();
      if (m_res_reporter)
        m_res_reporter(res_);
      delete this;
      return;
    }

    m_predicate_result = false; // next report will come from one of branches
    if (boost::any_cast<bool>(res_))
    {
      prepare_next_task(m_if);
    }
    else
    {
      if (m_else)
      {
        prepare_next_task(m_else);
      }
      else
      {
        // else part is missing
        m_stack->pop();
        if (m_res_reporter)
          m_res_reporter(boost::any());
        delete this;
        return;
      }
    }
  }

  void exception_report(const std::exception_ptr& e)
  {
    m_stack->pop();
    if (m_exc_reporter)
      m_exc_reporter(e);
    delete this;
  }

  void prepare_next_task(const std::shared_ptr<task>& t_)
  {
    auto ctx = t_->create_context(m_stack, t_, m_input);
    ctx->set_res_reporter(std::bind(&this_type::result_report, this, std::placeholders::_1));
    ctx->set_exc_reporter(std::bind(&this_type::exception_report, this, std::placeholders::_1));
  }

 private:
  bool m_predicate_result;
  std::shared_ptr<task> m_if;
  std::shared_ptr<task> m_else;
};

