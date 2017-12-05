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

#include <memory>
#include <functional>

#if !defined(WINDOWS_TARGET)
#include <dispatch/dispatch.h>
#endif

#include "cool/ng/ip_address.h"
#include "cool/ng/bases.h"
#include "cool/ng/impl/platform.h"
#include "cool/ng/async/runner.h"

namespace cool { namespace ng { namespace async {

namespace detail {

class startable {

 public:
  virtual ~startable() { /* noop */ }

  virtual const std::string& name() const = 0;
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void shutdown() = 0;
};

class event_source : public startable
{
 public:

};

class writable : public event_source
{
 public:
  virtual void write(const void* data, std::size_t sz) = 0;
};

class connected_writable : public writable
{
 public:
  virtual void connect(const cool::ng::net::ip::address&, uint16_t) = 0;
  virtual void disconnect() = 0;

};
} // namespace detail

namespace net {

namespace cb {  // callback interfaces

class server
{
 public:
  using weak_ptr = std::weak_ptr<server>;

 public:
  virtual ~server() { /* noop */ }
  virtual bool on_connect(const cool::ng::net::handle, const cool::ng::net::ip::address&, int) = 0;
};

class stream
{
 public:
  using weak_ptr = std::weak_ptr<stream>;
  enum class event { connected, failure_detected, disconnected };

 public:
  virtual ~stream() { /* noop */ }
  virtual void on_read(void*&, std::size_t&) = 0;
  virtual void on_write(const void*, std::size_t) = 0;
  virtual void on_event(event, const std::error_code&) = 0;
};

} // namespace cb

namespace impl {

dlldecl detail::startable* create_server(
    const std::shared_ptr<runner>& r_
  , const cool::ng::net::ip::address& addr_
  , int port_
  , const cb::server::weak_ptr& cb_);

dlldecl std::shared_ptr<async::detail::connected_writable> create_stream(
    const std::shared_ptr<runner>& runner_
  , const cool::ng::net::ip::address& addr_
  , int port_
  , const cb::stream::weak_ptr& cb_
  , void* buf_
  , std::size_t bufsz_);
dlldecl std::shared_ptr<async::detail::connected_writable> create_stream(
    const std::shared_ptr<runner>& runner_
  , cool::ng::net::handle h_
  , const cb::stream::weak_ptr& cb_
  , void* buf_
  , std::size_t bufsz_);
dlldecl std::shared_ptr<async::detail::connected_writable> create_stream(
    const std::shared_ptr<runner>& runner_
  , const cb::stream::weak_ptr& cb_
  , void* buf_
  , std::size_t bufsz_);

} // namespaace impl

namespace detail {

// --- template wrapper around platform dependent server implementation -
//     template parameter preserves actual runner type that is passed to
//     the user callback
template <typename RunnerT>
class server : public async::detail::event_source
             , public cb::server
             , public cool::ng::util::self_aware<server<RunnerT>>
{
 public:
  using connect_handler = std::function<bool(const std::shared_ptr<RunnerT>&, const cool::ng::net::handle, const cool::ng::net::ip::address&, int)>;
  using ptr = std::shared_ptr<server>;

 public:
  server(const std::weak_ptr<RunnerT>& runner_, const connect_handler& h_)
      : m_runner(runner_), m_handler(h_)
  { /* noop */ }

  void initialize(const cool::ng::net::ip::address& addr_, int port_)
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
  ~server()                 { m_impl->shutdown(); }

  // event_source interface
  void start() override     { m_impl->start();    }
  void stop() override      { m_impl->stop();     }
  void shutdown() override  { m_impl->stop();     }
  const std::string& name() const override { return m_impl->name(); }

  // cb_server interface
  bool on_connect(const cool::ng::net::handle handle_, const cool::ng::net::ip::address& addr_, int port_) override
  {
    auto r = m_runner.lock();
    return m_handler(r, handle_, addr_, port_);
  }

 private:
  std::weak_ptr<RunnerT>    m_runner;
  connect_handler           m_handler;
  async::detail::startable* m_impl;
};

template <typename RunnerT>
class stream : public async::detail::connected_writable
             , public cb::stream
             , public cool::ng::util::self_aware<stream<RunnerT>>
{
 public:
  using wr_handler    = std::function<void(const std::shared_ptr<RunnerT>&, const void*, std::size_t)>;
  using rd_handler    = std::function<void(const std::shared_ptr<RunnerT>&, void*&, std::size_t&)>;
  using event_handler = std::function<void(const std::shared_ptr<RunnerT>&, cb::stream::event, const std::error_code&)>;

 public:
  stream(const std::weak_ptr<RunnerT>& runner_
       , const rd_handler& rh_
       , const wr_handler& wh_
       , const event_handler& eh_)
      : m_runner(runner_), m_rhandler(rh_), m_whandler(wh_), m_oob(eh_)
  { /* noop */ }
  void initialize(const cool::ng::net::ip::address& addr_, int port_, void* buf_, std::size_t bufsz_)
  {
    auto r = m_runner.lock();
    if (r)
    {
      m_impl = impl::create_stream(r, addr_, port_, this->self(), buf_, bufsz_);
    }
    else
      throw cool::ng::exception::runner_not_available();
  }
 
  void initialize(cool::ng::net::handle h_, void* buf_, std::size_t bufsz_)
  {
    auto r = m_runner.lock();
    if (r)
    {
      m_impl = impl::create_stream(r, h_, this->self(), buf_, bufsz_);
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

  // event_source interface
  void start() override     { m_impl->start();    }
  void stop() override      { m_impl->stop();     }
  void shutdown() override  { m_impl->stop();     }
  const std::string& name() const override { return m_impl->name(); }

  // connected_writable interface
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
  // cb_stream interface
  void on_read(void*& buf_, std::size_t& size_) override
  {
    if (m_rhandler)
      try { m_rhandler(m_runner.lock(), buf_, size_); } catch (...) { }
  }
  void on_write(const void* buf_, std::size_t size_) override
  {
    if (m_whandler)
      try { m_whandler(m_runner.lock(), buf_, size_); } catch (...) { }
  }
  void on_event(cb::stream::event evt, const std::error_code& e) override
  {
    if (m_oob)
      try { m_oob(m_runner.lock(), evt, e); } catch (...) { }
  }

 private:
  std::shared_ptr<async::detail::connected_writable> m_impl;
  std::weak_ptr<RunnerT>            m_runner;
  rd_handler                        m_rhandler;
  wr_handler                        m_whandler;
  event_handler                     m_oob;
};

} } } } } // namespace

#endif

