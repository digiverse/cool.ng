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

#if !defined(cool_ng_f36defb0_dda1_4ce1_b25a_943f5deed23b)
#define      cool_ng_f36defb0_dda1_4ce1_b25a_943f5deed23b

#include <string>
#include <memory>
#include <cstdint>
#include <functional>
#include <chrono>

#include "cool/ng/impl/platform.h"
#include "cool/ng/ip_address.h"
#include "cool/ng/impl/async/event_sources_types.h"

#include "task.h"
#include "net/stream.h"
#include "net/server.h"

namespace cool { namespace ng { namespace async {
/**
 * Timer event source.
 *
 * Timer objects periodically, with a specified period, schedule a user specified
 * task for execution. The internal period resolution of @ref timer class is one
 * microsecond. The actual resolution is platform dependent, as follows:
 *   Platform        | Actual Resolution
 *   ----------------|-----------------
 *    MacOS/OS X     | 1 microsecond
 *    Linux          | 1 microsecond
 *    MS Windows     | 1 millisecond
 * When transforming period and leeway values to the actual resolution, the
 * result is rounded to the nearest actual resolution unit, but is always at
 * least 1 resolution unit. Thus both 100 microseconds and 1200 microseconds
 * would result in 1 millisecond actual period on Microsoft Windows.
 *
 * @note Upon creation the timer object is inactive and must be explicitly
 *   started using @ref start().
 * @note Timer objects created via copy construction or copy assignment
 *   are clones and refer to the same underlying timer implementation. Any
 *   changes made through one of the clones will affect all clones.
 */
class timer
{
 public:
  /**
   * User task type.
   *
   * The type of the task the user shall specify to be run periodically. It
   * corresponds to the following @ref cool::ng::async::task "task" type:
   * ~~~
   *    cool::ng::async::task<void, void>
   * ~~~
   */
  using task_type = detail::itf::timer::task_type;

 public:
  dlldecl ~timer();
  /**
   * Default constructor to allow @ref timer "timers" to be stored in standard
   * library containers.
   *
   * This constructor constructs an empty, non-functional @ref timer. The only
   * way to make it functional is to replace it with a functional timer using
   * copy assignment or move assignment operator.
   *
   * @note The only permitted operations on an empty timer are copy assignment
   *   and the @ref operator bool() "bool" conversion operator. Any other
   *   operation will throw @ref cool::ng::exception::empty_object "empty_object"
   *   exception.
   */
  timer() { /* noop */ }

  /**
   * Create a timer object.
   *
   * Creates a timer object with the specified period.
   *
   * @tparam RepT <b>RepT</b> is mapped into @c Rep template parameter of
   *         @c std::chrono::duration class template.
   * @tparam PeriodT <b>PeriodT</b> is mapped into @c Period template parameter of
   *         @c std::chrono::duration class template.
   *
   * @param task_ the user @ref task to execute periodically
   * @param period_ period of the timer.
   *
   * @throw exception::illegal_argument thrown if period, when converted
   *        to microseconds, is equal to 0 or if the task @a task_ is empty
   * @throw exception::threadpool_failure if threadpool operation failed. This
   *        is an MS Windows specific exception.
   *
   * @note Upon creating the timer is inactive and must explicitly be activated
   *       via @ref start().
   * @note The leeway of the timer is set to 10% of the period or to at least
   *       1 microsecond.
   */
  template <typename RepT, typename PeriodT>
  timer(const task_type& task_
      , const std::chrono::duration<RepT, PeriodT>& period_)
    : timer(task_
          , static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(period_).count()))
  { /* noop */ }

  /**
   * Create a timer object.
   *
   * Creates a timer object with the specified period and the specified leeway.
   *
   * @tparam RunnerT <b>RunnerT</b> is the actual type of the @ref runner to
   *         use to schedule calls to user @em Callable.
   * @tparam RepT <b>RepT</b> is mapped into @c Rep template parameter of
   *         @c std::chrono::duration class template.
   * @tparam PeriodT <b>PeriodT</b> is mapped into @c Period template parameter of
   *         @c std::chrono::duration class template.
   * @tparam Rep2T <b>Rep2T</b> is mapped into @c Rep template parameter of
   *         @c std::chrono::duration class template.
   * @tparam Period2T <b>Period2T</b> is mapped into @c Period template parameter of
   *         @c std::chrono::duration class template.
   *
   * @throw exception::illegal_argument thrown if period, when converted
   *        to microseconds, is equal to 0 or if the task @a task_ is empty
   * @throw exception::threadpool_failure if threadpool operation failed. This
   *        is an MS Windows specific exception.
   *
   * @param task_ the user @ref task to execute periodically
   * @param period_ period of the timer.
   * @param leeway_ leeway, the interval the system can defer the timer
   *
   * @note Upon creating the timer is inactive and must explicitly be activated
   *       via @ref start().
   */
  template <typename RepT, typename PeriodT, typename Rep2T, typename Period2T>
  timer(const task_type& task_
      , const std::chrono::duration<RepT, PeriodT>& period_
      , const std::chrono::duration<RepT, PeriodT>& leeway_)
    : timer(task_
          , static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(period_).count())
          , static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(leeway_).count()))
  { /* noop */  }

