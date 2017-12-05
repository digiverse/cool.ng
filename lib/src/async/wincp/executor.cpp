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
#include "executor.h"

#include <mutex>
#include "cool/ng/async/runner.h"
#include "cool/ng/exception.h"

namespace cool { namespace ng { namespace async { namespace impl {

CONSTEXPR_ const int TASK = 1;

poolmgr::weak_ptr poolmgr::m_self;
critical_section  poolmgr::m_cs;

poolmgr::poolmgr() : m_pool(nullptr)
{
  InitializeThreadpoolEnvironment(&m_environ);
  m_pool = CreateThreadpool(nullptr);
  if (m_pool == nullptr)
    throw exception::threadpool_failure();

  // Associate the callback environment with our thread pool.
  SetThreadpoolCallbackPool(&m_environ, m_pool);
}

poolmgr::~poolmgr()
{
  if (m_pool != nullptr)
    CloseThreadpool(m_pool);

  DestroyThreadpoolEnvironment(&m_environ);
}

// Use critical section to safely create thread pool - these are expected to be
// pretty infrequent calls and critical section is okay for that
// The deletion of the thread pool is handled via shared pointers owned by thread
// pool users and is thus inherently thread safe
poolmgr::ptr poolmgr::get_poolmgr()
{
  std::unique_lock<critical_section> l(m_cs);
  auto ret = m_self.lock();

  if (!ret)
  {
    ret = std::make_shared<poolmgr>();
    m_self = ret;
  }
  return ret;
}


executor::executor(RunPolicy policy_)
    : named("si.digiverse.ng.cool.runner")
    , m_work(nullptr)
    , m_fifo(nullptr)
    , m_pool(poolmgr::get_poolmgr())
    , m_work_in_progress(false)
    , m_active(true)
{
  m_fifo = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1000);
  if (m_fifo == NULL)
    throw exception::cp_failure();

  try
  {
    // Create work with the callback environment.
    m_work = CreateThreadpoolWork(task_executor, this, m_pool->get_environ());
    if (m_work == NULL)
      throw exception::threadpool_failure();
  }
  catch (...)
  {
    CloseHandle(m_fifo);
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

  auto work = static_cast<cool::ng::async::detail::work*>(static_cast<void*>(aux));
  switch (work->type())
  {
    case cool::ng::async::detail::work_type::event_work:
    {
      auto event = static_cast<cool::ng::async::detail::event_context*>(static_cast<void*>(aux));
      try
      {
        event->entry_point();
      }
      catch (...)
      { /* noop */ }

      delete event;
      break;
    }

    case cool::ng::async::detail::work_type::task_work:
    {
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

      break;
    }
  }

  start_work();

}

void executor::run(cool::ng::async::detail::work* ctx_)
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
