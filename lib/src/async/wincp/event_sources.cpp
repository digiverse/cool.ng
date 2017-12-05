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
#include <iostream>

#include "cool/ng/error.h"
#include "cool/ng/exception.h"

#include "event_sources.h"

#pragma comment(lib, "Ws2_32.lib")

namespace cool { namespace ng { namespace async {

using cool::ng::error::no_error;

// ==========================================================================
// ======
// ======
// ====== Network event sources
// ======
// ======
// ==========================================================================

namespace net { namespace impl {

using cool::ng::net::handle;
using cool::ng::net::invalid_handle;

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
  , handle h_
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
  : named("cool.ng.async.net.server")
  , m_executor(ex_)
  , m_pool(async::impl::poolmgr::get_poolmgr())
  , m_handle(invalid_handle)
  , m_active(false)
  , m_handler(cb_)
  , m_accept_ex(nullptr)
  , m_get_sock_addrs(nullptr)
  , m_tp_io(nullptr)
  , m_client_handle(invalid_handle)
{
  try
  {
    // ----
    // create and bind listen socket
    {
      m_sock_type = AF_INET6;
      if (addr_.version() == ip::version::ipv4)
        m_sock_type = AF_INET;

      m_handle = WSASocketW(m_sock_type, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
      if (m_handle == invalid_handle)
        throw exc::socket_failure();

      const int enable = 1;
      if (setsockopt(m_handle, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enable), sizeof(enable)) == SOCKET_ERROR)
        throw exc::socket_failure();

      if (addr_.version() == ip::version::ipv6)
      {
        const DWORD ipv6only = 0;
        if (setsockopt(m_handle, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&ipv6only), sizeof(ipv6only)) == SOCKET_ERROR)
          throw exc::socket_failure();
      }

      sockaddr_in addr4;
      sockaddr_in6 addr6;
      sockaddr* p;
      std::size_t size;

      if (addr_.version() == ip::version::ipv4)
      {
        std::memset(&addr4, 0, sizeof(addr4));
        addr4.sin_family = AF_INET;
        addr4.sin_addr = static_cast<in_addr>(addr_);
        addr4.sin_port = htons(port_);
        p = reinterpret_cast<sockaddr*>(&addr4);
        size = sizeof(addr4);
      }
      else
      {
        std::memset(&addr6, 0, sizeof(addr6));
        addr6.sin6_family = AF_INET6;
        addr6.sin6_addr = static_cast<in6_addr>(addr_);
        addr6.sin6_port = htons(port_);
        p = reinterpret_cast<sockaddr*>(&addr6);
        size = sizeof(addr6);
      }

      if (bind(m_handle, p, static_cast<int>(size)) == SOCKET_ERROR)
        throw exc::socket_failure();

      if (listen(m_handle, 10) == SOCKET_ERROR)
        throw exc::socket_failure();
    }

    // ----
    // For whatever reason beyond my comprehension AcceptEx and GetAcceptExSockAddrs
    // aren't directly reachable but their addresses must be fetched via ioctl!!??

    GUID guid = WSAID_ACCEPTEX;
    if (WSAIoctl(
          m_handle
        , SIO_GET_EXTENSION_FUNCTION_POINTER
        , &guid
        , sizeof(guid)
        , &m_accept_ex
        , sizeof(m_accept_ex)
        , &m_filler
        , nullptr
        , nullptr) == SOCKET_ERROR)
      throw exc::socket_failure();

    guid = WSAID_GETACCEPTEXSOCKADDRS;
    if (WSAIoctl(
          m_handle
        , SIO_GET_EXTENSION_FUNCTION_POINTER
        , &guid
        , sizeof(guid)
        , &m_get_sock_addrs
        , sizeof(m_get_sock_addrs)
        , &m_filler
        , nullptr
        , nullptr) == SOCKET_ERROR)
      throw exc::socket_failure();

    // ----
    // Create thread pool i/o completion object and associate it with
    // the listen socket
    m_tp_io = CreateThreadpoolIo(
        reinterpret_cast<HANDLE>(m_handle)
      , server::on_accept
      , this
      , m_pool->get_environ());
    if (m_tp_io == nullptr)
      throw exc::threadpool_failure();
  }
  catch (...)
  {
    if (m_handle != invalid_handle)
      closesocket(m_handle);
    if (m_tp_io != nullptr)
      CancelThreadpoolIo(m_tp_io);

    throw;
  }
}

void server::start_accept()
{
  m_client_handle = ::WSASocketW(m_sock_type, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
  if (m_client_handle == invalid_handle)
    throw exc::socket_failure();

  StartThreadpoolIo(m_tp_io);
  memset(&m_overlapped, 0, sizeof(m_overlapped));
  if (!m_accept_ex(
      m_handle
    , m_client_handle
    , m_buffer
    , 0
    , sizeof(m_buffer) / 2
    , sizeof(m_buffer) / 2
    , &m_filler
    , &m_overlapped))
  {
    auto hr = WSAGetLastError();
    if (hr != ERROR_IO_PENDING)
    {
      CancelThreadpoolIo(m_tp_io);
          // TODO: what to do here????
    }
  }
}

void server::start()
{
  if (m_active)
    return;
  m_active = true;
  start_accept();
}

void server::stop()
{
  if (!m_active)
    return;
  m_active = false;
  CancelThreadpoolIo(m_tp_io);
}

void server::shutdown()
{
  if (m_handle != invalid_handle)
    closesocket(m_handle);
  m_handle = invalid_handle;
  stop();
  if (m_tp_io != nullptr)
    CloseThreadpoolIo(m_tp_io);
  delete this;
}

// ---
// Execution context for a task submitterd to specified async::executor. This
// task will do actual callback into the user code
class exec_for_accept : public cool::ng::async::detail::event_context
{
 public:
  exec_for_accept(const cb::server::weak_ptr& cb_
                , handle h_
                , const ip::host_container& addr_
                , int port_)
      : m_addr(addr_)
      , m_port(port_)
      , m_handle(h_)
      , m_handler(cb_)
  { /* noop */ }

  void entry_point() override
  {
    auto self = static_cast<exec_for_accept*>(static_cast<void*>(this));
    bool res = false;
    auto cb = self->m_handler.lock();
    if (cb)
    {
      try { res = cb->on_connect(self->m_handle, self->m_addr, self->m_port); } catch (...) { /* noop */ }
    }
    if (!res)
    {
      closesocket(self->m_handle);
    }
  }

 private:
  ip::host_container    m_addr;
  int                   m_port;
  handle m_handle;
  cb::server::weak_ptr  m_handler;
};

// -- callback from the threadpool for AcceptEx call - this callback is a result
//    of StartThreadpoIo work object and was not submitted in the context of any
//    runner. To support synchronized semantics of event sources this callback
//    must submit work to the executor of the runner the event source is
//    associated with. To do so it must create a work context and submit it
//    to executor::run method.

void server::on_accept(
      PTP_CALLBACK_INSTANCE instance_
    , PVOID context_
    , PVOID overlapped_
    , ULONG io_result_
    , ULONG_PTR num_transferred_
    , PTP_IO io_)
{
  if (context_ != nullptr)
  {
    auto ctx = static_cast<server*>(context_);

    if (io_result_ == NO_ERROR)
      ctx->process_accept();

    // TODO: what on error?
  }
}

void server::process_accept()
{
  sockaddr_storage* local;
  sockaddr_storage* remote;
  int len_local, len_remote;

  m_get_sock_addrs(
      m_buffer
    , 0
    , sizeof(m_buffer) / 2
    , sizeof(m_buffer) / 2
    , reinterpret_cast<sockaddr**>(&local)
    , &len_local
    , reinterpret_cast<sockaddr**>(&remote)
    , &len_remote);

  auto r = m_executor.lock();
  if (r)
  {
    if (setsockopt(m_client_handle, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char *>(&m_handle), sizeof(m_handle)) != NO_ERROR)
    {
      // TODO: error handling
    }

    cool::ng::async::detail::event_context* ctx = nullptr;
    ip::host_container ca = *remote;
    int port = ntohs(remote->ss_family == AF_INET ? reinterpret_cast<sockaddr_in*>(remote)->sin_port : ntohs(reinterpret_cast<sockaddr_in6*>(remote)->sin6_port));

    ctx = new exec_for_accept(m_handler, m_client_handle, ca, port);

    if (ctx != nullptr)
      r->run(ctx);
  }
  else
  {
    closesocket(m_client_handle);
  }

  start_accept();
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

stream::context::context(const stream::ptr& s_, handle h_, void* buf_, std::size_t sz_)
  : m_stream(s_), m_handle(h_), m_rd_data(buf_), m_rd_size(sz_), m_rd_is_mine(buf_ == nullptr), m_tpio(nullptr)
{
  if (m_rd_is_mine)
    m_rd_data = new uint8_t[m_rd_size];

  m_tpio = CreateThreadpoolIo(
      reinterpret_cast<HANDLE>(m_handle)
    , stream::on_event
    , this
    , s_->m_pool->get_environ()
  );

  if (m_tpio == nullptr)
  {
    auto aux = exc::threadpool_failure();
    throw aux;
  }
}

stream::context::~context()
{
  if (m_handle != invalid_handle)
    closesocket(m_handle);

  if (m_tpio != nullptr)
    CloseThreadpoolIo(m_tpio);

  if (m_rd_is_mine && m_rd_data != nullptr)
    delete [] static_cast<uint8_t*>(m_rd_data);
}

stream::stream(const std::weak_ptr<async::impl::executor>& ex_
             , const cb::stream::weak_ptr& cb_)
    : named("cool.ng.async.net.stream")
    , m_state(state::disconnected)
    , m_executor(ex_)
    , m_pool(async::impl::poolmgr::get_poolmgr())
    , m_handler(cb_)
    , m_context(nullptr)
    , m_connect_ex(nullptr)
    , m_rd_size(32768)
{ /* noop */
  std::cout << "stream " << name() << " created.\n";
}

stream::~stream()
{ /* noop */
  std::cout << "stream " << name() << " destroyed.\n";
}

void stream::initialize(const cool::ng::net::ip::address& addr_
                      , uint16_t port_
                      , void* buf_
                      , std::size_t bufsz_)
{
  m_rd_size = bufsz_;
  m_rd_data = buf_;

  connect(addr_, port_);
}

void stream::initialize(handle h_, void* buf_, std::size_t bufsz_)
{
  m_rd_size = bufsz_;
  m_rd_data = buf_;

  try
  {
    m_context = new context(self().lock(), h_, m_rd_data, m_rd_size);

    m_state = state::connected;
    start_read_source();
  }
  catch (...)
  {
    // at this point no ovelapped operation is running hence
    // explicitly delete context

    if (m_context != nullptr)
      delete m_context;
    m_context = nullptr;
    throw;
  }
}

void stream::initialize(void* buf_, std::size_t bufsz_)
{
  m_rd_size = bufsz_;
  m_rd_data = buf_;
}

void stream::start_read_source()
{
  // start read operation
  memset(&m_context->m_rd_overlapped, 0, sizeof(m_context->m_rd_overlapped));
  StartThreadpoolIo(m_context->m_tpio);

  if (!ReadFile(reinterpret_cast<HANDLE>(m_context->m_handle)
              , m_context->m_rd_data
              , static_cast<DWORD>(m_context->m_rd_size)
              , &m_context->m_read_bytes
              , &m_context->m_rd_overlapped))
  {
    auto err = GetLastError();
    if (err != ERROR_IO_PENDING)
    {
      CancelThreadpoolIo(m_context->m_tpio);
      // TODO: disconnect, cleanup
    }
  }
  else
  {
      CancelThreadpoolIo(m_context->m_tpio);
    // TODO: what to do here?
  }
}

namespace {

void set_address(sockaddr_in& sa_, const ip::address& a_, int port_, sockaddr*& p_, int& size)
{
  memset(&sa_, 0, sizeof(sa_));
  sa_.sin_family = AF_INET;
  sa_.sin_addr = static_cast<in_addr>(a_);
  sa_.sin_port = htons(port_);
  p_ = reinterpret_cast<sockaddr*>(&sa_);
  size = sizeof(sa_);
}

void set_address(sockaddr_in6& sa_, const ip::address& a_, int port_, sockaddr*& p_, int& size)
{
  memset(&sa_, 0, sizeof(sa_));
  sa_.sin6_family = AF_INET6;
  sa_.sin6_addr = static_cast<in6_addr>(a_);
  sa_.sin6_port = htons(port_);
  p_ = reinterpret_cast<sockaddr*>(&sa_);
  size = sizeof(sa_);
}

} // anonymous namespace

void stream::connect(const cool::ng::net::ip::address& addr_, uint16_t port_)
{
  handle handle = invalid_handle;

  {
    state expect = state::disconnected;

    if (!m_state.compare_exchange_strong(expect, state::connecting))
    throw exc::invalid_state();
  }

  try
  {
    {
      auto type = AF_INET6;
      if (addr_.version() == ip::version::ipv4)
        type = AF_INET;

      auto handle = WSASocketW(type, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
      if (handle == invalid_handle)
        throw exc::socket_failure();

      m_context = new context(self().lock(), handle, m_rd_data, m_rd_size);
    }

    // TODO: does this apply to client sockets, too?
    if (addr_.version() == ip::version::ipv6)
    {
      const DWORD ipv6only = 0;
      if (setsockopt(m_context->m_handle, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&ipv6only), sizeof(ipv6only)) == SOCKET_ERROR)
        throw exc::socket_failure();
    }

    // get address of ConnectEx function (argh!)
    {
      GUID guid = WSAID_CONNECTEX;
      DWORD filler;
      if (WSAIoctl(
            m_context->m_handle
          , SIO_GET_EXTENSION_FUNCTION_POINTER
          , &guid
          , sizeof(guid)
          , &m_connect_ex
          , sizeof(m_connect_ex)
          , &filler
          , nullptr
          , nullptr) == SOCKET_ERROR)
        throw exc::socket_failure();
    }

    // --- wtf was Microsoft thinking?
    // must first bind socket to in_addr_any and port 0
    sockaddr* pointer;
    int size;
    sockaddr_in addr4;
    sockaddr_in6 addr6;

    if (addr_.version() == ip::version::ipv4)
      set_address(addr4, ipv4::any, 0, pointer, size);
    else
      set_address(addr6, ipv6::any, 0, pointer, size);

    if (bind(m_context->m_handle, pointer, size) != NO_ERROR)
      throw exc::socket_failure();

    // notify thread pool about incoming async operation
    StartThreadpoolIo(m_context->m_tpio);

    // now set the address to remote and trigger connect request
    memset(&m_context->m_rd_overlapped, 0, sizeof(m_context->m_rd_overlapped));
    if (addr_.version() == ip::version::ipv4)
      set_address(addr4, addr_, port_, pointer, size);
    else
      set_address(addr6, addr_, port_, pointer, size);

    if (!m_connect_ex(
          m_context->m_handle
        , pointer
        , size
        , nullptr
        , 0
        , nullptr
        , &m_context->m_rd_overlapped))
    {
      if (WSAGetLastError() != ERROR_IO_PENDING)
      {
        CancelThreadpoolIo(m_context->m_tpio);
        throw exc::socket_failure();
      }
    }
    else
    {
      CancelThreadpoolIo(m_context->m_tpio);
      // TODO: sync connect worked, process results
    }
  }
  catch (...)
  {
    if (m_context != nullptr)
    {
      auto ctx = m_context;
      m_context = nullptr;
      delete ctx;
    }

    m_state = state::disconnected;
    throw;
  }

}

// ---
// shutdown and disconnect are similar when in connected state. But shutdown
// has to handle other states, too

void stream::disconnect()
{
  state expect = state::connected;
  if (!m_state.compare_exchange_strong(expect, state::disconnecting))
    throw exc::invalid_state();

  auto h = m_context->m_handle;
  m_context->m_handle = invalid_handle;
  m_context = nullptr;
  m_state = state::disconnected;
  closesocket(h);
}

void stream::shutdown()
{
  try
  {
    disconnect();
  }
  catch (const exc::invalid_state&)
  {
    // TODO: handle other states
  }
}


void stream::start()
{
/*
  if (m_state != state::connected)
    return;
  auto aux = m_reader.load();
  if (aux != nullptr)
    aux->m_source.resume();
*/
}

void stream::stop()
{
/*
  if (m_state != state::connected)
    return;
  auto aux = m_reader.load();
  if (aux != nullptr)
    aux->m_source.suspend();
*/
}

void stream::write(const void* data, std::size_t size)
{
/*
  if (m_state != state::connected)
    throw exc::invalid_state();

  bool expected = false;
  if (!m_wr_busy.compare_exchange_strong(expected, true))
    throw exc::operation_failed(cool::ng::error::errc::resource_busy);

  m_wr_data = static_cast<const uint8_t*>(data);
  m_wr_size = size;
  m_wr_pos = 0;
  m_writer.load()->m_source.resume();
*/
}

void stream::process_connect_event()
{
  start_read_source();

  // notify user about connect
  auto cb = m_handler.lock();
  if (cb)
  {
    try { cb->on_event(cb::stream::event::connected, no_error()); } catch (...) { }
  }
  // TODO: what if cb no longer exists???
}

void stream::process_disconnect_event()
{
  {
    auto ctx = m_context;
    m_context = nullptr;
    delete ctx;
  }
  
  auto cb = m_handler.lock();
  m_state = state::disconnected;

  if (cb)
  {
    try { cb->on_event(cb::stream::event::disconnected, no_error()); } catch (...) { }
  }

}

void stream::process_read_event(ULONG_PTR count_)
{

}

void stream::process_write_event(ULONG_PTR count_)
{

}


// ---
// Execution context for a task submitterd to specified async::executor. This
// task will do actual callback into the user code
class exec_for_io : public cool::ng::async::detail::event_context
{
 public:
  exec_for_io(stream::context* ctx_
            , PVOID overlapped_
            , ULONG io_result_
            , ULONG_PTR num_transferred_)
      : m_ctx(ctx_)
      , m_overlapped(overlapped_)
      , m_io_result(io_result_)
      , m_num_transferred(num_transferred_)
  { /* noop */ }

  void entry_point() override
  {
    try { m_ctx->m_stream->on_event(m_ctx, m_overlapped, m_io_result, m_num_transferred); } catch (...) { /* noop */ }
  }

 private:
  stream::context* m_ctx;
  PVOID            m_overlapped;
  ULONG            m_io_result;
  ULONG_PTR        m_num_transferred;
};

// -- callback from the threadpool for i/o calls - this callback is a result
//    of StartThreadpoIo work object and was not submitted in the context of any
//    runner. To support synchronized semantics of event sources this callback
//    must submit work to the executor of the runner the event source is
//    associated with. To do so it must create a work context and submit it
//    to executor::run method.

void stream::on_event(
      PTP_CALLBACK_INSTANCE instance_
    , PVOID context_
    , PVOID overlapped_
    , ULONG io_result_
    , ULONG_PTR num_transferred_
    , PTP_IO io_)
{
  if (context_ != nullptr)
  {
    auto ctx = static_cast<stream::context*>(context_);

    // get executor and schedule event callback for execution on stream's runner
    auto r = ctx->m_stream->m_executor.lock();
    if (r)
    {
      auto exec = new exec_for_io(ctx, overlapped_, io_result_, num_transferred_);
      r->run(exec);
    }
    else
    {
      delete ctx;
    }
  }
  //  TODO: error handling????
}

void stream::on_event(context* ctx, PVOID overlapped_, ULONG io_result_, ULONG_PTR num_transferred_)
{
  state expect;

  switch (io_result_)
  {
    case NO_ERROR:
      // right, figure out what is going on ...
      if (overlapped_ == &m_context->m_rd_overlapped)   // must be read or connect
      {
        expect = state::connecting;
        if (m_state.compare_exchange_strong(expect, state::connected))
        {
          process_connect_event();
        }
        else
        {
          if (num_transferred_ == 0)
          {
            // -- sometimes caused by remote peer closing connection
            expect = state::connected;
            if (m_state.compare_exchange_strong(expect, state::disconnecting))
              process_disconnect_event();
          }
          else
          {
            process_read_event(num_transferred_);
          }
        }
      }
      else if (overlapped_ == &m_context->m_wr_overlapped)
      {
       process_write_event(num_transferred_);
      }
      break;

    // -- sometimes caused by remote peer closing connection
    case ERROR_NETNAME_DELETED:
      expect = state::connected;
      if (m_state.compare_exchange_strong(expect, state::disconnecting))
        process_disconnect_event();
      else
      {
        // TODO: what goes here?
      }
      break;

    // -- caused by shutdown() or disconnect
    case ERROR_CONNECTION_ABORTED:
      delete ctx;
      break;

    default:
      // TODO: what goes here?
      break;
  }
}


} } } } }


