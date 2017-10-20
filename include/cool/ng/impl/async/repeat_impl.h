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
// ---- Runtime task context
// ----
// ---- -----------------------------------------------------------------------

namespace {
template <typename R> class reporter
{
 public:
  static void report(const context::result_reporter& r_, const boost::any& res_)
  {
    if (res_.empty())
    {
      boost::any aux = R();
      r_(aux);
    }
    else
      r_(res_);
  }
};

template <> class reporter<void>
{
 public:
  static void report(const context::result_reporter& r_, const boost::any& res_)
  {
    r_(res_);
  }
};

}

template <typename ResultT>
class task_context<tag::repeat, default_runner_type, std::size_t, ResultT>
  : public task_context_base
{
 public:
  using this_type  = task_context;
  using base       = task_context_base;

 private:
  inline task_context(
      context_stack* st_
    , const std::shared_ptr<task>& task_
    , std::size_t limit_) : base(st_, task_), m_limit(limit_), m_counter(0)
  { /* noop */ }

 public:
  inline static this_type* create(
      context_stack* stack_
    , const std::shared_ptr<task>& task_
    , const std::size_t& input_)
  {
    auto aux = new this_type(stack_, task_, input_);
    stack_->push(aux);

    aux->set_input(input_);
    if (aux->m_counter < aux->m_limit)
      aux->prepare_next_task(aux->m_task->get_subtask(0));

    return aux;
  }

  // context interface
  inline std::weak_ptr<async::runner> get_runner() const override
  {
    return m_task->get_runner();
  }
  const char* name() const override
  {
    return "context::repeat";
  }
  bool will_execute() const override
  {
    return true;
  }

  void result_report(const boost::any& res_)
  {
    ++m_counter;
    if (m_counter < m_limit)
    {
      prepare_next_task(m_task->get_subtask(0));
      return;
    }

    // done
    m_stack->pop();
    if (m_res_reporter)
      reporter<ResultT>::report(m_res_reporter, res_);
    delete this;
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
    auto ctx = t_->create_context(m_stack, t_, m_counter);
    ctx->set_res_reporter(std::bind(&this_type::result_report, this, std::placeholders::_1));
    ctx->set_exc_reporter(std::bind(&this_type::exception_report, this, std::placeholders::_1));
  }

private:
  const std::size_t m_limit;
  std::size_t       m_counter;
};


// ---- -----------------------------------------------------------------------
// ----
// ---- Static task information
// ----
// ---- -----------------------------------------------------------------------

template <typename ResultT>
class taskinfo<tag::repeat, default_runner_type, std::size_t, ResultT> : public detail::task
{
 public:
  using tag           = tag::repeat;
  using this_type     = taskinfo;
  using runner_type   = default_runner_type;
  using result_type   = ResultT;
  using input_type    = std::size_t;
  using context_type  = task_context<tag, runner_type, input_type, result_type>;

 public:
  template <typename TaskT>
  explicit inline taskinfo(const std::shared_ptr<TaskT>& task_)
      : m_task(task_)
  { /* noop */ }
  
  inline void run(
      const std::shared_ptr<this_type>& self_
    , const std::size_t i_)
  {
    auto stack = new default_task_stack();
    create_context(stack, self_, i_);
    kickstart(stack);
  }

  inline context* create_context(
      context_stack* stack_
    , const std::shared_ptr<task>& self_
    , const boost::any& input_) const override
  {
    auto aux = context_type::create(stack_, self_, boost::any_cast<std::size_t>(input_));
    return aux;
  }

  inline std::weak_ptr<runner> get_runner() const override
  {
    return m_task->get_runner();
  }

  inline std::size_t get_subtask_count() const override
  {
    return 1;
  }

  inline std::shared_ptr<task> get_subtask(std::size_t index) const override
  {
    return m_task;
  }

 private:
  std::shared_ptr<task> m_task;
};

