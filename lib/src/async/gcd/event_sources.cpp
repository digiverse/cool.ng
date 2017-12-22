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

#include <unistd.h>
#include <fcntl.h>
#if defined(OSX_TARGET)
#include <sys/ioctl.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "cool/ng/error.h"
#include "cool/ng/exception.h"

#include "cool/ng/async/net/stream.h"
#include "event_sources.h"

namespace cool { namespace ng { namespace async {

using cool::ng::error::no_error;

using cool::ng::net::handle;
using cool::ng::net::invalid_handle;

namespace exc = cool::ng::exception;
namespace ip = cool::ng::net::ip;
namespace ipv4 = cool::ng::net::ipv4;
namespace ipv6 = cool::ng::net::ipv6;

namespace impl {

// ==========================================================================
// ======
// ======
// ====== Timer event source
// ======
// ======
// ==========================================================================

timer::context::context(const timer::ptr& t_, const std::shared_ptr<async::impl::executor>& ex_)
    : m_timer(t_)
{
  m_source = ::dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0 , ex_->queue());
  m_source.cancel_handler(on_cancel);
  m_source.event_handler(on_event);
  m_source.context(this);
}

timer::context::~context()
{ /* noop */ }

void timer::context::shutdown()
{
  m_source.resume();
  m_source.cancel();
}

void timer::context::on_cancel(void* ctx)
{
  auto self = static_cast<context*>(ctx);
  self->m_source.release();

  delete self;
}

void timer::context::on_event(void *ctx)
{
  auto self = static_cast<context*>(ctx);
  auto cb = self->m_timer->m_callback.lock();
  if (cb)
    cb->expired();
}

timer::timer(const std::weak_ptr<cb::timer>& t_
           , uint64_t p_
           , uint64_t l_)
  : named("si.digiverse.ng.cool.timer")
  , m_callback(t_)
  , m_context(nullptr)
  , m_period(p_ * 1000)
  , m_leeway(l_ * 1000)
{
}


timer::~timer()
{ }

void timer::initialize(const std::shared_ptr<async::impl::executor>& ex_)
{
  m_context = new context(self().lock(), ex_);
}

void timer::period(uint64_t p_, uint64_t l_)
{
  if (p_ == 0)
    throw exc::illegal_argument();
  m_period = p_ * 1000;
  m_leeway = l_ * 1000;

}

void timer::shutdown()
{
  m_context->shutdown();
}

void timer::start()
{
  if (m_context->m_source)
    m_context->m_source.suspend();

  ::dispatch_source_set_timer(m_context->m_source.source(), ::dispatch_time(DISPATCH_TIME_NOW, m_period), m_period, m_leeway);
  m_context->m_source.resume();
}

void timer::stop()
{
  m_context->m_source.suspend();
}

} // namespace impl




// ==========================================================================
// ======
// ======
// ====== Network event sources
// ======
// ======
// ==========================================================================
namespace net { namespace impl {

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// -----
// ----- server class implementation
// -----
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
server::context::context(const server::ptr& s_
                       , const std::shared_ptr<async::impl::executor>& ex_
                       , const ip::address& addr_
                       , uint16_t port_)
  : m_server(s_), m_handle(invalid_handle)
{
  try
  {
    m_handle = ::socket(addr_.version() == ip::version::ipv4 ? AF_INET : AF_INET6, SOCK_STREAM, 0);
    if (m_handle == ::cool::ng::net::invalid_handle)
      throw exc::socket_failure();
    {
      const int enable = 1;
      if (::setsockopt(m_handle, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enable), sizeof(enable)) != 0)
        throw exc::socket_failure();
    }
    {
      struct sockaddr* addr;
      std::size_t sz;
      sockaddr_in  addr4;
      sockaddr_in6 addr6;

      if (addr_.version() == ip::version::ipv4)
      {
        sz = sizeof(addr4);
        addr = reinterpret_cast<struct sockaddr*>(&addr4);
        std::memset(&addr4, 0, sizeof(addr4));
        addr4.sin_family = AF_INET;
        addr4.sin_addr = static_cast<in_addr>(addr_);
        addr4.sin_port = ntohs(port_);
      }
      else
      {
        sz = sizeof(addr6);
        addr = reinterpret_cast<struct sockaddr*>(&addr6);
        std::memset(&addr6, 0, sizeof(addr6));
        addr6.sin6_family = AF_INET6;
        addr6.sin6_addr = static_cast<in6_addr>(addr_);
        addr6.sin6_port = ntohs(port_);
      }

      if (::bind(m_handle, addr, sz) != 0)
        throw exc::socket_failure();
    }

    if (::listen(m_handle, 10) != 0)
      throw exc::socket_failure();

    m_source = ::dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, m_handle, 0 , ex_->queue());
    m_source.cancel_handler(on_cancel);
    m_source.event_handler(on_event);
    m_source.context(this);
  }
  catch (...)
  {
    m_source.destroy();
    if (m_handle != invalid_handle)
      ::close(m_handle);
    throw;
  }
}

void server::context::start_accept()
{
  m_source.resume();
}

void server::context::stop_accept()
{
  m_source.suspend();
}

void server::context::shutdown()
{
  start_accept();
  m_source.cancel();
}

void server::context::on_cancel(void* ctx)
{
  auto self = static_cast<context*>(ctx);
  self->m_source.release();

  ::close(self->m_handle);

  delete self;
}

void server::context::on_event(void* ctx)
{
  auto self = static_cast<context*>(ctx);
  std::size_t size = self->m_source.get_data();

  for (std::size_t i = 0; i < size; ++i)
  {
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    handle clt = accept(self->m_handle, reinterpret_cast<sockaddr*>(&addr), &len);

    if (clt != invalid_handle)
    {
      ip::host_container address(addr);
      uint16_t port = (static_cast<const ip::address&>(address).version() == ip::version::ipv4)
         ? ntohs(reinterpret_cast<sockaddr_in*>(&addr)->sin_port)
         : ntohs(reinterpret_cast<sockaddr_in6*>(&addr)->sin6_port);

      self->m_server->process_accept(clt, address, port);
    }
    else
    {
      // TODO: error logic
    }
  }
}

// ---------------------------
// server class implementation

server::server(const std::shared_ptr<async::impl::executor>& ex_
             , const cb::server::weak_ptr& cb_)
  : named("si.digiverse.ng.cool.server")
  , m_state(state::stopped)
  , m_context(nullptr)
  , m_handler(cb_)
  , m_exec(ex_)
{ /* noop */ }

server::~server()
{ /* noop */ }

void server::initialize(const cool::ng::net::ip::address& addr_, uint16_t port_)
{
  auto e = m_exec.lock();
  if (!e)
    throw exc::runner_not_available();

  m_context = new context(self().lock(), e, addr_, port_);
}


void server::start()
{
  state expect = state::stopped;

  if (m_state.compare_exchange_strong(expect, state::starting))
  {
    m_context->start_accept();
    expect = state::starting;
    m_state.compare_exchange_strong(expect, state::accepting); // TODO: any action if it fails
    return;
  }

  if (expect == state::accepting)  // was already accepting
    return;

  throw exc::invalid_state();
}

void server::stop()
{
  state expect = state::accepting;

  if (m_state.compare_exchange_strong(expect, state::stopping))
  {
    m_context->stop_accept();
    expect = state::stopping;
    m_state.compare_exchange_strong(expect, state::stopped); // TODO: any action if it fails
    return;
  }

  if (expect == state::stopped)  // was already accepting
    return;

  throw exc::invalid_state();
}

void server::shutdown()
{
  m_context->shutdown();
}

void server::process_accept(cool::ng::net::handle h_
                          , const cool::ng::net::ip::address& addr_
                          , uint16_t port_)
{
  auto cb = m_handler.lock();

  if (!cb || m_state != state::accepting)
  {
    // user handler no longer exists, close connection and be done
    ::close(h_);
    return;
  }

  try
  {
    auto stream = cb->manufacture(addr_, port_);
    stream.m_impl->set_handle(h_);
    try { cb->on_connect(stream); } catch (...) { /* noop */ }
  }
  catch (...)
  {
    ::close(h_);
  }

}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// -----
// ----- stream class  implementation
// -----
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

stream::stream(const std::weak_ptr<async::impl::executor>& ex_
             , const cb::stream::weak_ptr& cb_)
    : named("si.digiverse.ng.cool.stream")
    , m_state(state::disconnected)
    , m_executor(ex_)
    , m_handler(cb_)
    , m_reader(nullptr)
    , m_writer(nullptr)
    , m_wr_busy(false)
{ /* noop */ }

stream::~stream()
{ /* noop */ }

void stream::initialize(const cool::ng::net::ip::address& addr_
                      , uint16_t port_
                      , void* buf_
                      , std::size_t bufsz_)
{
  m_size = bufsz_;
  m_buf = buf_;

  connect(addr_, port_);
}

void stream::set_handle(cool::ng::net::handle h_)
{

  m_state = state::connected;

#if defined(OSX_TARGET)
  // on OSX accepted socket does not preserve non-blocking properties of the listen socket
  int option = 1;
  if (ioctl(h_, FIONBIO, &option) != 0)
    throw exc::socket_failure();
#endif

  auto rh = ::dup(h_);
  if (rh == cool::ng::net::invalid_handle)
    throw exc::socket_failure();

  create_write_source(h_, false);
  create_read_source(rh, m_buf, m_size);
}

void stream::initialize(void* buf_, std::size_t bufsz_)
{
  m_size = bufsz_;
  m_buf = buf_;
}

void stream::create_write_source(cool::ng::net::handle h_, bool  start_)
{
  auto ex_ = m_executor.lock();
  if (!ex_)
    throw exc::runner_not_available();

  auto writer = new context;
  writer->m_handle = h_;
  writer->m_stream = self().lock();

  // prepare write event source
  writer->m_source = ::dispatch_source_create(DISPATCH_SOURCE_TYPE_WRITE, writer->m_handle, 0 , ex_->queue());
  writer->m_source.cancel_handler(on_wr_cancel);
  writer->m_source.event_handler(on_wr_event);
  writer->m_source.context(writer);

  m_writer.store(writer);
  // if stream is not yet connected start the source to cover connect event
  if (start_)
    writer->m_source.resume();
}

void stream::create_read_source(cool::ng::net::handle h_, void* buf_, std::size_t bufsz_)
{
  auto ex_ = m_executor.lock();
  if (!ex_)
    throw exc::runner_not_available();

  auto reader = new rd_context;

  // prepare read buffer
  reader->m_rd_data = buf_;
  reader->m_rd_size = bufsz_;
  reader->m_rd_is_mine = false;
  if (buf_ == nullptr)
  {
    reader->m_rd_data = new uint8_t[bufsz_];
    reader->m_rd_is_mine = true;
  }

  reader->m_handle = h_;
  reader->m_stream = self().lock();

  // prepare read event source
  reader->m_source = ::dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, reader->m_handle, 0 , ex_->queue());
  reader->m_source.cancel_handler(on_rd_cancel);
  reader->m_source.event_handler(on_rd_event);
  reader->m_source.context(reader);

