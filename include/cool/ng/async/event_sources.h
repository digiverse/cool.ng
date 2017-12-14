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

#include <memory>
#include <cstdint>
#include <functional>

#include "cool/ng/impl/platform.h"
#include "cool/ng/ip_address.h"
#include "cool/ng/impl/async/net_types.h"
#include "cool/ng/impl/async/event_sources.h"

#include "net/stream.h"
#include "net/server.h"

namespace cool { namespace ng { namespace async {
/**
 * Timer event source.
 *
 * Timer objects periodically, with a period @a p_, submit a task to @ref runner
 * @a r_ that calls the user @em Callable @a h_.
 *
 * @note Upon creation the timer object is inactive and must be explicitly
 *   started using @ref start().
 * @note Timer objects created via copy construction or copy assignment
 *   are clones and refer to the same underlying timer implementation. Any
 *   changes made through one of the clones will affect all clones.
 * @note The internal resolution of @ref timer class is one microsecond. The actual
 *   resolution is platform dependent.
 */
class timer
{
 public:
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
   * @tparam RunnerT <b>RunnerT</b> is the actual type of the @ref runner to
   *         use to schedule calls to user @em Callable.
   * @tparam HandlerT <b>HandlerT</b> is the actual type of the user @em Callable
   *         and must be assignable to the following functional type:
   * ~~~{.c}
   *     std::function<void(const std::shared_ptr<RunnerT>&)>
   * ~~~
   * @tparam RepT <b>RepT</b> is mapped into @c Rep template parameter of
   *         @c std::chrono::duration class template.
   * @tparam PeriodT <b>PeriodT</b> is mapped into @c Period template parameter of
   *         @c std::chrono::duration class template.
   *
   * @param r_ the @ref runner to use to shedule t he periodic task
   * @param h_ the user @em Callable to be called from periodic task.
   * @param p_ period of the timer.
   *
   * @throw exception::illegal_argument thrown if period, when converted
   *        to microseconds, is equal to 0 or if the handler @a h_ is empty
   * @throw exception::runner_not_available thrown if the runner @a r_ no longer
   *        exists at the moment of construction
   * @note Upon creating the timer is inactive and must explicitly be activated
   *       via @ref start().
   * @note The leeway of the timer is set to 10% of the period or to at least
   *       1 microsecond.
   */
  template <typename RunnerT, typename HandlerT, typename RepT, typename PeriodT>
  timer(const std::weak_ptr<RunnerT>& r_
      , const HandlerT& h_
      , const std::chrono::duration<RepT, PeriodT>& p_)
    : timer(r_
          , h_
          , static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(p_).count()))
  { /* noop */ }

  /**
   * Create a timer object.
   *
   * Creates a timer object with the specified period.
   *
   * @tparam RunnerT <b>RunnerT</b> is the actual type of the @ref runner to
   *         use to schedule calls to user @em Callable.
   * @tparam HandlerT <b>HandlerT</b> is the actual type of the user @em Callable
   *         and must be assignable to the following functional type:
   * ~~~{.c}
   *     std::function<void(const std::shared_ptr<RunnerT>&)>
   * ~~~
   * @tparam RepT <b>RepT</b> is mapped into @c Rep template parameter of
   *         @c std::chrono::duration class template.
   * @tparam PeriodT <b>PeriodT</b> is mapped into @c Period template parameter of
   *         @c std::chrono::duration class template.
   * @tparam Rep2T <b>Rep2T</b> is mapped into @c Rep template parameter of
   *         @c std::chrono::duration class template.
   * @tparam Period2T <b>Period2T</b> is mapped into @c Period template parameter of
   *         @c std::chrono::duration class template.
   *
   * @param r_ the @ref runner to use to shedule t he periodic task
   * @param h_ the user @em Callable to be called from periodic task.
   * @param p_ period of the timer.
   * @param l_ leeway, the interval the system can defer the timer
   *
   * @note Upon creating the timer is inactive and must explicitly be activated
   *       via @ref start().
   */
  template <typename RunnerT, typename HandlerT, typename RepT, typename PeriodT, typename Rep2T, typename Period2T>
  timer(const std::weak_ptr<RunnerT>& r_
      , const HandlerT& h_
      , const std::chrono::duration<RepT, PeriodT>& p_
      , const std::chrono::duration<RepT, PeriodT>& l_)
    : timer(r_
          , h_
          , static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(p_).count())
          , static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(l_).count()))
  { /* noop */  }

