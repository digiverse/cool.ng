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
#include <condition_variable>
#include <Cassert>
#include <mutex>

#include "cool/ng/error.h"
#include "cool/ng/exception.h"

#include "cool/ng/async/net/stream.h"
#include "event_sources.h"

#pragma comment(lib, "Ws2_32.lib")

#define DO_TRACE 0
// #define DO_TRACE 1

#if DO_TRACE == 1
# define TRACE(a, b) std::cout << "---- [" << __LINE__ << "] " << a << ": " << b << "\n"
#else
# define TRACE(a,b)
#endif
namespace cool { namespace ng { namespace async {

using cool::ng::error::no_error;

namespace exc = cool::ng::exception;
namespace ip = cool::ng::net::ip;
namespace ipv4 = cool::ng::net::ipv4;
namespace ipv6 = cool::ng::net::ipv6;
namespace error = cool::ng::error;

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

VOID CALLBACK timer::context::on_event(PTP_CALLBACK_INSTANCE i_, PVOID ctx_, PTP_TIMER t_)
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
  exec_for_timer(PTP_CALLBACK_ENVIRON env, const std::weak_ptr<cb::timer>& cb_)
    : m_handler(cb_)
    , m_environ(env)
  { /* noop */ }

  void entry_point() override
  {
    auto cb = static_cast<exec_for_timer*>(static_cast<void*>(this))->m_handler.lock();
    if (cb)
      cb->expired();
  }

  void* environment() override { return m_environ; }

 private:
  std::weak_ptr<cb::timer> m_handler;
  PTP_CALLBACK_ENVIRON m_environ;
};

void timer::expired()
{
  try
  {
    auto r = m_executor.lock();
    if (r)
      r->run(new exec_for_timer(m_context->m_pool->get_environ(), m_callback));
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

server::server(const std::shared_ptr<async::impl::executor>& ex_
             , const cb::server::weak_ptr& cb_)
    : named("server") // named("si.digiverse.ng.cool.server")
    , m_state(state::init)
    , m_executor(ex_)
    , m_handler(cb_)
    , m_pool(async::impl::poolmgr::get_poolmgr())
    , m_handle(invalid_handle)
    , m_client_handle(invalid_handle)
    , m_sock_type(AF_INET)
    , m_accept_ex(nullptr)
    , m_get_sock_addrs(nullptr)
    , m_tpio(nullptr)
    , m_context(nullptr)
{ /* noop */ }

server::~server()
{
  TRACE("server", "to delete server");
  if (m_handle != invalid_handle)
    closesocket(m_handle);
  if (m_client_handle != invalid_handle)
    closesocket(m_client_handle);
  if (m_tpio != nullptr)
  {
    CloseThreadpoolIo(m_tpio);
  }
  if (m_context != nullptr)
    delete m_context;
  TRACE("server", "server deleted");
}

void server::initialize(const cool::ng::net::ip::address& addr_, uint16_t port_)
{
  m_sock_type = addr_.version() == ip::version::ipv6 ? AF_INET6 : AF_INET;
  m_context = new ptr(self().lock());

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
      , server::on_accept
      , m_context
      , m_pool->get_environ());
    if (m_tpio == nullptr)
    {
      throw exc::threadpool_failure();
    }

    m_state = state::stopped;
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
      CloseThreadpoolIo(m_tpio);
      m_tpio = nullptr;
    }
    if (m_context != nullptr)
    {
      delete m_context;
      m_context = nullptr;
    }

    m_state = state::error;
    throw;
  }
}

void server::start_accept()
{
  m_client_handle = ::WSASocketW(m_sock_type, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
  if (m_client_handle == invalid_handle)
    throw exc::socket_failure();

  StartThreadpoolIo(m_tpio);

  BOOL res;
  {
    std::unique_lock<async::impl::critical_section> l(m_cs);

    memset(&m_overlapped, 0, sizeof(m_overlapped));
    res = m_accept_ex(m_handle , m_client_handle, m_buffer , 0 , sizeof(m_buffer) / 2, sizeof(m_buffer) / 2, nullptr , &m_overlapped);
  }

  if (!res)
  {
    auto hr = WSAGetLastError();
    if (hr != ERROR_IO_PENDING)
    {
      CancelThreadpoolIo(m_tpio);
      closesocket(m_client_handle);
      m_client_handle = invalid_handle;
    }
  }
}

void server::stop_accept()
{
  std::unique_lock<async::impl::critical_section> l(m_cs);
  CancelIoEx(reinterpret_cast<HANDLE>(m_handle), nullptr);
}

void server::start()
{
  state expect = state::stopped;
  try
  {
    if (m_state.compare_exchange_strong(expect, state::starting))
    {
      start_accept();
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
      stop_accept();
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
    auto s = m_handle;
    m_handle = invalid_handle;
    m_context = nullptr;     // prevent dtorr from deleting context
    closesocket(s);
    return;
  }

  // if stopped it's simple, too
  expect = state::stopped;
  if (m_state.compare_exchange_strong(expect, state::destroying))
  {
    delete m_context;
    m_context = nullptr;
    return;
  }
}

// ---
// Execution context for a task submitterd to specified async::executor. This
// task will do actual callback into the user code
class exec_for_accept : public cool::ng::async::detail::event_context
{
 public:
  exec_for_accept(PTP_CALLBACK_ENVIRON env_
                , const cb::server::weak_ptr& cb_
                , handle h_
                , const ip::host_container& addr_
                , uint16_t port_)
      : m_addr(addr_)
      , m_port(port_)
      , m_handle(h_)
      , m_handler(cb_)
      , m_environ(env_)
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
        // use factory to get new stream instance, install client handle
        // and do final on_connect callback
        auto s = cb->manufacture(self->m_addr, self->m_port);
        server::install_handle(s, self->m_handle);
        try { cb->on_connect(s); } catch (...) { }
      }
      catch (...)
      {
        // if stream factory threw just close handle thus disconnecting client
        closesocket(self->m_handle);
      }
    }
    else
    {
      // if user callbackk no longer available just disconnect client
      closesocket(self->m_handle);
    }
  }