  m_reader.store(reader);
  reader->m_source.resume();
}


bool stream::cancel_write_source(stream::context*& writer)
{
  writer = m_writer.load();

  if (!m_writer.compare_exchange_strong(writer, nullptr))
    return false;  // somebody else is already meddling with this
  if (writer == nullptr)
    return true;

  writer->m_source.resume();
  writer->m_source.cancel();
  return true;
}

bool stream::cancel_read_source(stream::rd_context*& reader)
{
  reader = m_reader.load();
  if (!m_reader.compare_exchange_strong(reader, nullptr))
    return false;
  if (reader == nullptr)
    return true;

  reader->m_source.resume();
  reader->m_source.cancel();
  return true;
}

void stream::disconnect()
{
  state expect = state::connected;
  if (!m_state.compare_exchange_strong(expect, state::disconnecting))
    throw exc::invalid_state();

  {
    rd_context* aux;
    if (!cancel_read_source(aux))
      throw exc::operation_failed(cool::ng::error::errc::concurrency_problem);
  }
  {
    context* aux;
    if (!cancel_write_source(aux))
      throw exc::operation_failed(cool::ng::error::errc::concurrency_problem);
  }
  expect = state::disconnecting;
  if (!m_state.compare_exchange_strong(expect, state::disconnected))
    throw exc::operation_failed(cool::ng::error::errc::concurrency_problem);
}

void stream::connect(const cool::ng::net::ip::address& addr_, uint16_t port_)
{
  if (m_size == 0)
    throw exc::illegal_argument();


  cool::ng::net::handle handle = cool::ng::net::invalid_handle;

  if (m_state != state::disconnected)
    throw exc::invalid_state();
    
  try
  {
#if defined(LINUX_TARGET)
    handle = addr_.version() == ip::version::ipv6 ?
          ::socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0)
        : ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
#else
    handle = addr_.version() == ip::version::ipv6 ?
        ::socket(AF_INET6, SOCK_STREAM, 0)
      : ::socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (handle == cool::ng::net::invalid_handle)
      throw exc::socket_failure();

#if !defined(LINUX_TARGET)
    int option = 1;
    if (ioctl(handle, FIONBIO, &option) != 0)
      throw exc::socket_failure();
#endif

    create_write_source(handle);

    sockaddr* p;
    std::size_t size;
    sockaddr_in addr4;
    sockaddr_in6 addr6;
    if (addr_.version() == ip::version::ipv4)
    {
      addr4.sin_family = AF_INET;
      addr4.sin_addr = static_cast<in_addr>(addr_);
      addr4.sin_port = htons(port_);
      p = reinterpret_cast<sockaddr*>(&addr4);
      size = sizeof(addr4);
    }
    else
    {
      addr6.sin6_family = AF_INET6;
      addr6.sin6_addr = static_cast<in6_addr>(addr_);
      addr6.sin6_port = htons(port_);
      p = reinterpret_cast<sockaddr*>(&addr6);
      size = sizeof(addr6);
    }

    // Linux may sometimes do immediate connect with connect returning 0.
    // Nevertheless, we will consider this as async connect and let the
    // on_write event handler handle this in an usual way.
    m_state = state::connecting;
    if (::connect(handle, p, size) == -1)
    {
      if (errno != EINPROGRESS)
        throw exc::socket_failure();
    }
  }
  catch (...)
  {
    context* prev;
    if (cancel_write_source(prev))
    {
      if (prev == nullptr)
      {
        if (handle != cool::ng::net::invalid_handle)
          ::close(handle);
      }
    }

    m_state = state::disconnected;

    throw;
  }
}

