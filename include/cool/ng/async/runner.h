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

#if !defined(cool_ng_c5876e46_c998_4b2f_9c82_7cf2076f24ac)
#define      cool_ng_c5876e46_c998_4b2f_9c82_7cf2076f24ac

#include <memory>
#include <string>

#include "cool/ng/impl/platform.h"
#include "cool/ng/exception.h"

namespace cool { namespace ng { namespace async {

namespace impl { class executor; }

/**
 * Task scheduling policies for @ref cool::ng::async::runner "runner".
 *
 * A @ref runner can use either sequential or cuncurrent task scheduling policy.
 * The sequential policy instructs the runner to schedule the @ref task "tasks"
 * from its task queue one after another, only scheduling the next task after
 * the executuon of the previous task has completed. The concurrent policy
 * instructs the runner to schedule the execution of the next task as soon as 
 * an idle thread is available in the thread pool, regardless of whether the 
 * execution of the previous task has completed or not.
 *
 * The scheduling policy is only indicative. In particular, there is no guarantee
 * that the platform supports the cuncurrent scheduling, and even if it does, 
 * there is no guarantee that there will be an idle thread available in the 
 * worker thread pool. The only guarantee offered by the RunPolicy is that 
 * the runner using sequential policy <em>will not</em> execute the tasks 
 * from its task queue concurrently.
 */
enum class RunPolicy {
  /**
   * Sequantial scheduling policy where runner executes the tasks one after another.
   */
  SEQUENTIAL,
  /**
   * Concurrent scheduling policy where runner may execute several tasks in parallel.
   */
  CONCURRENT
};

/**
 * A representation of the queue of asynchronously executing tasks.
 *
 * This class is an abstraction representing a queue of asynchronously executing
 * tasks. Althought the actual implementation of the task queue is platform
 * dependent with the implementation details not exposed through API, all
 * platforms share the following common capabilities:
 *   - all runner implementations are capable of executing @ref task "tasks"
 *     sequentially, and some implementations can execute them concurrently
 *   - all runner implementations are thread-safe in a sense that all operations
 *     of this class are thread safe and that @ref task::run "run()" methods
 *     on tasks using the same runner may be called concurrently from several
 *     threads
 *   - all static methods returning runner will return a pointer to valid
 *     runner regardless of the implementation. Some implementations may
 *     return the same runner regardless of the method while other will return
 *     different runners.
 *
 * Reguraly created runner objects represent idependent task queues. However,
 * runner objects created via copy construction are considered to be clones of
 * the original object and refer to the same task queue as the original object.
 * The same is true for runner objects that get a different task queue assigned
 * via copy assignment.
 *
 * The runner provides no public facilities for task scheduling and execution.
 * Tasks are submitted into the runner's task queue via @ref task::run() "run"
 * method of the @ref task.
 */
class runner
{
 public:
  runner(runner&&) = delete;
  runner& operator=(runner&&) = delete;
  /**
   * Construct a new runner object.
   *
   * Constructs a new runner object, optionally with the desired task 
   * scheduling policy.
   *
   * @param policy_ optional parameter, set to RunPolicy::SEQUENTIAL by default.
   *
   * @exception cool::exception::create_failure thrown if a new instance cannot
   *   be created.
   *
   * @note The runner object is created in started state and is immediately
   *   capable of executing tasks.
   */
  dlldecl runner(RunPolicy policy_ = RunPolicy::SEQUENTIAL);

  /**
   * Copy constructor.
   *
   * Constructs a copy of a runner objects. Note that a newly construted runner
   * is considered to be a @em clone of the original object. Both runner objects
   * share the same internal task queue.
   */
  dlldecl runner(const runner&) = default;
  /**
   * Copy assignment operator.
   *
   * The left-hand side runer object will drop the reference to the current
   * internal task queue and receive the reference to the internal task queue
   * of the right-hand side runner object. The previous task queue of the
   * assignee may get destryoed, depending on whether there is another runner object
   * keeping a reference to it and on the platforms task queue destruction
   * strategy.
   */
  dlldecl runner& operator=(const runner&) = default;

  /**
   * Destroys the runner object.
   *
   * Destroying the runner object means dropping a reference to its internal
   * task queue. If this was the only reference to the task queue, the task
   * queue will begin its destruction cycle, but, depending on the platform,
   * may not be destroyed immediatelly. Some platforms will destroy the task
   * queue only after the last task from the queue was run.
   */
  dlldecl virtual ~runner();

  /**
   * Return the name of this runner.
   *
   * Every runner object has a process level unique name.
   */
  dlldecl const std::string& name() const;
  /**
   * Stop executing the tasks from this runner's queue.
   *
   * Currently executing tasks from this runner queue are executed to their
   * completion but new tasks are no longer scheduled for execution.
   * Note that suspending the execution will affect all clones that share the
   * same task queue.
   */
  dlldecl void stop();
  /**
   * Resume execution of tasks from this runner's queue.
   *
   * Note that resuming the execution will affect all clones that share the
   * same task queue.
   */
  dlldecl void start();
  /**
   * Return the task queue implementation.
   *
   * Returns a reference to the internal task queue implementation. Portable
   * applications should avoid using the internal implementation directly.
   */
  const std::shared_ptr<impl::executor>& impl() const;

 private:
  std::shared_ptr<impl::executor> m_impl;
};


#if 0


  /**
   * Returns system-wide runner object with the high priority.
   *
   * @note This runner may execute tasks concurrently.
   */
  dlldecl static ptr sys_high();
  /**
   * Returns system-wide runner object with the default priority.
   *
   * @note This runner may execute tasks concurrently.
   */
  dlldecl static ptr sys_default();
  /**
   * Returns system-wide runner object with the low priority.
   *
   * @note This runner may execute tasks concurrently.
   */
  dlldecl static ptr sys_low();
  /**
   * Returns system-wide runner object with the background (lowest) priority.
   *
   * @note This runner may execute tasks concurrently.
   */
  dlldecl static ptr sys_background();
  /**
   * Returns library default runner.
   *
   * @note This runner executes tasks sequentially.
   */
  dlldecl static ptr cool_default();
#endif

} } } // namespace




#endif
