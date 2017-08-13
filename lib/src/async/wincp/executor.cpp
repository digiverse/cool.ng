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
#include <mutex>

#include "cool/ng/async/runner.h"
#include "cool/ng/exception.h"
#include "executor.h"
namespace cool { namespace ng { namespace async { namespace impl {

/*constexpr*/ const int TASK = 1;


class poolmgr
{
 public:
  poolmgr()
    : m_pool(nullptr)
  {
    InitializeThreadpoolEnvironment(&m_environ);
    m_pool = CreateThreadpool(nullptr);
    if (m_pool == nullptr)
      throw exception::operation_failed("failed to create thread pool");

    // Associate the callback environment with our thread pool.
    SetThreadpoolCallbackPool(&m_environ, m_pool);
  }

  ~poolmgr()
  {
    if (m_pool != nullptr)
      CloseThreadpool(m_pool);

    DestroyThreadpoolEnvironment(&m_environ);
  }

  PTP_CALLBACK_ENVIRON get_environ()
  {
    return &m_environ;
  }

private:
  PTP_POOL            m_pool;
  TP_CALLBACK_ENVIRON m_environ;
};

std::unique_ptr<poolmgr> executor::m_pool;;
unsigned int             executor::m_refcnt(0);
critical_section         executor::m_cs;

executor::executor(RunPolicy policy_)
    : named("si.digiverse.ng.cool.runner")
    , m_work(NULL)
    , m_fifo(NULL)
    , m_work_in_progress(false)
    , m_active(true)
{
  check_create_thread_pool();

  try
  {
    m_fifo = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1000);
    if (m_fifo == NULL)
      throw exception::operation_failed("failed to create i/o completion port");

    try
    {
      // Create work with the callback environment.
      m_work = CreateThreadpoolWork(task_executor, this, m_pool->get_environ());
      if (m_work == NULL)
        throw exception::operation_failed("failed to create work");
    }
    catch (...)
    {
      CloseHandle(m_fifo);
      throw;
    }
  }
  catch (...) {
    check_delete_thread_pool();
    throw;
  }
}

executor::~executor()
{
  stop();

  if (m_fifo != NULL)
  {
    // delete all pending work in fifo queue (a.k.a. completion port)
    LPOVERLAPPED aux;
    DWORD        cmd;
    ULONG_PTR    key;

    while(GetQueuedCompletionStatus(m_fifo, &cmd, &key, &aux, 0))
    {
    // NOTE: it is assumed that the non-empty context_stack will delete all its
    // elements still left on the stack
      delete static_cast<cool::ng::async::detail::context_stack*>(static_cast<void*>(aux));
    }

    // now close the completion port
    CloseHandle(m_fifo);
  }

  if  (m_work != NULL)
    CloseThreadpoolWork(m_work);

  check_delete_thread_pool();
}

// use critical section to safely create and delete thread pool - these are
// expected to be pretty infrequent calls and critical section is okay for that
void executor::check_create_thread_pool()
{
  std::unique_lock<critical_section> l(m_cs);

  if (m_refcnt++ == 0)
  {
    m_pool.reset(new poolmgr());
  }
}

void executor::check_delete_thread_pool()
{
  std::unique_lock<critical_section> l(m_cs);

  if (--m_refcnt == 0)
  {
    m_pool.reset();
  }
}

void executor::start()
{
  bool expect = false;
  if (m_active.compare_exchange_strong(expect, true))
    SubmitThreadpoolWork(m_work);
}

void executor::stop()
{
  m_active = false;
}

// executor for task::run() that runs in the thread pool
VOID CALLBACK executor::task_executor(PTP_CALLBACK_INSTANCE instance_, PVOID pv_, PTP_WORK work_)
{
  static_cast<executor*>(pv_)->task_executor();
}

void executor::task_executor()
{
  LPOVERLAPPED aux;
  DWORD        cmd;
  ULONG_PTR    key;

  if (!GetQueuedCompletionStatus(m_fifo, &cmd, &key, &aux, 0))
  {
    m_work_in_progress = false;
    return;
  }

  auto stack = static_cast<cool::ng::async::detail::context_stack*>(static_cast<void*>(aux));
  auto context = stack->top();
  auto r = context->get_runner().lock();

  if (r)
  {
    // call into task
    try
    {
      context->entry_point(r, context);
    }
    catch(...)
    {
      /* noop */
    }

    if (stack->empty())
    {
      delete stack;
    }
    else
    {
      r->impl()->run(stack);
    }
  }
  else
  {
    delete stack;
  }

  start_work();
}

void executor::run(cool::ng::async::detail::context_stack* ctx_)
{
  bool do_start = false;

  PostQueuedCompletionStatus(m_fifo, TASK, NULL, reinterpret_cast<LPOVERLAPPED>(ctx_));
  do_start = m_work_in_progress.compare_exchange_strong(do_start, true);

  if (do_start)
    start_work();
}

void executor::start_work()
{
  if (m_active)
  {
    SubmitThreadpoolWork(m_work);
  }
}

} } } } // namespace
