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

#if !defined(cool_ng_f36abcb0_cce1_42a3_b25a_9beef951523a)
#define      cool_ng_f36abcb0_cce1_42a3_b25a_9beef951523a

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

#include "cool/ng/async/net/stream.h"

namespace cool { namespace ng { namespace async { namespace net {

namespace detail {

// --- template wrapper around platform dependent server implementation -
//     template parameter preserves actual runner type that is passed to
//     the user callback
template <typename RunnerT>
class server : public async::detail::itf::startable
             , public impl::cb::server
             , public cool::ng::util::self_aware<server<RunnerT>>
{
 public:
  using stream_factory  = typename detail::types<RunnerT>::stream_factory;
  using connect_handler = typename detail::types<RunnerT>::connect_handler;
  using error_handler   = typename detail::types<RunnerT>::error_handler;

  using ptr = std::shared_ptr<server>;

 public:
  server(const std::weak_ptr<RunnerT>& runner_
       , const stream_factory& sf_
       , const connect_handler& hc_
       , const error_handler& he_)
    : m_runner(runner_), m_factory(sf_), m_handler(hc_), m_err_handler(he_)
  { /* noop */ }

  void initialize(const cool::ng::net::ip::address& addr_, uint16_t port_)
  {
    auto r = m_runner.lock();
    if (r)
      m_impl = impl::create_server(r, addr_, port_, this->self());
    else
      throw cool::ng::exception::runner_not_available();
  }

  void initialize(cool::ng::net::handle h_)
  {
    auto r = m_runner.lock();
    if (r)
      m_impl = impl::create_server(r, h_, this->self());
    else
      throw cool::ng::exception::runner_not_available();

  }

  ~server()
  {
    if (m_impl)
      m_impl->shutdown();
  }

  //--- event_source interface
  void start() override                    { m_impl->start(); }
  void stop() override                     { m_impl->stop();  }
  void shutdown() override                 { m_impl->stop();  }
  const std::string& name() const override { return m_impl->name(); }

  //--- cb::server interface
  void on_connect(const cool::ng::async::net::stream& s_) override
  {
    auto r = m_runner.lock();
    if (r)
    {
      try
      {
        m_handler(r, s_);
      }
      catch (...)
      { /* noop */ }
    }
  }

  void on_event(const std::error_code& err) override
  {
    if (!m_err_handler)
      return;

    auto r = m_runner.lock();
    if (r)
      try { m_err_handler(r, err); } catch (...) { /* noop */ }
  }

  cool::ng::async::net::stream manufacture(const ip::address& addr_, uint16_t port_) override
  {
    auto r = m_runner.lock();
    if (!r)
      throw cool::ng::exception::runner_not_available();
    return m_factory(r, addr_, port_);
  }

 private:
  std::weak_ptr<RunnerT> m_runner;
  stream_factory         m_factory;
  connect_handler        m_handler;
  error_handler          m_err_handler;
  std::shared_ptr<async::detail::itf::startable> m_impl;
};


} } } } } // namespace

#endif

