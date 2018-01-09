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
#include "critical_section.h"

namespace cool { namespace ng { namespace async { namespace impl {

class poolmgr
{
 public:
  using ptr      = std::shared_ptr<poolmgr>;
  using weak_ptr = std::weak_ptr<poolmgr>;

 public:
  poolmgr();
  ~poolmgr();
  PTP_CALLBACK_ENVIRON get_environ() { return &m_environ; }
  void add_environ(PTP_CALLBACK_ENVIRON e_);
  static ptr get_poolmgr();

 private:
  PTP_POOL                        m_pool;
  TP_CALLBACK_ENVIRON             m_environ;
  static weak_ptr                 m_self;
  static critical_section         m_cs;
};


class executor : public ::cool::ng::util::named
{
  using queue_type = HANDLE;

 public:
  executor(RunPolicy policy_);
  ~executor();

  void run(detail::work*);
  bool is_system() const { return false; }

 private:
  static VOID CALLBACK task_executor(PTP_CALLBACK_INSTANCE instance_, PVOID pv_, PTP_WORK work_);
  void task_executor(PTP_WORK w_);

 private:
  std::atomic<PTP_WORK> m_work;
  queue_type        m_fifo;
  poolmgr::ptr      m_pool;

  std::atomic<bool> m_work_in_progress;
  std::atomic<bool> m_active;
};

} } } }// namespace

#endif

