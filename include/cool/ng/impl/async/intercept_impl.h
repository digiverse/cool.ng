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

struct catcher
{
  virtual ~catcher() { /* noop */ }
  virtual bool try_catch(
      const std::exception_ptr& e_
    , context_stack* stack_
    , const context::result_reporter& res_
    , const context::exception_reporter& exc_) = 0;
  virtual std::shared_ptr<task> get_task() const = 0;
};

template <typename E> class catcher_impl : public catcher
{
 public:
  catcher_impl(const std::shared_ptr<task>& t_) : m_task(t_)
  { /* noop */ }
  bool try_catch (
      const std::exception_ptr& e_
    , context_stack* stack_
    , const context::result_reporter& res_
    , const context::exception_reporter& exc_) override
  {
    try
    {
      std::rethrow_exception(e_);
    }
    catch ( const E& ex)
    {
      any input = ex;
      auto ctx = m_task->create_context(stack_, m_task, input);
      ctx->set_res_reporter(res_);
      ctx->set_exc_reporter(exc_);
      return true;
    }
    catch ( ... )
    { }
    return false;
  }

  std::shared_ptr<task> get_task() const override
  {
    return m_task;
  }

 private:
  std::shared_ptr<task> m_task;
};

// std::exception_ptr type of parameter is a catch-all catcher
template <> class catcher_impl<std::exception_ptr> : public catcher
{
 public:
  catcher_impl(const std::shared_ptr<task>& t_) : m_task(t_)
  { /* noop */ }
  bool try_catch (
      const std::exception_ptr& e_
    , context_stack* stack_
    , const context::result_reporter& res_
    , const context::exception_reporter& exc_) override
  {
    any input = e_;
    auto ctx = m_task->create_context(stack_, m_task, input);
    ctx->set_res_reporter(res_);
    ctx->set_exc_reporter(exc_);
    return true;
  }
  std::shared_ptr<task> get_task() const override
  {
    return m_task;
  }

 private:
  std::shared_ptr<task> m_task;
};


template <typename InputT, typename ResultT>
class taskinfo<tag::intercept, default_runner_type, InputT, ResultT> :public base::taskinfo<InputT, ResultT>
{
 public:
  using tag           = tag::intercept;
  using this_type     = taskinfo;
  using runner_type   = default_runner_type;
  using result_type   = ResultT;
  using input_type    = InputT;

  using catch_vector_type = std::vector<std::shared_ptr<catcher>>;

  using context_type  = task_context<tag, runner_type, input_type, result_type>;

 public:
#if !defined(WINDOWS_TARGET) || (_MSC_VER > 1800)
  template <typename TryT, typename... CatchT>
  explicit inline taskinfo(
      const std::shared_ptr<TryT>& task_
    , const std::shared_ptr<CatchT>&... catchers_)
        : m_subtask(task_)
        , m_catchers( { std::make_shared<catcher_impl<typename CatchT::input_type>>(catchers_)... } )
  { /* noop */ }

#else
  // Visual Studio 2013 has hickups with the above arg pack ctor - it
  // requires separate ctors for each number of arguments. Hence a sequence of
  // up to 5 catchers is supported
  template <typename TryT
    , typename C1
  > explicit inline taskinfo(const std::shared_ptr<TryT>& task_
    , const std::shared_ptr<C1>& c1_
  ) : m_subtask(task_)
  {
    m_catchers.push_back(std::make_shared<catcher_impl<typename C1::input_type>>(c1_));
  }
  template <typename TryT
    , typename C1
    , typename C2
  > explicit inline taskinfo(const std::shared_ptr<TryT>& task_
    , const std::shared_ptr<C1>& c1_
    , const std::shared_ptr<C2>& c2_
  ) : m_subtask(task_)
  {
    m_catchers.push_back(std::make_shared<catcher_impl<typename C1::input_type>>(c1_));
    m_catchers.push_back(std::make_shared<catcher_impl<typename C2::input_type>>(c2_));
  }
  template <typename TryT
    , typename C1
    , typename C2
    , typename C3
  > explicit inline taskinfo(const std::shared_ptr<TryT>& task_
    , const std::shared_ptr<C1>& c1_
    , const std::shared_ptr<C2>& c2_
    , const std::shared_ptr<C3>& c3_
  ) : m_subtask(task_)
  {
    m_catchers.push_back(std::make_shared<catcher_impl<typename C1::input_type>>(c1_));
    m_catchers.push_back(std::make_shared<catcher_impl<typename C2::input_type>>(c2_));
    m_catchers.push_back(std::make_shared<catcher_impl<typename C3::input_type>>(c3_));
  }
  template <typename TryT
    , typename C1
    , typename C2
    , typename C3
    , typename C4
  > explicit inline taskinfo(const std::shared_ptr<TryT>& task_
    , const std::shared_ptr<C1>& c1_
    , const std::shared_ptr<C2>& c2_
    , const std::shared_ptr<C3>& c3_
    , const std::shared_ptr<C4>& c4_
  ) : m_subtask(task_)
  {
    m_catchers.push_back(std::make_shared<catcher_impl<typename C1::input_type>>(c1_));
    m_catchers.push_back(std::make_shared<catcher_impl<typename C2::input_type>>(c2_));
    m_catchers.push_back(std::make_shared<catcher_impl<typename C3::input_type>>(c3_));
    m_catchers.push_back(std::make_shared<catcher_impl<typename C4::input_type>>(c4_));
  }
  template <typename TryT
    , typename C1
    , typename C2
    , typename C3
    , typename C4
    , typename C5
  > explicit inline taskinfo(const std::shared_ptr<TryT>& task_
    , const std::shared_ptr<C1>& c1_
    , const std::shared_ptr<C2>& c2_
    , const std::shared_ptr<C3>& c3_
    , const std::shared_ptr<C4>& c4_
    , const std::shared_ptr<C5>& c5_
  ) : m_subtask(task_)
  {
    m_catchers.push_back(std::make_shared<catcher_impl<typename C1::input_type>>(c1_));
    m_catchers.push_back(std::make_shared<catcher_impl<typename C2::input_type>>(c2_));
    m_catchers.push_back(std::make_shared<catcher_impl<typename C3::input_type>>(c3_));
    m_catchers.push_back(std::make_shared<catcher_impl<typename C4::input_type>>(c4_));
    m_catchers.push_back(std::make_shared<catcher_impl<typename C5::input_type>>(c5_));
  }


#endif