  void* environment() override { return m_environ; }

private:
  ip::host_container   m_addr;
  int                  m_port;
  handle               m_handle;
  cb::server::weak_ptr m_handler;
  PTP_CALLBACK_ENVIRON m_environ;
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
  if (context_ == nullptr)
    return;   // can't do anything here, at least prevents null pointer exception

  auto ctx = static_cast<std::shared_ptr<server>*>(context_);

  switch (io_result_)
  {
    case NO_ERROR:
      (*ctx)->process_accept();
      break;

  // either somebody closed the socket from the outside (shutdown()) or cancelled
  // the pending AcceptEx operation (stop()) - use server's state to differentiate
    case ERROR_OPERATION_ABORTED:
      {
        state expect = state::stopping;
        if ((*ctx)->m_state.compare_exchange_strong(expect, state::stopped))
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

void server::process_accept()
{

  sockaddr_storage* local;
  sockaddr_storage* remote;
  int len_local, len_remote;

  // fetch local and remote addresses ...
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

  ip::host_container addr = *remote;
  uint16_t port = ntohs(remote->ss_family == AF_INET
    ? reinterpret_cast<sockaddr_in*>(remote)->sin_port
      : ntohs(reinterpret_cast<sockaddr_in6*>(remote)->sin6_port));

  // ... and schedule callback to user code
  auto r = m_executor.lock();
  if (r)
  {
    cool::ng::async::detail::event_context* ctx = nullptr;

    auto aux = m_client_handle;
    m_client_handle = invalid_handle;
    ctx = new exec_for_accept(m_pool->get_environ(), m_handler, aux, addr, port);

    if (ctx != nullptr)
      r->run(ctx);
  }
  else
  {
    // executor no longer exists hence nobody can process connect request -
    // just close the client handle
    closesocket(m_client_handle);
    m_client_handle = invalid_handle;
  }

  // all is done, restart the overlapped accept
  start_accept();
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

std::error_code translate_error(ULONG result)
{
  std::error_code err;

  switch (result)
  {
    case ERROR_OPERATION_ABORTED:
      return error::make_error_code(error::errc::request_aborted);

    case ERROR_HOST_UNREACHABLE:
    case ERROR_NETWORK_UNREACHABLE:
    case ERROR_CONNECTION_REFUSED:
    case ERROR_PORT_UNREACHABLE:
      return error::make_error_code(error::errc::request_failed);
    default:
      break;
  }
  return std::error_code(result, std::system_category());
}


// ---
// Execution context for a task submitterd to specified async::executor. This
// task will do actual callback into the user code

class exec_for_io : public cool::ng::async::detail::event_context
{
 public:
  exec_for_io(PTP_CALLBACK_ENVIRON env, const std::function<void()>& l_)
    : m_lambda(l_)
    , m_environ(env)
  { /* noop */ }

  void* environment() override { return m_environ; }
  void entry_point() override { m_lambda(); }

 protected:
  PTP_CALLBACK_ENVIRON m_environ;
  std::function<void()> m_lambda;
};

class exec_for_cleanup : public cool::ng::async::detail::cleanup_context
{
 public:
  exec_for_cleanup(PTP_CALLBACK_ENVIRON env, const std::function<void()>& l_)
    : m_lambda(l_)
    , m_environ(env)
  { /* noop */ }

  void* environment() override { return m_environ; }
  void entry_point() override { m_lambda(); }

 protected:
  PTP_CALLBACK_ENVIRON m_environ;
  std::function<void()> m_lambda;
};

stream::context::context(async::impl::poolmgr::ptr p_, const stream::ptr& s_, void* buf_, std::size_t sz_)
  : m_stream(s_)
  , m_handle(invalid_handle)
  , m_state(state::connecting)
  , m_tpio(nullptr)
  , m_rd_data(buf_)
  , m_rd_size(sz_)
  , m_rd_is_mine(buf_ == nullptr)
  , m_wr_busy(false)
  , m_cleanup(nullptr)
{
  TRACE(s_->name(), "to create context");
  try
  {
    if (sz_ == 0)
      throw exc::illegal_argument();

    if (m_rd_is_mine)
      m_rd_data = new uint8_t[m_rd_size];

    // initialize callback environment, associate it with thread pool and
    // create a cleanup group for this environment
    InitializeThreadpoolEnvironment(&m_environ);
    p_->add_environ(&m_environ);
    m_cleanup = CreateThreadpoolCleanupGroup();
    if (m_cleanup == nullptr)
      throw exc::threadpool_failure();
    SetThreadpoolCallbackCleanupGroup(&m_environ, m_cleanup, NULL);

    TRACE(s_->name(), "context created, env=" << &m_environ);
  }
  catch (...)  // cleanup on exception
  {
    TRACE(s_->name(), "context failed to create");
    if (m_rd_is_mine && m_rd_data != nullptr)
      delete [] static_cast<uint8_t*>(m_rd_data);
    throw;
  }
}

void stream::context::set_handle(context::sptr* ctxptr, handle h_)
{
  TRACE(m_stream->name(), " context::set_handle: setting handle");
  if (m_handle != invalid_handle)
    throw exc::invalid_state();

  m_handle = h_;

  m_tpio = CreateThreadpoolIo(
      reinterpret_cast<HANDLE>(m_handle)
    , context::on_event
    , ctxptr
    , &m_environ
  );

  if (m_tpio == nullptr)
    throw exc::threadpool_failure();
}

stream::context::~context()
{
  TRACE(m_stream->name(), "to delete context");
  // if (m_handle != invalid_handle)
  //   closesocket(m_handle);

  if (m_cleanup != nullptr)
  {
    CloseThreadpoolCleanupGroup(m_cleanup);
    DestroyThreadpoolEnvironment(&m_environ);
  }

  if (m_rd_is_mine && m_rd_data != nullptr)
    delete [] static_cast<uint8_t*>(m_rd_data);

  TRACE(m_stream->name(), "context deleted");
}

void stream::context::on_event(
      PTP_CALLBACK_INSTANCE instance_
    , PVOID context_
    , PVOID overlapped_
    , ULONG io_result_
    , ULONG_PTR num_transferred_
    , PTP_IO io_)
{
  assert(context_ != nullptr);   // context_ cannot be nullptr

  auto cp = static_cast<context::sptr*>(context_);
  TRACE((*cp)->m_stream->name(), "context::on_event - result " << io_result_ << ", num_transferred " << num_transferred_);
  (*cp)->m_stream->process_event(cp, overlapped_, io_result_, num_transferred_);
}

stream::stream(const std::weak_ptr<async::impl::executor>& ex_
             , const cb::stream::weak_ptr& cb_)
    : named("stream") //named("si.digiverse.ng.cool.stream")
    , m_executor(ex_)
    , m_pool(async::impl::poolmgr::get_poolmgr())
    , m_handler(cb_)
    , m_context(nullptr)
    , m_connect_ex(nullptr)
    , m_rd_size(32768)
    , m_rd_data(nullptr)
{ /* noop */ }

stream::~stream()
{
  shutdown();
  TRACE(name(), "now destroyed");
}

stream::state stream::get_state() const
{
  auto c = m_context.load();
  return c == nullptr ? state::disconnected : (*c)->m_state.load();
}

stream::state stream::set_state(state e_, state s_)
{
  auto c = m_context.load();
  if (c == nullptr)
    return state::disconnected;

  (*c)->m_state.compare_exchange_strong(e_, s_);
  return e_;
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

void stream::initialize(void* buf_, std::size_t bufsz_)
{
  m_rd_size = bufsz_;
  m_rd_data = buf_;
}

void stream::set_handle(handle h_)
{
  TRACE(name(), "setting handle");
  try
  {
    auto cp = new context::sptr(new context(m_pool, self().lock(), m_rd_data, m_rd_size));

    (*cp)->set_handle(cp, h_);

    context::sptr* aux = nullptr;
    if (!m_context.compare_exchange_strong(aux, cp))
    {
      delete cp;
      throw exc::invalid_state();
    }

    if (set_state(state::connecting, state::connected) != state::connecting)
    {
      TRACE(name(), "inexplicably failed to set 'connected' state");
      throw exc::invalid_state();
    }

    start_read_source(cp);
    TRACE(name(), "set_handle completed");
  }
  catch (...)
  {
    TRACE(name(), "set_handle failed with exception");
    // at this point no ovelapped operation is running hence
    // delete context directly at here
    auto cp = m_context.load();
    if (cp != nullptr && m_context.compare_exchange_strong(cp, nullptr))
      delete cp;
    throw;
  }
}

// This method is always called from the user code in the context of the
// user thread; either from the stream's ctor or using connect API call
void stream::connect(const ip::address& addr_, uint16_t port_)
{
  handle handle = invalid_handle;
  context::sptr* cp = nullptr;

  {
    cp = new context::sptr(new context(m_pool, self().lock(), m_rd_data, m_rd_size));

    context::sptr* expect = nullptr;
    if (!m_context.compare_exchange_strong(expect, cp))
    {
      delete cp;
      throw exc::invalid_state();
    }

    auto type = AF_INET6;
    if (addr_.version() == ip::version::ipv4)
      type = AF_INET;

    auto handle = WSASocketW(type, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (handle == invalid_handle)
      throw exc::socket_failure();

    (*cp)->set_handle(cp, handle);
  }

  try
  {
    // TODO: is this needed to client sockets, too?
    if (addr_.version() == ip::version::ipv6)
    {
      const DWORD ipv6only = 0;
      if (setsockopt((*cp)->m_handle, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&ipv6only), sizeof(ipv6only)) == SOCKET_ERROR)
        throw exc::socket_failure();
    }

    // get address of ConnectEx function (argh!)
    {
      GUID guid = WSAID_CONNECTEX;
      DWORD filler;
      if (WSAIoctl(
            (*cp)->m_handle
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

    if (bind((*cp)->m_handle, pointer, size) != NO_ERROR)
      throw exc::socket_failure();

    // notify thread pool about incoming async operation
    StartThreadpoolIo((*cp)->m_tpio);

    // now set the address to remote and trigger connect request
    memset(&(*cp)->m_rd_overlapped, 0, sizeof((*cp)->m_rd_overlapped));
    if (addr_.version() == ip::version::ipv4)
      set_address(addr4, addr_, port_, pointer, size);
    else
      set_address(addr6, addr_, port_, pointer, size);

    if (!m_connect_ex(
          (*cp)->m_handle
        , pointer
        , size
        , nullptr
        , 0
        , nullptr
        , &(*cp)->m_rd_overlapped))
    {
      if (WSAGetLastError() != ERROR_IO_PENDING)
      {
        CancelThreadpoolIo((*cp)->m_tpio);
        throw exc::socket_failure();
      }
    }
  }
  catch (...)
  {
    if (cp != nullptr && m_context.compare_exchange_strong(cp, nullptr))
      delete cp;
    throw;
  }
}

void stream::start_cleanup(context::sptr *cp)
{
  if (!(*cp)->m_cleanup)
    return;

  auto ex = m_executor.lock();
  if (!ex)
    return;

  // signal the runner/poolmgr to clean up the stream
  std::atomic<bool> finished{false};
  std::condition_variable cv;
  std::mutex mutex;
  context::wptr wself = *cp;
  auto exe_ctx = new exec_for_cleanup(&(*cp)->m_environ,
    [wself, cp, &mutex, &cv, &finished]()
    {
      auto self = wself.lock();
      if (self)
        CloseThreadpoolCleanupGroupMembers((*cp)->m_cleanup, TRUE, cp);

      std::unique_lock<std::mutex> lock(mutex);
      finished = true;
      cv.notify_one();
    }
  );

  ex->run(exe_ctx);
  {
    std::unique_lock<std::mutex> lock(mutex);
    while (!finished)
      cv.wait(lock);

    delete cp;
  }
}

void stream::report_error(const detail::oob_event& event, const std::error_code& error_code)
{
  auto ex = m_executor.lock();
  if (!ex)
    return;

  auto handler = m_handler;
  auto exe_ctx = new exec_for_io(m_pool->get_environ(),
    [handler, event, error_code]()
    {
      auto cb = handler.lock();
      if (cb) try { cb->on_event(event, error_code); } catch (...) { }
    }
  );
  ex->run(exe_ctx);
}

// ---
// shutdown and disconnect are similar when in connected state. But shutdown
// has to handle other states, too

void stream::disconnect()
{
  bool is_connecting = false;

  switch (set_state(state::connected, state::disconnecting))
  {
    case state::disconnected:
    case state::disconnecting:
      TRACE(name(), "disconnect called in state 'disconnecting' or 'disconnected', nothing to do");
      return;   // already disconnecting or disconnected

    case state::connecting:
      TRACE(name(), "disconnect called in state 'connecting'");
      is_connecting = true;
      break;

    case state::connected:
      TRACE(name(), "disconnect called in state 'connected'");
      break;
  }

  auto cp = m_context.load();
  if (!m_context.compare_exchange_strong(cp, nullptr) || cp == nullptr)  // somebody else must have closed already
  {
    TRACE(name(), "other thread must have closed cleanup group already");
    return;
  }

  // close the socket so that IOCP stops
  auto h = (*cp)->m_handle;
  if (h != invalid_handle)
  {
    closesocket(h);
  }

  // disconnect() while connect is pending -> report failure
  if (is_connecting)
    report_error(detail::oob_event::failure, error::make_error_code(error::errc::request_aborted));

  // cancel all pending IO callbacks + prevent further ones
  WaitForThreadpoolIoCallbacks((*cp)->m_tpio, TRUE);
  // CancelThreadpoolIo((*cp)->m_tpio);


  TRACE(name(), "submitting work object to close context");
  start_cleanup(cp);
}

void stream::shutdown()
{
  TRACE(name(), "now shutting down");
  // in shutdown it's okay if it throws invalid_state; propagate other exceptions
  try  { disconnect(); } catch (const exc::invalid_state&) { /* noop */ std::cout << "******** invalid state\n"; }
  TRACE(name(), "shutdown complete");
}

void stream::write(const void* data, std::size_t size)
{
  auto cp = m_context.load();
  if (get_state() != state::connected)
    throw exc::invalid_state();

  bool expected = false;
  if (!(*cp)->m_wr_busy.compare_exchange_strong(expected, true))
    throw exc::operation_failed(cool::ng::error::errc::resource_busy);

  (*cp)->m_wr_data = static_cast<const uint8_t*>(data);
  (*cp)->m_wr_size = size;
  (*cp)->m_wr_pos = 0;
  start_write_source(cp);
}


void stream::start_read_source(context::sptr* cp)
{
  memset(&(*cp)->m_rd_overlapped, 0, sizeof((*cp)->m_rd_overlapped));
  DWORD size = static_cast<DWORD>((*cp)->m_rd_size);
  TRACE((*cp)->m_stream->name(), "starting read source: " << size);

  StartThreadpoolIo((*cp)->m_tpio);
  if (!ReadFile(reinterpret_cast<HANDLE>((*cp)->m_handle)
    , (*cp)->m_rd_data
    , size
    , nullptr
    , &(*cp)->m_rd_overlapped))
  {
    auto err = GetLastError();
    if (err != ERROR_IO_PENDING)
    {
      CancelThreadpoolIo((*cp)->m_tpio);
      switch (get_state())
      {
        case state::disconnecting:
        case state::disconnected:
        {
          // this failure and state mean that there was no active read operation
          // during external call to disconnect - cleanup the context and
          // consequently remove itself, too.
          if (m_context.compare_exchange_strong(cp, nullptr))
            delete cp;
          break;
        }

        default:
          if (m_context.compare_exchange_strong(cp, nullptr))
          {
            report_error(detail::oob_event::failure, translate_error(err));
            start_cleanup(cp);
          }
          break;
      }
    }
  }
}

void stream::start_write_source(context::sptr* cp)
{
  memset(&(*cp)->m_wr_overlapped, 0, sizeof((*cp)->m_wr_overlapped));

  // note really necessary. still ... internal sizes are std::size_t (64 bits)
  // while WriteFile works with 32 bits ... just in case somebody really
  // wanted to do write > 2G
  std::size_t full_size = (*cp)->m_wr_size - (*cp)->m_wr_pos;
  DWORD size = full_size > INT32_MAX ? INT32_MAX : static_cast<DWORD>(full_size);
  TRACE((*cp)->m_stream->name(), "starting write source: " << size);

  StartThreadpoolIo((*cp)->m_tpio);
  if (!WriteFile(
      reinterpret_cast<HANDLE>((*cp)->m_handle)
    , (*cp)->m_wr_data + (*cp)->m_wr_pos
    , size
    , nullptr
    , &(*cp)->m_wr_overlapped))
  {
    auto err = GetLastError();
    if (err != ERROR_IO_PENDING)
    {
      CancelThreadpoolIo((*cp)->m_tpio);
      return;
    }
    // any other failure means that handle was closed ... read operation will
    // take care about cleanup
  }
}


#if 0
void stream::process_connecting_event(ULONG io_result_)
{
  TRACE(name(), "processing connecting event begin");
  // notify user about connect
  auto cb = m_handler.lock();
  state expect = state::connecting;

  if (io_result_ == NO_ERROR)
  {
    if (cb)
    {
      if (set_state(state::connecting, state::connected) == state::connecting)
      {
        start_read_source();
        try { cb->on_event(detail::oob_event::connect, no_error()); } catch (...) { }
        TRACE(name(), "processing connecting event complete, new state 'connected'");
        return;  // upon success job done, return from the call
      }
    }  // if user callback processor no longer exists the below code will disconnect and clean up
  }

  if (cb)
  {
    try { cb->on_event(detail::oob_event::failure, translate_error(io_result_)); }
    catch (...) {}
  }

  auto ctx = m_context.load();
  if (ctx != nullptr && m_context.compare_exchange_strong(ctx, nullptr))
    CloseThreadpoolCleanupGroupMembers(ctx->m_cleanup, true, ctx);

  TRACE(name(), "processing connecting event complete, new state 'disconnected'");
}
#endif
#if 0
void stream::process_disconnect_event(const std::error_code& err_)
{
  TRACE(name(), "processing disconnect event begin");
  auto cb = m_handler.lock();

  if (cb)
  {
    try { cb->on_event(detail::oob_event::disconnect, err_); }
    catch (...) {}
  }

  auto ctx = m_context.load();
  if (ctx != nullptr && m_context.compare_exchange_strong(ctx, nullptr))
    CloseThreadpoolCleanupGroupMembers(ctx->m_cleanup, true, ctx);

  TRACE(name(), "processing disconnect event complete");
}

void stream::process_read_event(ULONG_PTR count_)
{
  auto ctx = m_context.load();
  // notify user about connect
  auto cb = m_handler.lock();
  if (cb)
  {
    auto data = ctx->m_rd_data;
    std::size_t size = count_;

    try
    {
      try { cb->on_read(data, size); } catch (...) { /* noop */ }

      // check if callback modified buffer or size parameters
      if (data != ctx->m_rd_data || size != ctx->m_rd_size)
      {
        // release current buffer if allocated by me
        if (ctx->m_rd_is_mine)
          delete [] static_cast<uint8_t*>(ctx->m_rd_data);

        // there are some special values that requeire different considerations
        //  - if size is zero revert to bufffer specified at the creation
        //  - if buf is nullptr allocate buffer of specified size
        if (size == 0)
        {
          ctx->m_rd_size = m_rd_size;
          ctx->m_rd_data = m_rd_data;
        }
        else
        {
          ctx->m_rd_data = data;
          ctx->m_rd_size = size;
        }
        ctx->m_rd_is_mine = ctx->m_rd_data == nullptr;

        if (ctx->m_rd_is_mine)
          ctx->m_rd_data = new uint8_t[ctx->m_rd_size];
      }
    }
    catch (...)
    { /* noop */ }
  }

  start_read_source();
}

void stream::process_write_event(ULONG_PTR count_)
{
  auto ctx = m_context.load();

  ctx->m_wr_pos += count_;
  if (ctx->m_wr_pos >= ctx->m_wr_size)
  {
    auto cb = m_handler.lock();
    ctx->m_wr_busy = false;
    if (cb)
      try { cb->on_write(ctx->m_wr_data, ctx->m_wr_size); } catch (...) { /* noop */ }
  }
  else
    start_write_source(cb);
}
#endif

// -- callback from the threadpool for i/o calls - this callback is a result
//    of StartThreadpoIo work object and was not submitted in the context of any
//    runner. To support synchronized semantics of event sources this callback
//    must, when issuing callbacks into user code, submit work to the executor
//    of the runner the event source is associated with. To do so it must create
//    an event work context and submit it to executor::run method.

void stream::process_event(context::sptr* cp_, PVOID overlapped_, ULONG io_result_, ULONG_PTR num_transferred_)
{
  TRACE(name(), "process_event: io_result: " << io_result_ << " size: " << num_transferred_ << " for " <<
      (overlapped_ == &(*cp_)->m_rd_overlapped ? " read " : (overlapped_ == nullptr ? " (nullptr)" : " write")));

  auto st = (*cp_)->m_state.load();

  if (overlapped_ == &(*cp_)->m_rd_overlapped)  // read related event
  {
    if (st == state::connecting)
      process_event_connecting(cp_, num_transferred_, io_result_);
    else
      process_event_read(cp_, num_transferred_, io_result_);
  }
  else if (overlapped_ == &(*cp_)->m_wr_overlapped)  // write related event
  {
    process_event_write(cp_, num_transferred_, io_result_);
  }
  else
  {
    TRACE(name(), "neither read nor write event detected");
  }
}

// -- Processing of the read event in the 'connecting' state.
void stream::process_event_connecting(context::sptr* cp_, ULONG_PTR num_transferred_, ULONG io_result_)
{
  TRACE(name(), "process connecting event, count: " << num_transferred_ << " result: " << io_result_);
  assert(num_transferred_ == 0);
  auto ex = m_executor.lock();

  switch (io_result_)
  {
    case NO_ERROR:
      if (ex)
      {
        // must be in connecting state to acknowledge connect - if not, state shift
        // might have happened in the mean time for some other reason, like shutdown
        if (set_state(state::connecting, state::connected) == state::connecting)
        {
          auto handler = m_handler;
          context::wptr wself = *cp_;
          auto exe_ctx = new exec_for_io(&(*cp_)->m_environ,
            [handler, wself, cp_]()
            {
              auto cb = handler.lock();
              if (cb)
              {
                try { cb->on_event(detail::oob_event::connect, no_error()); } catch (...) { }
                auto self = wself.lock();
                if (self)
                {
                  self->m_stream->start_read_source(cp_);
                }
              }
            }
          );
          ex->run(exe_ctx);
          TRACE(name(), "process connecting event complete, new state 'connected'");
        }
        break;
      }  // !!!!NOTE: fall through if !ex, but not if state wasn't 'connecting'

    default:
      if (m_context.compare_exchange_strong(cp_, nullptr))
      {
        report_error(detail::oob_event::failure, translate_error(io_result_));
        start_cleanup(cp_);
      }
      break;
  }
}

void stream::process_event_read(context::sptr* cp_, ULONG_PTR num_transferred_, ULONG io_result_)
{
  TRACE(name(), "process read event, count: " << num_transferred_ << " result: " << io_result_);

  auto ex = m_executor.lock();
  if (!ex)  //
  {
    // CHECKME: what do we do here? executor is down, so the cleanup work cannot
    // be submitted... it's also possible it already had been submitted (via disconnect())
    return;
  }

  if (io_result_ != NO_ERROR || num_transferred_ == 0)
  {
    switch (io_result_)
    {
      case NO_ERROR:
      case ERROR_NETNAME_DELETED:
      case ERROR_CONNECTION_ABORTED:
        report_error(detail::oob_event::disconnect, no_error());
        break;

      default:
        report_error(detail::oob_event::disconnect, std::error_code(io_result_, std::system_category()));
        break;
    }

    return;
  }


  auto handler = m_handler;
  auto capture_data = (*cp_)->m_rd_data;
  context::wptr wctx = *cp_;
  auto exe_ctx = new exec_for_io(&(*cp_)->m_environ,
    [handler, capture_data, num_transferred_, wctx, cp_]()
    {
      void* data = capture_data;
      std::size_t size = num_transferred_;
      auto ctx = wctx.lock();
      if (!ctx)
        return;

      auto cb = handler.lock();
      if (!cb)
      {
        // TODO: this is serious stuff, need to close the stream
        return;
      }
      try { cb->on_read(data, size); } catch (...) { }

      // check if user changed the buffere and restart read operation
      if (data != ctx->m_rd_data || size > ctx->m_rd_size)
      {
        // release current buffer if allocated by me
        if (ctx->m_rd_is_mine)
        {
          // if "data" points to our internal buffer, make sure it is invalidated,
          // so that it will be properly reallocated
          if (data == ctx->m_rd_data)
            data = nullptr;
          delete [] static_cast<uint8_t*>(ctx->m_rd_data);
        }

        // there are some special values that requeire different considerations
        //  - if size is zero revert to bufffer specified at the creation
        //  - if buf is nullptr allocate buffer of specified size
        if (size == 0)
        {
          ctx->m_rd_size = ctx->m_stream->m_rd_size;
          ctx->m_rd_data = ctx->m_stream->m_rd_data;
        }
        else
        {
          ctx->m_rd_data = data;
          ctx->m_rd_size = size;
        }
        ctx->m_rd_is_mine = ctx->m_rd_data == nullptr;

        try
        {
          if (ctx->m_rd_is_mine)
            ctx->m_rd_data = new uint8_t[ctx->m_rd_size];
        }
        catch (...)   // TODO: out-of-memory, shutdown the stream
        { }
      }
      ctx->m_stream->start_read_source(cp_);
    }
  );
  ex->run(exe_ctx);
}

void stream::process_event_write(context::sptr* cp_, ULONG_PTR num_transferred_, ULONG io_result_)
{
  TRACE(name(), "process write event, count: " << num_transferred_ << " result: " << io_result_);
  if (io_result_ != NO_ERROR)
  {
    return; // TODO: do we report error here???
  }

  assert(num_transferred_ > 0);

  (*cp_)->m_wr_pos += num_transferred_;
  if ((*cp_)->m_wr_pos >= (*cp_)->m_wr_size)  // write complete, remove busy flag and notify user
  {
    (*cp_)->m_wr_busy = false;
    auto ex = m_executor.lock();
    if (ex)
    {
      auto handler = m_handler;
      auto data = (*cp_)->m_wr_data;
      auto size = (*cp_)->m_wr_size;
      auto exe_ctx = new exec_for_io(&(*cp_)->m_environ,
        [handler, data, size]()
        {
          auto cb = handler.lock();
          if (cb) try { cb->on_write(data, size); } catch (...) { }
        }
      );
      ex->run(exe_ctx);
    }
  }
  else  // there's still some data to write, start write operation
  {
    start_write_source(cp_);
  }
}




#if 0
void stream::on_event(PVOID overlapped_, ULONG io_result_, ULONG_PTR num_transferred_)
{
  auto ctx = m_context.load();
  state expect;
  TRACE(name(), " io_result: " << io_result_ << " size: " << num_transferred_ << " for " <<
      (overlapped_ == &ctx->m_rd_overlapped ? " read " : (overlapped_ == nullptr ? " (nullptr)" : " write")));

  switch (get_state())
  {
    case state::connecting:
      process_connecting_event(io_result_);
      break;

    default:
      switch (io_result_)
      {
        case NO_ERROR:
          // right, figure out what is going on ...
          if (overlapped_ == &ctx->m_rd_overlapped)   // must be read or connect
          {
            {
              if (num_transferred_ == 0)
              {
                // -- sometimes caused by remote peer closing connection
                expect = state::connected;
                if (set_state(state::connected, state::disconnecting) == state::connected)
                  process_disconnect_event(no_error());
              }
              else
              {
                process_read_event(num_transferred_);
              }
            }
          }
          else if (overlapped_ == &ctx->m_wr_overlapped)
          {
            process_write_event(num_transferred_);
          }
          break;

        // -- sometimes caused by remote peer closing connection
        case ERROR_NETNAME_DELETED:    // error code 64
          if (set_state(state::connected, state::disconnecting) == state::connected)
          {
            TRACE(name(), "NETNAME_DELETED in state 'connected'");
            process_disconnect_event(no_error());
          }
 //         else if (expect == state::disconnecting || expect == state::disconnected)
 //         {
 //           process_disconnect_event(no_error());
 //         }
          break;

        // -- caused by shutdown() or disconnect
        case ERROR_CONNECTION_ABORTED:
          // TODO: FIXME
          if (m_context.compare_exchange_strong(ctx, nullptr))
            delete ctx;
          break;

        // -- check if failed connect request
        default:
          break;
      }
  }
}
#endif

} } } } }


