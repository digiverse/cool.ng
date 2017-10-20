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

#if !defined(cool_ng_d2aa94ac_15ec_4748_9d69_a7d096d1b861)
#define      cool_ng_d2aa94ac_15ec_4748_9d69_a7d096d1b861

#define WIN32_MEAN_AND_LEAN
#include <windows.h>

#include <atomic>
#include <memory>
#include "cool/ng/bases.h"
#include "cool/ng/async/runner.h"
#include "cool/ng/impl/async/context.h"

namespace cool { namespace ng { namespace async { namespace impl {

class poolmgr;

class critical_section
{
  critical_section(const critical_section&) = delete;
  critical_section(critical_section&&) = delete;
  critical_section& operator =(const critical_section&) = delete;
  critical_section& operator =(critical_section&&) = delete;

 public:
  inline critical_section(long spin_count = 1000000)
  {
    InitializeCriticalSectionAndSpinCount(&m_cs, spin_count);
  }
  inline ~critical_section()
  {
    DeleteCriticalSection(&m_cs);
  }

  // BasicLockable requirements
  void lock()
  {
    EnterCriticalSection(&m_cs);
  }
  void unlock()
  {
    LeaveCriticalSection(&m_cs);
  }

  // Lockable requirements
  bool try_lock()
  {
#pragma warning( suppress: 4800 )
    return static_cast<bool>(TryEnterCriticalSection(&m_cs));
  }

private:
  CRITICAL_SECTION m_cs;
};

class executor : public ::cool::ng::util::named
{
  using queue_type = HANDLE;

 public:
  executor(RunPolicy policy_);
  ~executor();

  void start();
  void stop();
  void run(detail::context_stack*);
  bool is_system() const { return false; }

 private:
  static VOID CALLBACK task_executor(PTP_CALLBACK_INSTANCE instance_, PVOID pv_, PTP_WORK work_);
  void task_executor();
  void start_work();
  void check_create_thread_pool();
  void check_delete_thread_pool();

 private:
  PTP_WORK          m_work;
  queue_type        m_fifo;
  std::atomic<bool> m_work_in_progress;

  static unsigned int             m_refcnt;
  static std::unique_ptr<poolmgr> m_pool;
  static critical_section         m_cs;

  std::atomic<bool> m_active;
};

} } } }// namespace

#endif

