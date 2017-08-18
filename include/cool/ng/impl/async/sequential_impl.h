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

#if !defined(cool_ng_f36abcb0_33a1_84a1_b25a_943f5951523a)
#define      cool_ng_f36abcb0_33a1_84a1_b25a_943f5951523a

namespace cool { namespace ng { namespace async { namespace detail {

// ---- -----------------------------------------------------------------------
// ----
// ---- Static task information
// ----
// ---- -----------------------------------------------------------------------

template <typename InputT, typename ResultT>
class taskinfo<tag::sequential, default_runner_type, InputT, ResultT, TaskT...> : public detail::task
{
 public:
  using tag           = tag::sequential;
  using this_type     = taskinfo;
  using runner_type   = default_runner_type;
  using result_type   = ResultT;
  using input_type    = InputT;

  using subtasks_vector_type = std::array<std::shared_ptr<detail::task>, sizeof...(TaskT)>;

  using function_type = typename traits::run_signature<runner_type, input_type, result_type>::type;
  using context_type  = task_context<tag, runner_type, input_type, result_type>;

 public:
  explicit inline taskinfo(const std::shared_ptr<TaskT>&... tasks_)
      : m_subtasks( { tasks_ ... } )
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
    auto aux = create_context(stack, self_);
    kickstart(stack);
  }

  inline context* create_context(
      context_stack* stack_
    , const std::shared_ptr<task>& self_
    , const boost::any& input_) const override
  {
    auto aux = context_type::create(stack_, std::dynamic_pointer_cast<this_type>(self_), input_);
    return aux;
  }

  inline std::weak_ptr<runner> get_runner() const override
  {
    return m_subtasks[0]->get_runner();
  }

  inline std::size_t get_subtask_count() const override
  {
    return m_subtasks.size();
  }

  inline std::shared_ptr<task> get_subtask(std::size_t index) const override
  {
    return m_subtasks[index];
  }

 private:
  subtasks_vector_type m_subtasks;
};

// ---- -----------------------------------------------------------------------
// ----
// ---- Runtime task context
// ----
// ---- -----------------------------------------------------------------------
template <typename RunnerT, typename InputT, typename ResultT>
class task_context<tag::sequential, RunnerT, InputT, ResultT>
  : public task_context_base
{
 public:
  using this_type  = task_context;
  using base       = task_context_base;

 private:
  inline task_context(context_stack* st_, const std::shared_ptr<task>& t_)
    : base(st_, t_), m_next_task(0), m_num_tasks(t_->get_subtask_count())
  { /* noop */ }

 public:
  inline static this_type* create(
      context_stack* stack_
    , const std::shared_ptr<task>& task_
    , const boost::any& input_)
  {
    auto aux = new this_type(stack_, task_);
    stack_->push(aux);

    aux->set_input(input_);
    aux->prepare_next_task();

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
    return m_next_task < m_num_tasks;
  }

  void result_report(const boost::any& res_)
  {
    if (m_next_task == m_num_tasks)
    {
      m_stack->pop();
      if (m_res_reporter)
        m_res_reporter(res_);
      delete this;
    }
    else
    {
      m_input = res_;
      prepare_next_task();
    }
  }

  void exception_report(const std::exception_ptr& e)
  {
    m_stack->pop();
    if (m_exc_reporter)
      m_exc_reporter(e);
    delete this;
  }

  bool prepare_next_task()
  {
    if (m_next_task < m_num_tasks)
    {
      auto t_ = m_task->get_subtask(m_next_task);
      auto ctx = t_->create_context(m_stack, t_, m_input);
      ctx->set_res_reporter(std::bind(&this_type::result_report, this, std::placeholders::_1));
      ctx->set_exc_reporter(std::bind(&this_type::exception_report, this, std::placeholders::_1));
      m_next_task++;
      return true;
    }
    return false;
  }

 private:
  std::size_t       m_next_task;
  const std::size_t m_num_tasks;
};

} } } }

#endif
