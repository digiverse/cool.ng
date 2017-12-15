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
#include <stdint.h>
#include "cool/ng/error.h"
#include "cool/ng/exception.h"

#include "cool/ng/async/net/stream.h"
#include "event_sources.h"

#pragma comment(lib, "Ws2_32.lib")

namespace cool { namespace ng { namespace async {

using cool::ng::error::no_error;

namespace exc = cool::ng::exception;
namespace ip = cool::ng::net::ip;
namespace ipv4 = cool::ng::net::ipv4;
namespace ipv6 = cool::ng::net::ipv6;


namespace impl {
// ==========================================================================
// ======
// ======
// ====== Timer
// ======
// ======
// ==========================================================================


timer::context::context(const timer::ptr& t_)
    : m_timer(t_)
    , m_pool(async::impl::poolmgr::get_poolmgr())
    , m_source(nullptr)
    , m_active(false)
{
  m_source = CreateThreadpoolTimer(on_event, this, m_pool->get_environ());
  if (m_source == nullptr)
    throw exc::threadpool_failure();
}

timer::context::~context()
{ /* noop */ }

void timer::context::on_event(PTP_CALLBACK_INSTANCE i_, PVOID ctx_, PTP_TIMER t_)
{
  auto self = static_cast<context*>(ctx_);
  switch (self->m_timer->m_state)
  {
    case state::destroying:
      CloseThreadpoolTimer(self->m_source);
      delete self;
      return;

    case state::running:
      if (self->m_active.load())
        self->m_timer->expired();
      break;
  }
}

timer::timer(const std::weak_ptr<cb::timer>& t_
           , uint64_t p_
           , uint64_t l_)
  : named("si.digiverse.ng.cool.timer")
  , m_state(state::running)
  , m_callback(t_)
  , m_context(nullptr)
  , m_period((p_ + 500) / 1000)
  , m_leeway((l_ + 500) / 1000)
{
  if (m_period == 0)
    m_period = 1;
  if (m_leeway == 0)
    m_leeway = 1;
}

timer::~timer()
{ }

void timer::initialize(const std::shared_ptr<async::impl::executor>& ex_)
{
  m_executor = ex_;
  m_context = new context(self().lock());
}

void timer::period(uint64_t p_, uint64_t l_)
{
  if (p_ == 0)
    throw exc::illegal_argument();

  m_period = (p_ + 500) / 1000;
  m_leeway = (l_ + 500) / 1000;
  if (m_period == 0)
    m_period = 1;
  if (m_leeway == 0)
    m_leeway = 1;
}

void timer::shutdown()
{
  m_state = state::destroying;
  stop();  // make it fire once more to do cleanup
}

void timer::start()
{
  FILETIME fdt;
  ULARGE_INTEGER dt;

  // set timer to fire in m_period msec(NOTE: negative number)
#pragma warning( suppress: 4146 )
  dt.QuadPart = -static_cast<ULONGLONG>(m_period * 1000 * 10);
  fdt.dwHighDateTime = dt.HighPart;
  fdt.dwLowDateTime = dt.LowPart;

  m_context->m_active = true;
  SetThreadpoolTimer(m_context->m_source, &fdt, static_cast<DWORD>(m_period), static_cast<DWORD>(m_leeway));
}

void timer::stop()
{
  m_context->m_active = false;
  FILETIME fdt;

  fdt.dwHighDateTime = 0;
  fdt.dwLowDateTime = 0;
  // cancel any previous settings - it will fire once but m_active
  // flag should prevent propagation to user callback
  SetThreadpoolTimer(m_context->m_source, &fdt, 0, 0);
}

// ---
// Execution context for a task submitterd to specified async::executor. This
// task will do actual callback into the user code
class exec_for_timer : public cool::ng::async::detail::event_context
{
 public:
  exec_for_timer(const std::weak_ptr<cb::timer>& cb_)
      : m_handler(cb_)
  { /* noop */ }

  void entry_point() override
  {
    auto cb = static_cast<exec_for_timer*>(static_cast<void*>(this))->m_handler.lock();
    if (cb)
      cb->expired();
  }

