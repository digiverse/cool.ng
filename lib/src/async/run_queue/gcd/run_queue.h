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

#if !defined(cool_ng_d2de9442_15ec_4748_9d69_a7d086d1b861)
#define      cool_ng_d2de9442_15ec_4748_9d69_a7d086d1b861

#include <atomic>
#include <memory>
#include <string>
#include <dispatch/dispatch.h>
#include "cool/ng/bases.h"

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

class run_queue : public ::cool::ng::util::named
{
 public:
  using deleter =  void (*)(void*);
  using executor = void (*)(void*);
  using pointer = std::shared_ptr<run_queue>;

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
  bool is_active() const { return m_active.load(); }

 private:
  std::atomic<bool> m_active;
  dispatch_queue_t  m_queue;
};

} } } }// namespace

#endif