void stream::on_wr_cancel(void* ctx)
{
  auto self = static_cast<context*>(ctx);
  self->m_source.release();
  ::close(self->m_handle);

  self->m_stream->m_writer = nullptr;
  delete self;
}

void stream::on_rd_cancel(void* ctx)
{
  auto self = static_cast<rd_context*>(ctx);
  self->m_source.release();

  ::close(self->m_handle);

  if (self->m_rd_is_mine)
    delete [] static_cast<uint8_t*>(self->m_rd_data);
  self->m_stream->m_reader = nullptr;

  delete self;
}

void stream::on_rd_event(void* ctx)
{
  auto self = static_cast<rd_context*>(ctx);
  std::size_t size = self->m_source.get_data();

  if (size == 0)   // indicates disconnect of peer
  {
    self->m_stream->process_disconnect_event();
    return;
  }

  size = ::read(self->m_handle, self->m_rd_data, self->m_rd_size);
  auto buf = self->m_rd_data;
  try
  {
    auto aux = self->m_stream->m_handler.lock();
    if (aux)
    {
      try { aux->on_read(buf, size); } catch (...) { /* noop */ }

      // check if callback modified buffer or size parameters
      if (buf != self->m_rd_data || size != self->m_rd_size)
      {
        // release current buffer if allocated by me
        if (self->m_rd_is_mine)
          delete [] static_cast<uint8_t*>(self->m_rd_data);

        // there are some special values that requeire different considerations
        //  - if size is zero revert to bufffer specified at the creation
        //  - if buf is nullptr allocate buffer of specified size
        if (size == 0)
        {
          self->m_rd_size = self->m_stream->m_size;
          self->m_rd_data = self->m_stream->m_buf;
        }
        else
        {
          self->m_rd_data = buf;
          self->m_rd_size = size;
        }
        self->m_rd_is_mine = self->m_rd_data == nullptr;

        if (self->m_rd_is_mine)
          self->m_rd_data = new uint8_t[self->m_rd_size];
      }
    }
  }
  catch(...)
  { /* noop */ }
}

