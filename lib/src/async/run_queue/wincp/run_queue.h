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
#include <unordered_set>

#include "cool/ng/bases.h"
#include "critical_section.h"

/*
  Notes on run_queue:

  The run_queue class is internal to Cool.NG library and should not be used
  directly by the user code. It is not a part of Cool.NG API and its may change
  in the future without notice. This text documents the run_queue to Cool.NG
  library developers.

  1.Public methods

  1.1 Methods for Creation and Destruction

  These methods are static.

  1.1.1 create(cnst std::string& name_)

  Creates a run_queue with a giben name and returns a shared pointer to it. Use
  create() method to create a new run_queue. Do not use ctor directly; the ctor
  is public only to permit the use of std::make_shared inside create().

  1.1.1 release(const std::shared_ptr& queue_)

  Releases the run_queue. Note that without calling this method the run_queue may
  not get destroyed. The behavior of dropping the shared_ptr without prior call
  to release() is platform dependent and undefined. The behavior on posting a
  new task after the call of release() is platform dependent and undefined.
  Calling release() on the already released queue has no effect.

  release() is thread safe. Multiple threads may call release() on the same
  run_queue instance simultaneously.

  1.2 Posting Tasks to Execute

  1.2.1 enqueue(executor exe_, deleter del_, void* data_)

  Enqueues the request to call the execution function exe_ with the data pointer
  specified as data_ as soon as possible but after the previous request posted to
  this run_queue has completed. After the call to execution function exe_ completes
  the data will be deleted by by the deletion function del_. While the calls to
  exe_ and del_ will be done from the context of the same thread it is impossible
  to predict from which thread. Either del_ or data_, or both, may be nullptr. If
  any is nullptr, the deletion function will not be called. The behavior on
  enqueueing a new task after a call to release() is platform dependent and
  undefined.

  enqueue() is thread safe. This method, or any other thread-safe methods, may
  be called simultaneously from multiple threads.

  1.3 Starting and Stopping the Task Execution

  When created, the run_queue is active and will be executing the tasks as soon as
  they are posted.

  1.3.1 stop()

  Will suspend the execution of enqueued tasks and render the run_queue instance
  inactive. When inactive, new tasks may still be posted to the run_queue but
  their execution will not proceed as long as the run_queue is suspended. Note
  that the task being executed at the time stop() is called will be executed to
  its completion before the execution is suspended. If the run_queue is already
  inactive this call has no effect.

  stop() is thread safe. This method, or any other thread-safe methods, may
  be called simultaneously from multiple threads.

  1.3.2 start()

  Will resume the execution of the enqueued tasks thus rendering the run_queue
  instance active.  IF the run_queue is already active this call has no effect.

  start() is thread safe. This method, or any other thread-safe methods, may
  be called simultaneously from multiple threads.

  1.3.3 is_active()

  Returns true if the run_queue instance is active, false if not. Note that due
  to the multi-thread capabilities of run_queue the return value may be incorrect
  even before this call returns the value.

  is_active() is thread safe. This method, or any other thread-safe methods, may
  be called simultaneously from multiple threads.

  2. Behavior on Destruction

  The call to release(), which is mandatory before the queue instance is
  destroyed, will activate the inactive run_queue's. In consequence, the
  run_queue's will execute all enqueued tasks before it is destroyed, even if the
  user code drops its shared_ptr before the last task is executed. While the exact
  moment of when the run_queue instance will cease to exist is platform dependent
  and thus undefined, it will certainly no longer exist immediatelly after the
  execution of the last task completes.
*/

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

class run_queue : public ::cool::ng::util::named
{
 public:
  using deleter = void (*)(void*);
  using executor = void (*)(void*);
  using pointer = std::shared_ptr<run_queue>;

 private:
  enum : int {
      RELEASED = 0x08
    , EMPTY    = 0x04
    , ACTIVE   = 0x02
    , BUSY     = 0x01
    , NOT_EMPTY_ACTIVE_NOT_BUSY = ACTIVE
    , NOT_EMPTY_ACTIVE_BUSY     = ACTIVE | BUSY
    , EMPTY_ACTIVE_NOT_BUSY     = EMPTY | ACTIVE
    , RELEASED_NOT_EMPTY_ACTIVE_NOT_BUSY = RELEASED | ACTIVE
    , RELEASED_NOT_EMPTY_ACTIVE_BUSY     = RELEASED | ACTIVE | BUSY
  };

  struct context
  {
    context();
    context(executor exe_, deleter del_, void* data_, const pointer& q_);
    ~context();

    void clear();

    executor m_executor;
    deleter  m_deleter;
    void     *m_data;
    pointer  m_queue;
  };

 public:
  static pointer create(const std::string& name_ = "si.digiverse.cool.ng.runner");
  static void release(const pointer& arg);

  // --- Do not use ctor directly; use create instead.
  // --- Ctor is public only to permit the use of std::make_shared
  run_queue(const std::string& name_);
  ~run_queue();

  void enqueue(executor exe_, deleter del_, void* data_);
  void stop();
  void start();
  bool is_active() const { return (m_status.load() & ACTIVE) != 0; }

 private:
  void check_submit_next();
  static VOID CALLBACK run_next(PTP_CALLBACK_INSTANCE instance_, PVOID pv_, PTP_WORK work_);
  void run_next(PTP_WORK w_);

 private:
  std::atomic<int> m_status;

  PTP_WORK         m_work;
  HANDLE           m_fifo;
  poolmgr::ptr     m_pool;

  pointer          m_self;
};


#if 0
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
  static VOID CALLBACK event_cb(PTP_CALLBACK_INSTANCE instance_, PVOID pv_, PTP_WORK work_);
  static VOID CALLBACK cleanup_cb(PTP_CALLBACK_INSTANCE instance_, PVOID pv_, PTP_WORK work_);

 private:
  std::atomic<PTP_WORK> m_work;
  queue_type        m_fifo;
  poolmgr::ptr      m_pool;

  std::atomic<bool> m_work_in_progress;
  std::atomic<bool> m_active;

  SRWLOCK m_lock;
  std::unordered_set<void*> m_cleanup_environments;
};
#endif
} } } }// namespace

#endif

