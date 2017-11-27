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
#include "cool/ng/exception.h"

#include "event_sources.h"

namespace cool { namespace ng { namespace async {

// ==========================================================================
// ======
// ======
// ====== Network event sources
// ======
// ======
// ==========================================================================
namespace net { namespace impl {

namespace exc = cool::ng::exception;
namespace ip = cool::ng::net::ip;
namespace ipv4 = cool::ng::net::ipv4;
namespace ipv6 = cool::ng::net::ipv6;

// --------------------------------------------------------------------------
// -----
// ----- Factory methods
// ------

cool::ng::async::detail::startable* create_server(
    const std::shared_ptr<runner>& r_
  , const ip::address& addr_
  , int port_
  , const cb::server::weak_ptr& cb_)
{
  return new server(r_->impl(), addr_, port_, cb_);
}

std::shared_ptr<async::detail::connected_writable> create_stream(
    const std::shared_ptr<runner>& r_
  , const cool::ng::net::ip::address& addr_
  , int port_
  , const cb::stream::weak_ptr& cb_
  , void* buf_
  , std::size_t bufsz_)
{
  auto ret = cool::ng::util::shared_new<stream>(r_->impl(), cb_);
  ret->initialize(addr_, port_, buf_, bufsz_);
  return ret;
}

std::shared_ptr<async::detail::connected_writable> create_stream(
    const std::shared_ptr<runner>& r_
  , cool::ng::net::handle h_
  , const cb::stream::weak_ptr& cb_
  , void* buf_
  , std::size_t bufsz_)
{
  auto ret = cool::ng::util::shared_new<stream>(r_->impl(), cb_);
  ret->initialize(h_, buf_, bufsz_);
  return ret;
}
std::shared_ptr<async::detail::connected_writable> create_stream(
    const std::shared_ptr<runner>& r_
  , const cb::stream::weak_ptr& cb_
  , void* buf_
  , std::size_t bufsz_)
{
  auto ret = cool::ng::util::shared_new<stream>(r_->impl(), cb_);
  ret->initialize(buf_, bufsz_);
  return ret;
}

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

server::server(const std::shared_ptr<async::impl::executor>& ex_
             , const cool::ng::net::ip::address& addr_
             , int port_
             , const cb::server::weak_ptr& cb_)
  : named("cool.ng.async.net.server"), m_handler(cb_)
{
  switch (addr_.version())
  {
    case ip::version::ipv6:
      init_ipv6(addr_, port_);
      break;

    case ip::version::ipv4:
      init_ipv4(addr_, port_);
      break;
  }

  m_source = ::dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, m_handle, 0 , ex_->queue());
  m_source.cancel_handler(on_cancel);
  m_source.event_handler(on_event);
  m_source.context(this);
}

void server::init_ipv6(const cool::ng::net::ip::address& addr_, int port_)
{
  m_handle = ::socket(AF_INET6, SOCK_STREAM, 0);
  if (m_handle == ::cool::ng::net::invalid_handle)
    throw exc::operation_failed("failed to allocate listen socket");

  {
    const int enable = 1;
    if (::setsockopt(m_handle, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enable), sizeof(enable)) != 0)
      throw exc::operation_failed("failed to setsockopt");
  }
  {
    sockaddr_in6 addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = static_cast<in6_addr>(addr_);
    addr.sin6_port = htons(port_);

    if (::bind(m_handle, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
      throw exc::operation_failed("bind call failed");

    if (::listen(m_handle, 10) != 0)
      throw exc::operation_failed("listen call failed");
  }
}

void server::init_ipv4(const cool::ng::net::ip::address& addr_, int port_)
{
  m_handle = ::socket(AF_INET, SOCK_STREAM, 0);
  if (m_handle == ::cool::ng::net::invalid_handle)
    throw exc::operation_failed("failed to allocate listen socket");

  {
    const int enable = 1;
    if (::setsockopt(m_handle, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enable), sizeof(enable)) != 0)
      throw exc::operation_failed("failed to setsockopt");
  }
  {
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = static_cast<in_addr>(addr_);
    addr.sin_port = htons(port_);

    if (::bind(m_handle, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
      throw exc::operation_failed("bind call failed");

    if (::listen(m_handle, 10) != 0)
      throw exc::operation_failed("listen call failed");
  }
}

void server::start()
{
  m_source.resume();
}

void server::stop()
{
  m_source.suspend();
}

void server::on_cancel(void* ctx)
{
  auto self = static_cast<server*>(ctx);
  self->m_source.release();
  ::close(self->m_handle);

  delete self;
}

void server::on_event(void* ctx)
{
  auto self = static_cast<server*>(ctx);
  auto cb = self->m_handler.lock();
  std::size_t size = self->m_source.get_data();

  for (std::size_t i = 0; i < size; ++i)
  {
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    cool::ng::net::handle clt = accept(self->m_handle, reinterpret_cast<sockaddr*>(&addr), &len);

    if (!cb)  // close immediatelly if callback no longer exists - but do accept to avoid repeated calls
    {
      ::close(clt);
      continue;
    }

    try
    {
      bool res = false;
      if (addr.ss_family == AF_INET)
      {
        ipv4::host ca(reinterpret_cast<sockaddr_in*>(&addr)->sin_addr);
        res = cb->on_connect(clt, ca, ntohs(reinterpret_cast<sockaddr_in*>(&addr)->sin_port));
      }
      else if (addr.ss_family == AF_INET6)
      {
        ipv6::host ca(reinterpret_cast<sockaddr_in6*>(&addr)->sin6_addr);
        res = cb->on_connect(clt, ca, ntohs(reinterpret_cast<sockaddr_in6*>(&addr)->sin6_port));
      }

      // ELSE case is covered by res being false, which will close the accepted socket
      if (!res)
        ::close(clt);
    }
    catch (...)
    {
      ::close(clt);   // if user handlers throw an uncontained exception
    }
  }
}

void server::shutdown()
{
  start();
  m_source.cancel();
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
    : named("cool.ng.async.net.stream")
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

  m_state = state::starting;
  connect(addr_, port_);
}

void stream::initialize(cool::ng::net::handle h_, void* buf_, std::size_t bufsz_)
{

  m_state = state::connected;

#if defined(OSX_TARGET)
  // on OSX accepted socket does not preserve non-blocking properties of listen socket
  int option = 1;
  if (ioctl(h_, FIONBIO, &option) != 0)
    throw exc::operation_failed("ioctl call failed");
#endif

  auto rh = ::dup(h_);
  if (rh == cool::ng::net::invalid_handle)
    throw exc::operation_failed("failed to dup socket");

  create_write_source(h_, false);
  create_read_source(rh, buf_, bufsz_);
}

void stream::initialize(void* buf_, std::size_t bufsz_)
{
  m_state = state::disconnected;
}

void stream::create_write_source(cool::ng::net::handle h_, bool  start_)
{
  auto ex_ = m_executor.lock();
  if (!ex_)
    throw exc::runner_not_available();
  if (h_ == cool::ng::net::invalid_handle)
    throw exc::illegal_argument("invalid file descriptor");

  m_writer = new context;
  m_writer->m_handle = h_;
  m_writer->m_stream = self().lock();

  // prepare write event source
  m_writer->m_source = ::dispatch_source_create(DISPATCH_SOURCE_TYPE_WRITE, m_writer->m_handle, 0 , ex_->queue());
  m_writer->m_source.cancel_handler(on_wr_cancel);
  m_writer->m_source.event_handler(on_wr_event);
  m_writer->m_source.context(m_writer);

  // if stream is not yet connected start the source to cover connect event
  if (start_)
    m_writer->m_source.resume();
}

void stream::create_read_source(cool::ng::net::handle h_, void* buf_, std::size_t bufsz_)
{
  auto ex_ = m_executor.lock();
  if (!ex_)
    throw exc::runner_not_available();
  if (h_ == cool::ng::net::invalid_handle)
    throw exc::illegal_argument("invalid file descriptor");

  m_reader = new rd_context;

  // prepare read buffer
  m_reader->m_rd_data = buf_;
  m_reader->m_rd_size = bufsz_;
  m_reader->m_rd_is_mine = false;
  if (buf_ == nullptr)
  {
    m_reader->m_rd_data = new uint8_t[bufsz_];  // TODO: check for null
    m_reader->m_rd_is_mine = true;
  }

  m_reader->m_handle = h_;
  m_reader->m_stream = self().lock();

  // prepare read event source
  m_reader->m_source = ::dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, m_reader->m_handle, 0 , ex_->queue());
  m_reader->m_source.cancel_handler(on_rd_cancel);
  m_reader->m_source.event_handler(on_rd_event);
  m_reader->m_source.context(m_reader);

  m_reader->m_source.resume();
}

void stream::cancel_write_source()
{
  if (m_writer == nullptr)
    return;

  m_writer->m_source.resume();
  m_writer->m_source.cancel();
}

void stream::cancel_read_source()
{
  if (m_reader == nullptr)
    return;

  m_reader->m_source.resume();
  m_reader->m_source.cancel();
}

void stream::disconnect()
{
  cancel_read_source();
  cancel_write_source();
  m_state = state::disconnected;
}

void stream::connect(const cool::ng::net::ip::address& addr_, uint16_t port_)
{
  cool::ng::net::handle handle = cool::ng::net::invalid_handle;

  if (m_state == state::connected)
    throw exc::invalid_state();
    
  try
  {
#if defined(LINUX_TARGET)
    handle = addr_.version() == ip::version::ipv6 ?
        ::socket(AF_INET6, SOCK_STREAM, 0) : ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
#else
    handle = addr_.version() == ip::version::ipv6 ?
        ::socket(AF_INET6, SOCK_STREAM, 0) : ::socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (handle == cool::ng::net::invalid_handle)
      throw exc::operation_failed("failed to allocate socket");

#if !defined(LINUX_TARGET)
    int option = 1;
    if (ioctl(handle, FIONBIO, &option) != 0)
      throw exc::operation_failed("ioctl call failed");
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
        throw exc::operation_failed("connect failed");
    }
  }
  catch (...)
  {
    if (m_writer != nullptr)
    {
      m_writer->m_source.cancel();
    }
    else
    {
      if (handle != cool::ng::net::invalid_handle)
        ::close(handle);
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
      aux->on_read(buf, size);
      if (buf != self->m_rd_data)
      {
        if (self->m_rd_is_mine)
        {
          delete [] static_cast<uint8_t*>(self->m_rd_data);
          self->m_rd_is_mine = false;
        }
        self->m_rd_data = buf;
        self->m_rd_size = size;
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
    case state::starting:
    case state::connecting:
      self->m_stream->process_connect_event(size);
      break;

    case state::connected:
      self->m_stream->process_write_event(size);
      break;

    case state::disconnected:
      break;
  }
}

void stream::write(const void* data, std::size_t size)
{
  if (m_state != state::connected)
    throw exc::illegal_state("not connected");

  bool expected = false;
  if (!m_wr_busy.compare_exchange_strong(expected, true))
    throw exc::illegal_state("writer busy");

  m_wr_data = static_cast<const uint8_t*>(data);
  m_wr_size = size;
  m_wr_pos = 0;
  m_writer->m_source.resume();
}

void stream::process_write_event(std::size_t size)
{
  std::size_t res = ::write(m_writer->m_handle, m_wr_data + m_wr_pos, m_wr_size - m_wr_pos);
  m_wr_pos += res;

  if (m_wr_pos >= m_wr_size)
  {
    m_writer->m_source.suspend();
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
void stream::process_connect_event(std::size_t size)
{
  m_writer->m_source.suspend();

#if defined(LINUX_TARGET)
  if (size != 0)
#else
  if (size <= 2048)
#endif
  {
    m_state = state::disconnected;
    auto aux = m_handler.lock();
    if (aux)
      try { aux->on_event(cb::stream::event::connect_failed); } catch (...) { }
    return;
  }

  // connect succeeded - create reader context and start reader
  // !! must dup because Linux wouldn't have read/write on same fd
  create_read_source(::dup(m_writer->m_handle), m_buf, m_size);
  //TODO: check for throw if dup failed
  m_state = state::connected;

  auto aux = m_handler.lock();
  if (aux)
    try { aux->on_event(cb::stream::event::connected); } catch (...) { }
}

void stream::process_disconnect_event()
{
  m_state = state::disconnected;
  // TODO: what to do with ongoing write?
  cancel_write_source();
  cancel_read_source();

  auto aux = m_handler.lock();
  if (aux)
    try { aux->on_event(cb::stream::event::disconnected); } catch (...) { }
}

void stream::start()
{
  m_reader->m_source.resume();
}

void stream::stop()
{
  m_reader->m_source.suspend();
}

void stream::shutdown()
{
  cancel_read_source();
  cancel_write_source();
}

} } } } }