void stream::on_wr_event(void* ctx)
{
  auto self = static_cast<context*>(ctx);
  auto size = self->m_source.get_data();

  switch (static_cast<state>(self->m_stream->m_state))
  {
    case state::connecting:
      self->m_stream->process_connect_event(self, size);
      break;

    case state::connected:
      self->m_stream->process_write_event(self, size);
      break;

    case state::disconnected:
    case state::disconnecting:
      break;
  }
}

void stream::write(const void* data, std::size_t size)
{
  if (m_state != state::connected)
    throw exc::invalid_state();

  bool expected = false;
  if (!m_wr_busy.compare_exchange_strong(expected, true))
    throw exc::operation_failed(cool::ng::error::errc::resource_busy);

  m_wr_data = static_cast<const uint8_t*>(data);
  m_wr_size = size;
  m_wr_pos = 0;
  m_writer.load()->m_source.resume();
}

void stream::process_write_event(context* ctx, std::size_t size)
{
  std::size_t res = ::write(ctx->m_handle, m_wr_data + m_wr_pos, m_wr_size - m_wr_pos);
  m_wr_pos += res;

  if (m_wr_pos >= m_wr_size)
  {
    ctx->m_source.suspend();
    m_wr_busy = false;
    auto aux = m_handler.lock();
    if (aux)
    {
      try { aux->on_write(m_wr_data, m_wr_size); } catch (...) { }
    }
  }
}

