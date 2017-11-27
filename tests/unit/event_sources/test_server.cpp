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

#if defined(WINDOWS_TARGET)

# include <Winsock2.h>
# define CLOSE_(a) ::closesocket(a)
# define SEND_(a, b, c, d) ::send(a, static_cast<const char*>(b), static_cast<int>(c), d)

#else

# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# define CLOSE_(a) ::close(a)
# define SEND_(a, b, c, d) ::send(a, b, c, d)

#endif

#include "cool/ng/bases.h"
#include "cool/ng/async.h"
#include "cool/ng/exception.h"

#include "test_server.h"

#define DBG(a) do { std::cout << a << "\n"; } while (false)

namespace test
{

namespace exc = cool::ng::exception;

test_server::ptr test_server::create(
    int port
  , const callback& on_accept
  , const receive_ready& on_receive
  , const callback on_write_complete
  , const callback& on_disconnect)
{
  ptr res = cool::ng::util::shared_new<test_server>(port);

  res->m_on_accept = on_accept;
  res->m_on_receive = on_receive;
  res->m_on_write = on_write_complete;
  res->m_on_disconnect = on_disconnect;

  res->start();

  return res;
}

test_server::test_server(int port) : m_port(port), m_listen(io::invalid_handle)
{ /* noop */ }

void test_server::start()
{
#if (!defined WINDOWS_TARGET)
  using SOCKADDR = struct sockaddr;
#endif

  try
  {

    m_listen = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_listen == io::invalid_handle)
      throw exc::operation_failed("failed to allocate listen socket");

    {
  #if defined(WINDOWS_TARGET)
      const bool enable = true;
  #else
      const int enable = 1;
  #endif
      if (::setsockopt(m_listen, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enable), sizeof(enable)) != 0)
        throw exc::operation_failed("failed to setsockopt");
    }

    {
      sockaddr_in addr;
      std::memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = INADDR_ANY;
      addr.sin_port = htons(m_port);

      if (::bind(m_listen, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) != 0)
        throw exc::operation_failed("bind call failed");

      if (::listen(m_listen, 10) != 0)
        throw exc::operation_failed("listen call failed");
    }

    m_acceptor.reset(new async::reader(
        self()
      , m_listen
      , [] (const test_server::weak_ptr& me_, io::handle fd_, std::size_t size_) -> void
        {
          DBG("master socket " << fd_ << ": read event, size " << size_);
          auto self = me_.lock();
          if (self)
            self->on_connect(fd_, size_);
        }
      , [] (const test_server::weak_ptr& me_, io::handle fd_) -> void
        {
          DBG("master socket " << fd_ << ": cancel event");
          CLOSE_(fd_);
        }
    ));

    // !!! do not forget to start reader
    m_acceptor->start();
  }
  catch (...)
  {
    if (m_listen != io::invalid_handle)
    {
      CLOSE_(m_listen);
      m_listen = io::invalid_handle;
    }
    throw;
  }
}

void test_server::on_connect(io::handle fd_, std::size_t size_)
{
  if (size_ == 0)
    return;

  for (std::size_t i = 0; i < size_; ++i)
  {
    try
    {
      io::handle sock = ::accept(m_listen, nullptr, nullptr);
      if (sock == io::invalid_handle)
        continue;

      DBG("new client socket " << sock);
      std::unique_ptr<async::reader> clt(new async::reader(
          self()
        , sock
        , [] (test_server::weak_ptr me_, io::handle fd_, std::size_t size_) -> void
          {
            DBG("client socket " << fd_ << ": read event, size " << size_);
            auto self = me_.lock();
            if (self)
            {
              if (size_ == 0)
              {
                if (self->m_on_disconnect)
                  self->m_on_disconnect();
                self->m_clients.erase(fd_);
              }
              else
              {
                if (self->m_on_receive)
                  self->m_on_receive(fd_, size_);
              }
            }
          }
      , [] (test_server::weak_ptr me_, io::handle fd_) -> void
        {
          DBG("client socket " << fd_ << ": cancel event");
          CLOSE_(fd_);
        }
      ));

      clt->start();
      m_clients.insert(client_map::value_type(sock, std::move(clt)));
      if (m_on_accept)
        try { m_on_accept(); } catch (...) { }
    }
    catch (...)
    { /* noop */ }
  }
}

// ------
test_client::ptr test_client::create(
    int port
  , const receive_ready& on_receive
  , const callback on_write_complete)
{
  ptr res = cool::ng::util::shared_new<test_client>(port);

  res->m_on_receive = on_receive;
  res->m_on_write = on_write_complete;

  res->start();

  return res;
}

test_client::test_client(int port) : m_port(port), m_handle(io::invalid_handle)
{ /* noop */ }

void test_client::start()
{
#if (!defined WINDOWS_TARGET)
  using SOCKADDR = struct sockaddr;
#endif

  try
  {
    m_handle = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_handle == io::invalid_handle)
      throw exc::operation_failed("failed to allocate socket");

    {
      struct sockaddr_in addr;
      std::memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      addr.sin_port = htons(m_port);

      if (::connect(m_handle, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) != 0)
      {
        CLOSE_(m_handle);
        throw exc::operation_failed("connect failed");
      }
    }

    m_socket.reset(new async::reader(
        self()
      , m_handle
      , static_cast<std::function<void(test_client::weak_ptr, io::handle, std::size_t)>>(
          [] (test_client::weak_ptr me_, io::handle fd_, std::size_t size_) -> void
          {
            auto self = me_.lock();
            if (self)
              if (self->m_on_receive)
                self->m_on_receive(fd_, size_);
          }
        )
      , static_cast<std::function<void(test_client::weak_ptr, io::handle)>>(
          [] (test_client::weak_ptr me_, io::handle fd_) -> void
          {
            std::cout << "closing client socket " << fd_ << "\n";
            CLOSE_(fd_);
          }
        )
    ));

    m_socket->start();
  }
  catch (...)
  {
    if (m_handle != io::invalid_handle)
    {
      CLOSE_(m_handle);
      m_handle = io::invalid_handle;
    }
    throw;
  }
}

void test_client::write(int context_, const void *data_, std::size_t size_)
{
  // TODO: use writer
  SEND_(m_handle, data_, size_, 0);
}

}