  inline context* create_context(
      context_stack* stack_
    , const std::shared_ptr<task>& self_
    , const any& input_) const override
  {
    auto aux = context_type::create(stack_, self_, m_subtask, m_catchers, input_);
    return aux;
  }

  inline std::weak_ptr<runner> get_runner() const override
  {
    return m_subtask->get_runner();
  }

  inline std::size_t get_subtask_count() const override
  {
    return 1;
  }

  inline std::shared_ptr<task> get_subtask(std::size_t index) const override
  {
    return m_subtask;
  }

 private:
  std::shared_ptr<task> m_subtask;
  catch_vector_type     m_catchers;
};

// ---- -----------------------------------------------------------------------
// ----
// ---- Runtime task context
// ----
// ---- -----------------------------------------------------------------------
template <typename RunnerT, typename InputT, typename ResultT>
class task_context<tag::intercept, RunnerT, InputT, ResultT>
  : public task_context_base
{
 public:
  using this_type = task_context;
  using task_type = taskinfo<tag::intercept, RunnerT, InputT, ResultT>;
  using base      = task_context_base;

 private:
  inline task_context(
      context_stack* st_
    , const std::shared_ptr<task>& t_
    , const typename task_type::catch_vector_type& catchers_)
        : base(st_, t_), m_catchers(catchers_)
  { /* noop */ }

 public:
  inline static this_type* create(
      context_stack* stack_
    , const std::shared_ptr<task>& task_
    , const std::shared_ptr<task>& subtask_
    , const typename task_type::catch_vector_type& catchers_
    , const any& input_)
  {
    auto aux = new this_type(stack_, task_, catchers_);
    stack_->push(aux);

    aux->set_input(input_);
    auto sub_ctx = subtask_->create_context(stack_, subtask_, input_);
    sub_ctx->set_res_reporter(std::bind(&this_type::result_report, aux, std::placeholders::_1));
    sub_ctx->set_exc_reporter(std::bind(&this_type::exception_report, aux, std::placeholders::_1));

    return aux;
  }

  // context interface
  inline std::weak_ptr<async::runner> get_runner() const override
  {
    return m_task->get_runner();
  }
  const char* name() const override
  {
    return "context::intercept";
  }
  bool will_execute() const override
  {
    return true;
  }

  void result_report(const any& res_)
  {
    m_stack->pop();
    if (m_res_reporter)
      m_res_reporter(res_);
    delete this;
  }

  void final_exception_report(const std::exception_ptr& e_)
  {
    m_stack->pop();
    if (m_exc_reporter)
      m_exc_reporter(e_);
    delete this;
  }

  void exception_report(const std::exception_ptr& e_)
  {
    // first go through catch testers to see if any subtask would catch exception
    for (std::size_t i = 0; i < m_catchers.size(); ++i)
    {
      if (m_catchers[i]->try_catch(
          e_
        , m_stack
        , std::bind(&this_type::result_report, this, std::placeholders::_1)
        , std::bind(&this_type::final_exception_report, this, std::placeholders::_1)))
      {
        // exception was caught and it's processing pushed to execution stack
        return;
      }
    }

    // no catcher found, propagate exception upwards if possible
    final_exception_report(e_);
  }

 private:
  const typename task_type::catch_vector_type& m_catchers;
};

