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
#include <iostream>
#include "cool/ng/async/runner.h"
#include "cool/ng/exception.h"

#define DO_TRACE 0
// #define DO_TRACE 1

#if DO_TRACE == 1
#define TRACE(a, b) std::cout << "---- [" << __LINE__ << "] " << a << ": " << b << "\n"
#else
#define TRACE(a, b)
#endif

namespace cool { namespace ng { namespace async { namespace impl {

namespace {

const PTP_WORK invalid_work = reinterpret_cast<const PTP_WORK>(0x1);
CONSTEXPR_ const int TASK = 1;

}


poolmgr::weak_ptr poolmgr::m_self;
critical_section  poolmgr::m_cs;

poolmgr::poolmgr() : m_pool(nullptr)
{
  InitializeThreadpoolEnvironment(&m_environ);
  m_pool = CreateThreadpool(nullptr);
  if (m_pool == nullptr)
    throw exception::system_error("Failed to create new threadpool");

  // Associate the callback environment with our thread pool.
  SetThreadpoolCallbackPool(&m_environ, m_pool);

  TRACE("poolmgr", "this=" << this << ", env=" << &m_environ);
}

poolmgr::~poolmgr()
{
  TRACE("poolmgr", "to delete poolmgr " << this);

  if (m_pool != nullptr)
    CloseThreadpool(m_pool);

  DestroyThreadpoolEnvironment(&m_environ);

  TRACE("poolmgr", "deleted");
}

void poolmgr::add_environ(PTP_CALLBACK_ENVIRON e_)
{
  TRACE("poolmgr", "pool " << this << " for callback env " << e_);
  SetThreadpoolCallbackPool(e_, m_pool);
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


struct executor_cleanup
{
  executor_cleanup(executor* e_, cool::ng::async::detail::cleanup_context* cc_)
    : ex(e_)
    , cc(cc_)
  { /* NOP */ }

  executor* ex;
  cool::ng::async::detail::cleanup_context* cc;
};


executor::executor(RunPolicy policy_)
    : named("runner") // named("si.digiverse.ng.cool.runner")
    , m_work(nullptr)
    , m_fifo(nullptr)
    , m_pool(poolmgr::get_poolmgr())
    , m_work_in_progress(false)
    , m_active(true)
    , m_lock(SRWLOCK_INIT)
{
  TRACE(name(), "new " << this);

  m_fifo = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1000);
  if (m_fifo == nullptr)
    throw exception::system_error("failed to create new I/O completion port");

  try
  {
    // Create work with the callback environment.
    m_work = CreateThreadpoolWork(task_executor, this, m_pool->get_environ());
    if (m_work.load() == nullptr)
      throw exception::system_error("failed to create new threadpool work object");
  }
  catch (...)
  {
    CloseHandle(m_fifo);
    throw;
  }
}

executor::~executor()
{
  TRACE(name(), "to delete executor " << this);

  PTP_WORK expect = m_work.load();
  if (m_work.compare_exchange_strong(expect, invalid_work) && expect != nullptr)
  {
    WaitForThreadpoolWorkCallbacks(expect, TRUE);
    CloseThreadpoolWork(expect);
  }

  if (m_fifo != nullptr)
  {
    // delete all pending work in fifo queue (a.k.a. completion port)
    LPOVERLAPPED aux;
    DWORD        cmd;
    ULONG_PTR    key;

    while(GetQueuedCompletionStatus(m_fifo, &cmd, &key, &aux, 0))
    {
      // NOTE: it is assumed that the non-empty context_stack will delete all its
      // elements still left on the stack

      delete static_cast<cool::ng::async::detail::work*>(static_cast<void*>(aux));
    }

    // now close the completion port
    CloseHandle(m_fifo);
  }

  TRACE(name(), "deleted");
}

VOID CALLBACK executor::event_cb(PTP_CALLBACK_INSTANCE instance_, PVOID pv_, PTP_WORK work_)
{
  TRACE("executor", "event_cb for work: " << work_);
  auto event = static_cast<cool::ng::async::detail::event_context*>(static_cast<void *>(pv_));
  try { event->entry_point(); } catch (...) { /* noop */ }
  delete event;

  CloseThreadpoolWork(work_);
}

VOID CALLBACK executor::cleanup_cb(PTP_CALLBACK_INSTANCE instance_, PVOID pv_, PTP_WORK work_)
{
  TRACE("executor", "cleanup_cb for work: " << work_);
  auto excc = static_cast<executor_cleanup*>(static_cast<void*>(pv_));

  auto cleanup = excc->cc;
  auto env = cleanup->environment();
  try { cleanup->entry_point(); } catch (...) { /* noop */ }

  AcquireSRWLockExclusive(&excc->ex->m_lock);
  excc->ex->m_cleanup_environments.erase(env);
  ReleaseSRWLockExclusive(&excc->ex->m_lock);

  delete cleanup;
  delete excc;

  CloseThreadpoolWork(work_);
}

// executor for task::run() that runs in the thread pool
VOID CALLBACK executor::task_executor(PTP_CALLBACK_INSTANCE instance_, PVOID pv_, PTP_WORK work_)
{
  static_cast<executor*>(pv_)->task_executor(work_);
}

void executor::task_executor(PTP_WORK w_)
{
  LPOVERLAPPED aux;
  DWORD        cmd;
  ULONG_PTR    key;

  if (!GetQueuedCompletionStatus(m_fifo, &cmd, &key, &aux, 100))
  {
    PTP_WORK expect = nullptr;
    // somebody else must have created new work or, more likely, invalid_work has
    // been set to signal end of execution
    if (!m_work.compare_exchange_strong(expect, w_))
    {
      WaitForThreadpoolWorkCallbacks(w_, FALSE);
      CloseThreadpoolWork(w_);
    }
    return;
  }

  auto work = static_cast<cool::ng::async::detail::work*>(static_cast<void*>(aux));
  switch (work->type())
  {
    case cool::ng::async::detail::work_type::event_work:
    {
      // submit a new work to caller's environment, so that when CloseThreadpoolCleanupGroupMembers()
      // is called, it will clean up all items
      AcquireSRWLockShared(&m_lock);
      
      auto event = static_cast<cool::ng::async::detail::event_context*>(static_cast<void*>(aux));
      auto env = event->environment();
      TRACE(name(), "new work[" << env << "]: " << work);
      if (m_cleanup_environments.find(env) != m_cleanup_environments.end())
      {
        TRACE(name(), "environment " << env << " is being cleaned up, not submitting new work " << work);
        ReleaseSRWLockShared(&m_lock);
        return;
      }

      PTP_WORK w = CreateThreadpoolWork(event_cb, event, static_cast<PTP_CALLBACK_ENVIRON>(env));
      TRACE(name(), "event[" << env << "]: " << w);
      SubmitThreadpoolWork(w);

      ReleaseSRWLockShared(&m_lock);
      break;
    }

    case cool::ng::async::detail::work_type::cleanup_work:
    {
      // submit the cleanup work to executor's environment
      AcquireSRWLockExclusive(&m_lock);

      auto cleanup = static_cast<cool::ng::async::detail::cleanup_context*>(static_cast<void*>(aux));
      auto env = cleanup->environment();
      if (m_cleanup_environments.find(env) != m_cleanup_environments.end())
      {
        TRACE(name(), "environment " << env << " is already being cleaned up, ignoring cleanup request " << work);
        ReleaseSRWLockExclusive(&m_lock);
        return;
      }

      m_cleanup_environments.insert(env);
      TRACE(name(), "new work[" << env << "]: " << work);

      PTP_WORK w = CreateThreadpoolWork(cleanup_cb, new executor_cleanup(this, cleanup), m_pool->get_environ());
      TRACE(name(), "cleanup[" << env << "]: " << w);
      SubmitThreadpoolWork(w);

      ReleaseSRWLockExclusive(&m_lock);
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
        try { context->entry_point(r, context); } catch (...) { /* noop */ }
        if (stack->empty())
          delete stack;
        else
          r->impl()->run(stack);
      }
      else
        delete stack;

      break;
    }
  }

  if (m_work.load() == invalid_work)  // invalid work signals end of execution
  {
    CloseThreadpoolWork(w_);
    return;
  }

  SubmitThreadpoolWork(w_);
}

void executor::run(cool::ng::async::detail::work* ctx_)
{
  TRACE(name(), "run: " << ctx_);

  PostQueuedCompletionStatus(m_fifo, TASK, NULL, reinterpret_cast<LPOVERLAPPED>(ctx_));

  PTP_WORK w = m_work;
  if (w != nullptr && w != invalid_work)
  {
    if (m_work.compare_exchange_strong(w, nullptr))
      SubmitThreadpoolWork(w);
  }
}


} } } } // namespace