  /**
   * Create a timer object.
   *
   * Creates a timer object with the specified period.
   *
   * @tparam RunnerT <b>RunnerT</b> is the actual type of the @ref runner to
   *         use to schedule calls to user @em Callable.
   * @tparam HandlerT <b>HandlerT</b> is the actual type of the user @em Callable
   *         and must be assignable to the following functional type:
   * ~~~{.c}
   *     std::function<void(const std::shared_ptr<RunnerT>&)>
   * ~~~
   *
   * @param r_ the @ref runner to use to shedule t he periodic task
   * @param h_ the user @em Callable to be called from periodic task.
   * @param p_ period of the timer, in microseconds.
   *
   * @note Upon creating the timer is inactive and must explicitly be activated
   *       via @ref start().
   * @note The leeway of the timer is set to 10% of the period or to at least
   *       1 microsecond.
   */
  template <typename RunnerT, typename HandlerT>
  timer(const std::weak_ptr<RunnerT>& r_
      , const HandlerT& h_
      , uint64_t p_)
    : timer(r_, h_, p_, p_ / 10 == 0 ? 1 : p_ / 10)
  { /* noop */  }

  /**
   * Create a timer object.
   *
   * Creates a timer object with the specified period.
   *
   * @tparam RunnerT <b>RunnerT</b> is the actual type of the @ref runner to
   *         use to schedule calls to user @em Callable.
   * @tparam HandlerT <b>HandlerT</b> is the actual type of the user @em Callable
   *         and must be assignable to the following functional type:
   * ~~~{.c}
   *     std::function<void(const std::shared_ptr<RunnerT>&)>
   * ~~~
   *
   * @param r_ the @ref runner to use to shedule t he periodic task
   * @param h_ the user @em Callable to be called from periodic task.
   * @param p_ period of the timer, in microseconds.
   * @param l_ leeway, the interval the system can defer the timer, in microseconds
   *
   * @note Upon creating the timer is inactive and must explicitly be activated
   *       via @ref start().
   */
  template <typename RunnerT, typename HandlerT>
  timer(const std::weak_ptr<RunnerT>& r_
      , const HandlerT& h_
      , uint64_t p_
      , uint64_t l_)
  {
    auto impl = cool::ng::util::shared_new<detail::timer<RunnerT>>(r_, h_);
    impl->initialize(p_, l_);
    m_impl = impl;
  }
  /**
   * Change the period of the timer.
   *
   * When changing the period the new period becomes effective after the call
   * to @ref start().
   *
   * @tparam RepT <b>RepT</b> is mapped into @c Rep template parameter of
   *         @c std::chrono::duration class template.
   * @tparam PeriodT <b>PeriodT</b> is mapped into @c Period template parameter of
   *         @c std::chrono::duration class template.
   *
   * @param p_  The period of the timer.
   *
   * @note The leeway of the timer is set to 10% of the period or to at least
   *       1 microsecond..
   */
  template <typename RepT, typename PeriodT>
  void period(const std::chrono::duration<RepT, PeriodT>& p_)
  {
    period(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(p_).count()));
  }

  /**
   * Set or change the period of the timer.
   *
   * Sets or changes the period of the timer. For the timers created through one
   * of the constructors that do not set the timer period at the construction
   * time the period must be set before they can be started. When changing the
   * period the new period becomes effective after the call to start().
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
   * @param p_ the period of the timer. in microseconds
   * @param l_ leeway, the amount of time the system can defer the timer, in microseconds
   *
   * @exception cool::exception::illegal_argument Thrown if the period is set to 0.
   */
  template <typename RepT, typename PeriodT, typename Rep2T ,typename Period2T>
  void period(const std::chrono::duration<RepT, PeriodT>& p_,
                    const std::chrono::duration<Rep2T, Period2T>& l_)
  {
    period(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(p_).count())
         , static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(l_).count()));
  }

  /**
   * Set or change the period of the timer.
   *
   * Sets or changes the period of the timer. For the timers created through one
   * of the constructors that do not set the timer period at the construction
   * time the period must be set before they can be started. When changing the
   * period the new period becomes effective after the call to start().
   *
   * @param p_ The period of the timer in microseconds.
   * @param l_ leeway, the amount of time, in microseconds, the system can defer
   *           the timer.
   *
   * @exception cool::exception::illegal_argument Thrown if the period is set to 0.
   *
   * @note The leeway parameter is optional. If not specified it is set to 10% of
   *   the period or to at least 1 microsecond.
   */
  dlldecl void period(uint64_t p_, uint64_t l_ = 0);

  /**
   * Start or restart the timer.
   *
   * Starts the timer or the first time, or restarts it. In either case the timer
   * will trigger one full period after the call to start().
   *
   * @exception cool::exception::illegal_state Thrown if the timer period is not set.
   *
   * @note start() is required after the period of the timer is changed.
   */
  dlldecl void start();

  /**
   * Suspend the timer.
   *
   * Suspends the timer by disabling the calls to the user callback. The timer
   * is till running and is triggered at each period but without calling the
   * user callback.
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
   * Each timer has a unique name.
   */
  dlldecl const std::string& name() const;

 private:
  std::shared_ptr<detail::itf::timer> m_impl;
};

} } } // namespace

#endif