// - Despite both OSX and Linux supporting O_NDELAY flag to fcntl call, this
// - flag does not result in non-blocking connect. For non-blocking connect,
// - Linux requires socket to be created witn SOCK_NONBLOCK type flag and OX
// - requires FIONBIO ioctl flag to be set to 1 on the socket.
// -
// - The behavior of read an write event sources in combination with non-blocking
// - connect seems to differ between linux and OSX versions of libdispatch. The
// - following is the summary:
// -
// -                   +---------------+---------------+---------------+---------------+
// -                   |             OS X              |         Ubuntu 16.04          |
// -  +----------------+---------------+---------------+---------------+---------------+
// -  | status         | read    size  | write   size  | read    size  | write   size  |
// -  +----------------+------+--------+------+--------+------+--------+------+--------+
// -  | connected      |  --  |        |  ++  | 131228 |  --  |        |  ++  |      0 |
// -  +----------------+------+--------+------+--------+------+--------+------+--------+
// -  | timeout        | ++(2)|      0 | ++(1)|   2048 | ++(1)|      1 | ++(2)|      1 |
// -  +----------------+------+--------+------+--------+------+--------+------+--------+
// -  | reject         | ++(2)|      0 | ++(1)|   2048 | ++(1)|      1 | ++(2)|      1 |
// -  +----------------+------+--------+------+--------+------+--------+------+--------+
// -
// - Notes:
// -  o callback order on linux depends on event source creation order. The last
// -    created event source is called first
// -  o the implementation will only use write event source and will use size
// -    data to determine the outcome of connect
void stream::process_connect_event(context* ctx, std::size_t size)
{
  try
  {
    ctx->m_source.suspend();

  #if defined(LINUX_TARGET)
    if (size != 0)
  #else
    if (size <= 2048)
  #endif
    {
      throw exc::connection_failure();
    }

    // connect succeeded - create reader context and start reader
    // !! must dup because Linux wouldn't have read/write on same fd
    auto aux_h = ::dup(ctx->m_handle);
    if (aux_h == cool::ng::net::invalid_handle)
      throw exc::socket_failure();

    create_read_source(aux_h, m_buf, m_size);
    m_state = state::connected;

    auto aux = m_handler.lock();
    if (aux)
      try { aux->on_event(detail::oob_event::connect, no_error()); } catch (...) { }
  }
  catch (const cool::ng::exception::base& e)
  {
    {
      context* aux;
      cancel_write_source(aux);
    }
    m_state = state::disconnected;

    auto aux = m_handler.lock();
    if (aux)
      try { aux->on_event(detail::oob_event::failure, e.code()); } catch (...) { }
  }
}

void stream::process_disconnect_event()
{
  state expect = state::connected;
  if (!m_state.compare_exchange_strong(expect, state::disconnected))
    return; // TODO: should we assert here?

  {
    context* aux;
    cancel_write_source(aux);
  }
  {
    rd_context*  aux;
    cancel_read_source(aux);
  }

  auto aux = m_handler.lock();
  if (aux)
    try { aux->on_event(detail::oob_event::disconnect, no_error()); } catch (...) { }
}

void stream::shutdown()
{
  {
    rd_context* aux;
    cancel_read_source(aux);
  }
  {
    context* aux;
    cancel_write_source(aux);
  }
}

} } } } }