  /**
   * Create a timer object.
   *
   * Creates a timer object with the specified period.
   *
   * @param task_ the user @ref task to execute periodically
   * @param period_ period of the timer, in microseconds.
   *
   * @throw exception::illegal_argument thrown if period, when converted
   *        to microseconds, is equal to 0 or if the task @a task_ is empty
   * @throw exception::threadpool_failure if threadpool operation failed. This
   *        is an MS Windows specific exception.
   *
   * @note Upon creating the timer is inactive and must explicitly be activated
   *       via @ref start().
   * @note The leeway of the timer is set to 10% of the period or to at least
   *       1 microsecond.
   */
  timer(const task_type& task_, uint64_t period_)
    : timer(task_, period_, period_ / 10 == 0 ? 1 : period_ / 10)
  { /* noop */  }

  /**
   * Create a timer object.
   *
   * Creates a timer object with the specified period and the specified leeway.
   *
   * @param task_ the user @ref task to execute periodically
   * @param period_ period of the timer, in microseconds.
   * @param leeway_ leeway, the interval the system can defer the timer, in microseconds
   *
   * @throw exception::illegal_argument thrown if period, when converted
   *        to microseconds, is equal to 0 or if the task @a task_ is empty
   * @throw exception::threadpool_failure if threadpool operation failed. This
   *        is an MS Windows specific exception.
   *
   * @note Upon creating the timer is inactive and must explicitly be activated
   *       via @ref start().
   */
  dlldecl timer(const task_type& task_, uint64_t period_, uint64_t leeway_);
  /**
   * Change the period of the timer.
   *
   * When changing the period the new period becomes effective after the next
   * call to @ref start().
   *
   * @tparam RepT <b>RepT</b> is mapped into @c Rep template parameter of
   *         @c std::chrono::duration class template.
   * @tparam PeriodT <b>PeriodT</b> is mapped into @c Period template parameter of
   *         @c std::chrono::duration class template.
   *
   * @param period_  The period of the timer.
   *
   * @note The leeway of the timer is set to 10% of the period or to at least
   *       1 microsecond..
   */
  template <typename RepT, typename PeriodT>
  void period(const std::chrono::duration<RepT, PeriodT>& period_)
  {
    period(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(period_).count()));
  }

  /**
   * Set or change the period of the timer.
   *
   * When changing the period the new period becomes effective after the next
   * call to @ref start().
   *
   * @tparam RepT <b>RepT</b> is mapped into @c Rep template parameter of
   *         @c std::chrono::duration class template.
   * @tparam PeriodT <b>PeriodT</b> is mapped into @c Period template parameter of
   *         @c std::chrono::duration class template.
   * @tparam Rep2T <b>Rep2T</b> is mapped into @c Rep template parameter of
   *         @c std::chrono::duration class template.
   * @tparam Period2T <b>Period2T</b> is mapped into @c Period template parameter of
   *         @c std::chrono::duration class template.
   *
   * @param period_ the period of the timer. in microseconds
   * @param leeway_ leeway, the amount of time the system can defer the timer, in microseconds
   *
   * @exception cool::exception::illegal_argument Thrown if the period is set to 0.
   */
  template <typename RepT, typename PeriodT, typename Rep2T ,typename Period2T>
  void period(const std::chrono::duration<RepT, PeriodT>& period_,
                    const std::chrono::duration<Rep2T, Period2T>& leeway_)
  {
    period(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(period_).count())
         , static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(leeway_).count()));
  }

  /**
   * Set or change the period of the timer.
   *
   * When changing the period the new period becomes effective after the next
   * call to @ref start().
   *
   * @param period_ The period of the timer in microseconds.
   * @param leeway_ leeway, the amount of time, in microseconds, the system can defer
   *           the timer.
   *
   * @exception cool::exception::illegal_argument Thrown if the period is set to 0.
   *
   * @note The leeway parameter is optional. If not specified it is set to 10% of
   *   the period or to at least 1 microsecond.
   */
  dlldecl void period(uint64_t period_, uint64_t leeway_ = 0);

  /**
   * Start or restart the timer.
   *
   * Starts the timer or the first time, or restarts it. In either case the timer
   * will trigger one full period after the call to start().
   *
   * @note A call to start() is required after the period of the timer is changed
   *       in order to activate the new period.
   */
  dlldecl void start();

  /**
   * Suspend the timer.
   *
   * Suspends the timer by disabling the calls to the user callback. Depending on
   * the platform, the timer may still running and be triggered at each period,
   * but without calling the user callback.
   */
  dlldecl void stop();

  /**
   * Empty timer predicate.
   *
   * @return true if this @ref timer is properly created and functional, false if empty.
   */
  dlldecl explicit operator bool() const;

  /**
   * Return timer's name.
   *
   * Each timer instance, except empty timers, has a unique name.
   */
  dlldecl const std::string& name() const;

 private:
  std::shared_ptr<detail::itf::timer> m_impl;
};

} } } // namespace

#endif