 private:
  std::weak_ptr<cb::timer> m_handler;
};

void timer::expired()
{
  try
  {
    auto r = m_executor.lock();
    if (r)
      r->run(new exec_for_timer(m_callback));
  }
  catch (...)
  { /* noop */ }
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

using cool::ng::net::handle;
using cool::ng::net::invalid_handle;

namespace {

void set_address(sockaddr_in& sa_, const ip::address& a_, uint16_t port_, sockaddr*& p_, int& size)
{
  memset(&sa_, 0, sizeof(sa_));
  sa_.sin_family = AF_INET;
  sa_.sin_addr = static_cast<in_addr>(a_);
  sa_.sin_port = htons(port_);
  p_ = reinterpret_cast<sockaddr*>(&sa_);
  size = sizeof(sa_);
}

void set_address(sockaddr_in6& sa_, const ip::address& a_, uint16_t port_, sockaddr*& p_, int& size)
{
  memset(&sa_, 0, sizeof(sa_));
  sa_.sin6_family = AF_INET6;
  sa_.sin6_addr = static_cast<in6_addr>(a_);
  sa_.sin6_port = htons(port_);
  p_ = reinterpret_cast<sockaddr*>(&sa_);
  size = sizeof(sa_);
}

} // anonymous namespace

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
server::context::context(const server::ptr& s_, const ip::address& addr_, uint16_t port_)
    : m_server(s_)
    , m_pool(async::impl::poolmgr::get_poolmgr())
    , m_handle(invalid_handle)
    , m_sock_type(addr_.version() == ip::version::ipv4 ? AF_INET : AF_INET6)
    , m_accept_ex(nullptr)
    , m_get_sock_addrs(nullptr)
    , m_tpio(nullptr)
    , m_client_handle(invalid_handle)
{
  try
  {
    // ----
    // create and bind listen socket
    {
      m_handle = WSASocketW(m_sock_type, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
      if (m_handle == invalid_handle)
        throw exc::socket_failure();

      const int enable = 1;
      if (setsockopt(m_handle, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enable), sizeof(enable)) == SOCKET_ERROR)
        throw exc::socket_failure();

      // enable connect from IPv4 sockets, too ... something that's enable by
      // default on OSX and Linux
      if (addr_.version() == ip::version::ipv6)
      {
        const DWORD ipv6only = 0;
        if (setsockopt(m_handle, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&ipv6only), sizeof(ipv6only)) == SOCKET_ERROR)
          throw exc::socket_failure();
      }

      sockaddr_in addr4;
      sockaddr_in6 addr6;
      sockaddr* p;
      int size;

      if (addr_.version() == ip::version::ipv4)
        set_address(addr4, addr_, port_, p, size);
      else
        set_address(addr6, addr_, port_, p, size);

      if (bind(m_handle, p, static_cast<int>(size)) == SOCKET_ERROR)
        throw exc::socket_failure();

      if (listen(m_handle, 10) == SOCKET_ERROR)
        throw exc::socket_failure();
    }

    // ----
    // Fetch AcceptEx and GetAcceptExSockAddrs function pointers
    {
      DWORD filler;

      GUID guid = WSAID_ACCEPTEX;
      if (WSAIoctl(
            m_handle
          , SIO_GET_EXTENSION_FUNCTION_POINTER
          , &guid
          , sizeof(guid)
          , &m_accept_ex
          , sizeof(m_accept_ex)
          , &filler
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
          , &filler
          , nullptr
          , nullptr) == SOCKET_ERROR)
        throw exc::socket_failure();
    }

    // ----
    // Create thread pool i/o completion object and associate it with
    // the listen socket
    m_tpio = CreateThreadpoolIo(
        reinterpret_cast<HANDLE>(m_handle)
      , context::on_accept
      , this
      , m_pool->get_environ());
    if (m_tpio == nullptr)
      throw exc::threadpool_failure();
  }
  catch (...)
  {
    if (m_handle != invalid_handle)
    {
      closesocket(m_handle);
      m_handle = invalid_handle;
    }
    if (m_tpio != nullptr)
    {
      CancelThreadpoolIo(m_tpio);
      m_tpio = nullptr;
    }
    throw;
  }
}

server::context::~context()
{
  if (m_handle != invalid_handle)
    closesocket(m_handle);

  if (m_tpio != nullptr)
    CancelThreadpoolIo(m_tpio);
}

void server::context::start_accept()
{
  m_client_handle = ::WSASocketW(m_sock_type, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
  if (m_client_handle == invalid_handle)
    throw exc::socket_failure();

  StartThreadpoolIo(m_tpio);
  memset(&m_overlapped, 0, sizeof(m_overlapped));
  if (!m_accept_ex(
      m_handle
    , m_client_handle
    , m_buffer
    , 0
    , sizeof(m_buffer) / 2
    , sizeof(m_buffer) / 2
    , nullptr //&m_filler
    , &m_overlapped))
  {
    auto hr = WSAGetLastError();
    if (hr != ERROR_IO_PENDING)
    {
      CancelThreadpoolIo(m_tpio);
          // TODO: what to do here????
    }
  }
}

void server::context::stop_accept()
{
  CancelIoEx(reinterpret_cast<HANDLE>(m_handle), &m_overlapped);
}

void server::context::shutdown()
{
  // this should make a roundtrip though threadpool with ABORTED error
  // at which point callback should delete context
  auto s = m_handle;
  m_handle = invalid_handle;
  closesocket(s);
}

// -- callback from the threadpool for AcceptEx call - this callback is a result
//    of StartThreadpoIo work object and was not submitted in the context of any
//    runner. To support synchronized semantics of event sources this callback
//    must submit work to the executor of the runner the event source is
//    associated with. To do so it must create a work context and submit it
//    to executor::run method.
void server::context::on_accept(
      PTP_CALLBACK_INSTANCE instance_
    , PVOID context_
    , PVOID overlapped_
    , ULONG io_result_
    , ULONG_PTR num_transferred_
    , PTP_IO io_)
{
  if (context_ == nullptr)
    return;   // can't do anything here, at least prevents null pointer exception

  auto ctx = static_cast<context*>(context_);
  switch (io_result_)
  {
    case NO_ERROR:
      ctx->process_accept();
      break;

	// either somebody closed the socket from the outside (shutdown()) or cancelled
	// the pending AcceptEx operation (stop()) - use m_server's state to differentiate
    case ERROR_OPERATION_ABORTED: 
      {
        state expect = state::stopping;
        if (ctx->m_server->m_state.compare_exchange_strong(expect, state::stopped))
          return;

        if (expect == state::destroying)
          delete ctx;
      }
      break;

    default:
      // TODO: what to do here:
      break;
  }
}

void server::context::process_accept()
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

  if (setsockopt(
      m_client_handle
    , SOL_SOCKET
    , SO_UPDATE_ACCEPT_CONTEXT
    , reinterpret_cast<char *>(&m_handle), sizeof(m_handle)) != NO_ERROR)
  {
    // TODO: error handling
  }

  ip::host_container ca = *remote;
  uint16_t port = ntohs(remote->ss_family == AF_INET
    ? reinterpret_cast<sockaddr_in*>(remote)->sin_port
      : ntohs(reinterpret_cast<sockaddr_in6*>(remote)->sin6_port));

  m_server->process_accept(ca, port);
}

server::server(const std::shared_ptr<async::impl::executor>& ex_
             , const cb::server::weak_ptr& cb_)
    : named("si.digiverse.ng.cool.server")
    , m_state(state::stopped)
    , m_executor(ex_)
    , m_handler(cb_)
    , m_context(nullptr)
{ /* noop */ }

server::~server()
{ /* noop */ }

void server::initialize(const cool::ng::net::ip::address& addr_, uint16_t port_)
{
  m_context = new context(self().lock(), addr_, port_);
}

void server::start()
{
  state expect = state::stopped;
  try
  {
    if (m_state.compare_exchange_strong(expect, state::starting))
    {
      m_context->start_accept();
      expect = state::starting;
      m_state.compare_exchange_strong(expect, state::accepting);
      return;
    }
  }
  catch (...)
  {
    m_state = state::error;
    auto aux = m_context;
    m_context = nullptr;
    delete m_context;
    return;
  }

  expect = state::accepting;
  if (m_state.compare_exchange_strong(expect, state::accepting))
    return;   // already  accepting

  throw exc::invalid_state();
}

void server::stop()
{
  state expect = state::accepting;
  try
  {
    if (m_state.compare_exchange_strong(expect, state::stopping))
    {
      m_context->stop_accept();
      return;
    }
  }
  catch (...)
  {
    m_state = state::error;
    auto aux = m_context;
    m_context = nullptr;
    delete m_context;
    return;
  }

  expect = state::stopped;
  if (m_state.compare_exchange_strong(expect, state::stopped))
    return;   // already  accepting

  throw exc::invalid_state();
}

void server::shutdown()
{
  // if accepting it's simple
  state expect = state::accepting;
  if (m_state.compare_exchange_strong(expect, state::destroying))
  {
    m_context->shutdown();
    return;
  }

  // if stopped it's simple, too
  expect = state::stopped;
  if (m_state.compare_exchange_strong(expect, state::destroying))
  {
    delete m_context;
    return;
  }

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
                , uint16_t port_)
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
      try
      {
        // this call is made directly to detail::server template in the context of
        // the runner
        auto s = cb->manufacture(self->m_addr, self->m_port);
        server::install_handle(s, self->m_handle);
        try { cb->on_connect(s); } catch (...) { }
      }
      catch (...)
      {
        closesocket(self->m_handle);
      }
    }
    else
    {
      closesocket(self->m_handle);
    }
  }

