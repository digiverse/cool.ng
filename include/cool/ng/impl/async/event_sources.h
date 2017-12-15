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

#if !defined(cool_ng_f36abcb0_dda1_42a3_b25a_9beef951523a)
#define      cool_ng_f36abcb0_dda1_42a3_b25a_9beef951523a

#include <functional>
#include <cstdint>
#include <memory>
#include <chrono>

#include "cool/ng/bases.h"
#include "cool/ng/exception.h"

namespace cool { namespace ng { namespace async {

namespace detail {

template <typename RunnerT>
class timer : public cool::ng::util::self_aware<timer<RunnerT>>
            , public itf::timer
            , public impl::cb::timer
{
 public:
  using handler = std::function<void(const std::shared_ptr<RunnerT>&)>;

 public:
  timer(const std::weak_ptr<RunnerT>& r_, const handler& h_ )
    : m_runner(r_), m_handler(h_)
  { /* noop */}

  ~timer()
  {
    if (m_impl)
      m_impl->shutdown();
  }

  void initialize(uint64_t p_, uint64_t l_)
  {
    if (p_ == 0 || !m_handler)
      throw exception::illegal_argument();
    auto r = m_runner.lock();
    if (!r)
      throw exception::runner_not_available();
    m_impl = impl::create_timer(r, this->self(), p_, l_);
  }

  // itf::timer interface
  void start() override
  {
    m_impl->start();
  }
  void stop() override
  {
    m_impl->stop();
  }
  void period(uint64_t p_, uint64_t l_) override
  {
    m_impl->period(p_, l_);
  }
  const std::string& name() const override
  {
    return m_impl->name();
  }

  void shutdown() override
  { /* noop */ }

  // impl::cb::timer interface
  void expired() override
  {
    auto r = m_runner.lock();
    if (r)
    {
      try { m_handler(r); } catch (...) { /* noop */ }
    }
  }

 private:
  std::shared_ptr<itf::timer> m_impl;
  std::weak_ptr<RunnerT>      m_runner;
  handler                     m_handler;
};



} } } } // namespace
#endif

