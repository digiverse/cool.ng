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

#if !defined(cool_ng_f36abcb0_bbc1_42a3_b25a_9beef951523a)
#define      cool_ng_f36abcb0_bbc1_42a3_b25a_9beef951523a

#include <memory>
#include <functional>

#if !defined(WINDOWS_TARGET)
#include <dispatch/dispatch.h>
#endif

#include "cool/ng/ip_address.h"
#include "cool/ng/bases.h"
#include "cool/ng/impl/platform.h"
#include "cool/ng/async/runner.h"

#include "event_sources_types.h"

namespace cool { namespace ng { namespace async { namespace net {

namespace detail {

template <typename RunnerT>
class stream : public itf::connected_writable
             , public impl::cb::stream
             , public cool::ng::util::self_aware<stream<RunnerT>>
{
 public:
  using wr_handler    = typename detail::types<RunnerT>::write_handler;
  using rd_handler    = typename detail::types<RunnerT>::read_handler;
  using event_handler = typename detail::types<RunnerT>::event_handler;

 public:
  stream(const std::weak_ptr<RunnerT>& runner_
       , const rd_handler& rh_
       , const wr_handler& wh_
       , const event_handler& eh_)
      : m_runner(runner_), m_rhandler(rh_), m_whandler(wh_), m_oob(eh_)
  { /* noop */ }
  void initialize(const cool::ng::net::ip::address& addr_, uint16_t port_, void* buf_, std::size_t bufsz_)
  {
    auto r = m_runner.lock();
    if (r)
    {
      m_impl = impl::create_stream(r, addr_, port_, this->self(), buf_, bufsz_);
    }
    else
      throw cool::ng::exception::runner_not_available();
  }
 
  void initialize(void* buf_, std::size_t bufsz_)
  {
    auto r = m_runner.lock();
    if (r)
    {
      m_impl = impl::create_stream(r, this->self(), buf_, bufsz_);
    }
    else
      throw cool::ng::exception::runner_not_available();
  }

  ~stream()
  {
    if (m_impl)
    {
      m_impl->shutdown();
    }
  }

  //--- connected_writable interface
  void shutdown() override
  {
    m_impl->shutdown();
  }
  const std::string& name() const override
  {
    return m_impl->name();
  }
  inline void write(const void* data, std::size_t size) override
  {
    m_impl->write(data, size);
  }
  inline void connect(const cool::ng::net::ip::address& addr_, uint16_t port_) override
  {
    m_impl->connect(addr_, port_);
  }
  inline void disconnect() override
  {
    m_impl->disconnect();
  }
  inline void set_handle(cool::ng::net::handle h_) override
  {
    m_impl->set_handle(h_);
  }
  //--- cb::stream interface
  void on_read(void*& buf_, std::size_t& size_) override
  {
    if (!m_rhandler)
      return;

    auto r = m_runner.lock();
    if (r)
      try { m_rhandler(r, buf_, size_); } catch (...) { /* noop */ }
  }
  void on_write(const void* buf_, std::size_t size_) override
  {
    if (!m_whandler)
      return;

    auto r = m_runner.lock();
    if (r)
      try { m_whandler(r, buf_, size_); } catch (...) { /* noop */ }
  }
  void on_event(detail::oob_event evt, const std::error_code& e) override
  {
    if (!m_oob)
      return;

    auto r = m_runner.lock();
    if (r)
      try { m_oob(r, evt, e); } catch (...) { /* noop */ }
  }

 private:
  std::shared_ptr<connected_writable> m_impl;
  std::weak_ptr<RunnerT> m_runner;
  rd_handler             m_rhandler;
  wr_handler             m_whandler;
  event_handler          m_oob;
};

} } } } } // namespace

#endif

