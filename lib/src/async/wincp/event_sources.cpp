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

#include "cool/ng/exception.h"

#include "event_sources.h"

#pragma comment(lib, "Ws2_32.lib")

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

cool::ng::async::detail::startable* create_server(
    const std::shared_ptr<runner>& r_
  , const ip::address& addr_
  , int port_
  , const cb_server::weak_ptr& cb_)
{
  return new server(r_->impl(), addr_, port_, cb_);
}

server::server(const std::shared_ptr<async::impl::executor>& ex_
             , const cool::ng::net::ip::address& addr_
             , int port_
             , const cb_server::weak_ptr& cb_)
  : m_executor(ex_)
  , m_pool(async::impl::poolmgr::get_poolmgr())
  , m_handle(cool::ng::net::invalid_handle)
  , m_active(false)
  , m_handler(cb_)
  , m_accept_ex(nullptr)
  , m_get_sock_addrs(nullptr)
  , m_tp_io(nullptr)
  , m_client_handle(cool::ng::net::invalid_handle)
{
  try
  {
    // ----
    // create and bind listen socket
    {
      auto type = AF_INET6;
      if (addr_.version() == ip::version::ipv4)
        type = AF_INET;

      m_handle = WSASocketW(type, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
      if (m_handle == ::cool::ng::net::invalid_handle)
        throw exc::operation_failed("failed to allocate listen socket");

      const int enable = 1;
      if (setsockopt(m_handle, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enable), sizeof(enable)) == SOCKET_ERROR)
        throw exc::operation_failed("failed to setsockopt");

      if (addr_.version() == ip::version::ipv6)
      {
        const DWORD ipv6only = 0;
        if (setsockopt(m_handle, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&ipv6only), sizeof(ipv6only)) == SOCKET_ERROR)
          throw exc::operation_failed("failed to setsockopt");
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
        throw exc::operation_failed("bind call failed");

      if (listen(m_handle, 10) == SOCKET_ERROR)
        throw exc::operation_failed("listen call failed");
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
    {
      closesocket(m_handle);
      throw exc::operation_failed("failed to load AcceptEx");
    }

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
    {
      closesocket(m_handle);
      throw exc::operation_failed("failed to load AcceptEx");
    }

    // ----
    // Create thread pool i/o completion object and associate it with
    // the listen socket
    m_tp_io = CreateThreadpoolIo(
        reinterpret_cast<HANDLE>(m_handle)
      , server::on_accept
      , this
      , m_pool->get_environ());
    if (m_tp_io == nullptr)
    {
      closesocket(m_handle);
      throw exc::operation_failed("failed to create thread pool i/o object");
    }
  }
  catch (...)
  {
    if (m_handle != cool::ng::net::invalid_handle)
      closesocket(m_handle);
    if (m_tp_io != nullptr)
      CancelThreadpoolIo(m_tp_io);

    throw;
  }
}

void server::start_accept()
{
  auto m_client_handle = ::WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
  if (m_client_handle == ::cool::ng::net::invalid_handle)
    throw exc::operation_failed("failed to allocate accept socket");

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
    auto hr = HRESULT_FROM_WIN32(WSAGetLastError());
    if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
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
  if (m_handle != cool::ng::net::invalid_handle)
    closesocket(m_handle);
  m_handle = cool::ng::net::invalid_handle;
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
  exec_for_accept(const cb_server::weak_ptr& cb_
                , cool::ng::net::handle h_
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
  cool::ng::net::handle m_handle;
  cb_server::weak_ptr   m_handler;
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

    if (io_result_ == 0)
      ctx->process_accept();
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

} } } } }