 private:
  ip::host_container   m_addr;
  int                  m_port;
  handle               m_handle;
  cb::server::weak_ptr m_handler;
};

void server::process_accept(const cool::ng::net::ip::address& addr_, uint16_t port_)
{
  auto r = m_executor.lock();
  if (r)
  {
    cool::ng::async::detail::event_context* ctx = nullptr;

    ctx = new exec_for_accept(m_handler, m_context->m_client_handle, addr_, port_);

    if (ctx != nullptr)
      r->run(ctx);
  }
  else
  {
    // executor no longer exists hence nobody can process connect request -
    // just close the client handle
    closesocket(m_context->m_client_handle);
  }

  // all is done, restart the overlapped accept
  m_context->start_accept();
}

void server::install_handle(cool::ng::async::net::stream& s_, cool::ng::net::handle h_)
{
  s_.m_impl->set_handle(h_);
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
    // this call is made into impl::server, but in the context of the runner
    m_ctx->m_stream->on_event(m_ctx, m_overlapped, m_io_result, m_num_transferred);
  }

 private:
  stream::context* m_ctx;
  PVOID            m_overlapped;
  ULONG            m_io_result;
  ULONG_PTR        m_num_transferred;
};

stream::context::context(const stream::ptr& s_, handle h_, void* buf_, std::size_t sz_)
  : m_stream(s_)
  , m_handle(h_)
  , m_tpio(nullptr)
  , m_rd_data(buf_)
  , m_rd_size(sz_)
  , m_rd_is_mine(buf_ == nullptr)
  , m_wr_busy(false)
{
  if (m_rd_is_mine)
    m_rd_data = new uint8_t[m_rd_size];

  m_tpio = CreateThreadpoolIo(
      reinterpret_cast<HANDLE>(m_handle)
    , context::on_event
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

// -- callback from the threadpool for i/o calls - this callback is a result
//    of StartThreadpoIo work object and was not submitted in the context of any
//    runner. To support synchronized semantics of event sources this callback
//    must submit work to the executor of the runner the event source is
//    associated with. To do so it must create a work context and submit it
//    to executor::run method.

void stream::context::on_event(
      PTP_CALLBACK_INSTANCE instance_
    , PVOID context_
    , PVOID overlapped_
    , ULONG io_result_
    , ULONG_PTR num_transferred_
    , PTP_IO io_)
{
  if (context_ != nullptr)
  {
    auto ctx = static_cast<context*>(context_);

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

stream::stream(const std::weak_ptr<async::impl::executor>& ex_
             , const cb::stream::weak_ptr& cb_)
    : named("si.digiverse.ng.cool.stream")
    , m_state(state::disconnected)
    , m_executor(ex_)
    , m_pool(async::impl::poolmgr::get_poolmgr())
    , m_handler(cb_)
    , m_context(nullptr)
    , m_connect_ex(nullptr)
    , m_rd_size(32768)
    , m_rd_data(nullptr)
{ /* noop */ }

stream::~stream()
{ /* noop */ }

void stream::initialize(const cool::ng::net::ip::address& addr_
                      , uint16_t port_
                      , void* buf_
                      , std::size_t bufsz_)
{
  m_rd_size = bufsz_;
  m_rd_data = buf_;

  connect(addr_, port_);
}

void stream::set_handle(handle h_)
{
  try
  {
    m_context = new context(self().lock(), h_, m_rd_data, m_rd_size);

    m_state = state::connected;
    start_read_source();
  }
  catch (...)
  {
    // at this point no ovelapped operation is running hence
    // delete context directly at here
    if (m_context != nullptr)
      delete m_context;
    m_context = nullptr;
    m_state = state::disconnected;
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
  memset(&m_context->m_rd_overlapped, 0, sizeof(m_context->m_rd_overlapped));

  StartThreadpoolIo(m_context->m_tpio);
  if (!ReadFile(reinterpret_cast<HANDLE>(m_context->m_handle)
    , m_context->m_rd_data
    , static_cast<DWORD>(m_context->m_rd_size)
    , nullptr
    , &m_context->m_rd_overlapped))
  {
    auto err = GetLastError();
    if (err != ERROR_IO_PENDING)
    {
      // TODO: disconnect, report error
      CancelThreadpoolIo(m_context->m_tpio);
      return;
    }
  }
}

void stream::start_write_source()
{
  memset(&m_context->m_wr_overlapped, 0, sizeof(m_context->m_wr_overlapped));

  // note really necessary. still ... internal sizes are std::size_t (64 bits)
  // while WriteFile works with 32 bits ... just in case somebody really
  // wanted to do write > 2G
  std::size_t full_size = m_context->m_wr_size - m_context->m_wr_pos;
  DWORD size = full_size > INT32_MAX ? INT32_MAX : static_cast<DWORD>(full_size);

  StartThreadpoolIo(m_context->m_tpio);
  if (!WriteFile(
      reinterpret_cast<HANDLE>(m_context->m_handle)
    , m_context->m_wr_data + m_context->m_wr_pos
    , size
    , nullptr
    , &m_context->m_wr_overlapped))
  {
    auto err = GetLastError();
    if (err != ERROR_IO_PENDING)
    {
      // TODO: error handling
      CancelThreadpoolIo(m_context->m_tpio);
      return;
    }
  }
}

// This method is always called from the user code in the context of the
// user thread; either from the stream's ctor or using connect API call
void stream::connect(const ip::address& addr_, uint16_t port_)
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

    // TODO: is this needed to client sockets, too?
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

void stream::write(const void* data, std::size_t size)
{
  if (m_state != state::connected)
    throw exc::invalid_state();

  bool expected = false;
  if (!m_context->m_wr_busy.compare_exchange_strong(expected, true))
    throw exc::operation_failed(cool::ng::error::errc::resource_busy);

  m_context->m_wr_data = static_cast<const uint8_t*>(data);
  m_context->m_wr_size = size;
  m_context->m_wr_pos = 0;
  start_write_source();
}

void stream::process_connect_event(ULONG io_result_)
{
  // notify user about connect
  auto cb = m_handler.lock();
  if (cb)
  {
    if (io_result_ == NO_ERROR)
    {
      try { cb->on_event(detail::oob_event::connect, no_error()); } catch (...) { }
      start_read_source();
      return;  // upon success job done, return from the call
    }
    else
    {
      try { cb->on_event(detail::oob_event::failure, std::error_code(io_result_, std::system_category())); } catch (...) { }
      // ... let the code continue below for cleanup ...
    }
  }

  // either if failed or if callback service no longer exist, set the state  to
  // disconnected and remove the current context
  m_state = state::disconnected;
  auto context = m_context;
  m_context = nullptr;
  delete context;
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
    try { cb->on_event(detail::oob_event::disconnect, no_error()); } catch (...) { }
  }

}

void stream::process_read_event(ULONG_PTR count_)
{
  // notify user about connect
  auto cb = m_handler.lock();
  if (cb)
  {
    auto data = m_context->m_rd_data;
    std::size_t size = count_;

    try
    {
      try { cb->on_read(data, size); } catch (...) { /* noop */ }
      if (data != m_context->m_rd_data && size > 0)
      {
        if (m_context->m_rd_is_mine)
        {
          delete [] static_cast<uint8_t*>(m_context->m_rd_data);
          m_context->m_rd_is_mine = false;
        }
        m_context->m_rd_data = data;
        m_context->m_rd_size = size;
      }
    }
    catch (...)
    { /* noop */ }
  }

  start_read_source();
}

void stream::process_write_event(ULONG_PTR count_)
{
  m_context->m_wr_pos += count_;
  if (m_context->m_wr_pos >= m_context->m_wr_size)
  {
    auto cb = m_handler.lock();
    m_context->m_wr_busy = false;
    if (cb)
      try { cb->on_write(m_context->m_wr_data, m_context->m_wr_size); } catch (...) { /* noop */ }
  }
  else
    start_write_source();
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
          process_connect_event(io_result_);
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

    // -- check if failed connect request
    default:
      expect = state::connecting;
      if (m_state.compare_exchange_strong(expect, state::disconnected))
      {
        process_connect_event(io_result_);
      }
      break;
  }
}


} } } } }


