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

#if defined(WINDOWS_TARGET)
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
# include <sys/types.h>
# include <sys/socket.h>
#endif

#include <iostream>
#include <typeinfo>
#include <memory>
#include <array>
#include <stack>
#include <functional>
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <condition_variable>
#include <exception>


#define BOOST_TEST_MODULE NetworkEventSources
#include <boost/test/unit_test.hpp>

#include "cool/ng/bases.h"
#include "cool/ng/async.h"

// #include "test_server.h"

using ms = std::chrono::milliseconds;

#define DELAY 100000

BOOST_AUTO_TEST_SUITE(net_sources)

namespace net = cool::ng::net;
namespace ip = cool::ng::net::ip;
namespace ipv4 = cool::ng::net::ipv4;
namespace ipv6 = cool::ng::net::ipv6;
namespace async = cool::ng::async;

class test_runner : public cool::ng::async::runner
{
 public:
  test_runner()       { clear(); }
  void inc()          { ++m_counter; }
  void clear()        { m_counter = 0; }
  int counter() const { return m_counter; }

 private:
  int m_counter = 0;
};

void check_start_sockets()
{
#if defined(WINDOWS_TARGET)
  static bool wsa_started = false;

  if (!wsa_started)
  {
    WORD requested;
    WSADATA data;
    int err;

    // Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h
    requested = MAKEWORD(2, 2);

    WSAStartup(requested, &data);
  }
#endif
}

#if 0
// Tests:
//   - accept calls from the server
//   - both client-side ctors of the stream
BOOST_AUTO_TEST_CASE(accept_test)
{
  check_start_sockets();

  std::mutex m;
  std::condition_variable cv;
  int num_clients = 0;
  bool client1_connected = false;
  bool client2_connected = false;
  std::array<std::shared_ptr<async::net::stream>, 10> srv_stream;
  std::array<std::string, 10> srv_addr;

  auto r = std::make_shared<test_runner>();
  auto r2 = std::make_shared<test_runner>();

  auto server = async::net::server(
      std::weak_ptr<test_runner>(r)
    , ipv6::any
    , 22220
    , [&m, &cv, &srv_stream, &r2, &num_clients, &srv_addr](const std::shared_ptr<test_runner>& r
                          , const net::handle h
                          , const ip::address& a
                          , int p)
      {
        if (r)
          r->inc();

        std::cout << "Connect from [" << static_cast<std::string>(a) << "]:" << p << "\n";
        std::unique_lock<std::mutex> l(m);
        srv_stream[num_clients] = std::make_shared<async::net::stream>(
            std::weak_ptr<test_runner>(r2)
          , h
          , [] (const std::shared_ptr<test_runner>&, void*, std::size_t&)
            { }
          , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
            { }
          , [&num_clients, &m, &cv] (const std::shared_ptr<test_runner>& r, async::net::stream::oob_event evt)
            {
              if (r)
                r->inc();
              std::cout << "server received event " << static_cast<int>(evt) << "\n";
              switch (evt)
              {
                case async::net::stream::oob_event::disconnected:
                  --num_clients;
                  cv.notify_one();
                  break;

                default:
                  break;
              }
            }
        );
        srv_addr[num_clients] = static_cast<std::string>(a);

        ++num_clients;
        cv.notify_one();
        return true;
      }
  );

  server.start();

  {
    auto client = std::make_shared<async::net::stream>(
        std::weak_ptr<test_runner>(r2)
      , ipv4::loopback
      , 22220
      , [] (const std::shared_ptr<test_runner>&, void*&, std::size_t&)
        { }
      , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
        { }
      , [&m, &cv, &client1_connected] (const std::shared_ptr<test_runner>& r, async::net::stream::oob_event evt)
        {
          if (r)
            r->inc();

          std::unique_lock<std::mutex> l(m);
          std::cout << "client received event " << static_cast<int>(evt) << "\n";
          switch (evt)
          {
            case async::net::stream::oob_event::connected:
              client1_connected = true;
              cv.notify_one();
              break;

            case async::net::stream::oob_event::disconnected:
              client1_connected = false;
              cv.notify_one();
              break;

            default:
              break;
          }
        }
    );

    std::unique_lock<std::mutex> l(m);
    cv.wait_for(l, ms(500), [&client1_connected, &num_clients] () { return client1_connected && num_clients == 1; } );
    BOOST_CHECK_EQUAL(1, num_clients);
    BOOST_CHECK_EQUAL(true, client1_connected);
    BOOST_CHECK_EQUAL(1, r->counter());
    BOOST_CHECK_EQUAL(1, r2->counter());

    {
      auto client2 = std::make_shared<async::net::stream>(
          std::weak_ptr<test_runner>(r2)
        , [] (const std::shared_ptr<test_runner>&, void*&, std::size_t&)
          { }
        , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
          { }
        , [&m, &cv, &client2_connected] (const std::shared_ptr<test_runner>& r, async::net::stream::oob_event evt)
          {
            if (r)
              r->inc();

            std::unique_lock<std::mutex> l(m);
            std::cout << "client received event " << static_cast<int>(evt) << "\n";
            switch (evt)
            {
              case async::net::stream::oob_event::connected:
                client2_connected = true;
                cv.notify_one();
                break;

              case async::net::stream::oob_event::disconnected:
                client2_connected = false;
                cv.notify_one();
                break;

              default:
                std::cout << "**** error reported for connect\n";
                break;
            }
          }
      );

      client2->connect(cool::ng::net::ipv6::loopback, 22220);

      std::cout << "client 1 name: " << client->name() << "\n";
      std::cout << "client 2 name: " << client2->name() << "\n";

      cv.wait_for(l, ms(500), [&client2_connected, &num_clients] () { return client2_connected && num_clients == 2; } );
      BOOST_CHECK_EQUAL(2, num_clients);
      BOOST_CHECK_EQUAL(true, client2_connected);
      BOOST_CHECK_EQUAL(2, r->counter());
      BOOST_CHECK_EQUAL(2, r2->counter());

      std::this_thread::sleep_for(ms(50));
    }

    cv.wait_for(l, ms(500), [&num_clients] () { return num_clients == 1; } );
    BOOST_CHECK_EQUAL(1, num_clients);
    BOOST_CHECK_EQUAL(2, r->counter());
    BOOST_CHECK_EQUAL(3, r2->counter());

    std::this_thread::sleep_for(ms(50));

  }

  std::unique_lock<std::mutex> l(m);
  cv.wait_for(l, ms(500), [&num_clients] () { return num_clients == 0; } );
  BOOST_CHECK_EQUAL(0, num_clients);
  BOOST_CHECK_EQUAL(true, client2_connected);
  BOOST_CHECK_EQUAL(true, client1_connected);
  BOOST_CHECK_EQUAL(2, r->counter());
  BOOST_CHECK_EQUAL(4, r2->counter());

  BOOST_CHECK_EQUAL("::ffff:127.0.0.1", srv_addr[0]);
  BOOST_CHECK_EQUAL("::1", srv_addr[1]);
}
#endif
#if 0
BOOST_AUTO_TEST_CASE(read_write_test_1)
{
  uint8_t buffer[2500000];

  check_start_sockets();

  std::mutex m;
  std::condition_variable cv;
  bool client_connected = false;
  bool srv_connected = false;
  std::size_t srv_size = 0;
  std::size_t clt_size = 0;
  bool srv_written = false;
  bool clt_written = false;

  std::shared_ptr<async::net::stream> srv_stream;

  auto r = std::make_shared<test_runner>();
  auto r2 = std::make_shared<test_runner>();

  auto server = async::net::server(
      std::weak_ptr<test_runner>(r)
    , ipv6::any
    , 22228
    , [&m, &cv, &srv_stream, &r2, &srv_connected, &srv_size, &buffer, &srv_written](const std::shared_ptr<test_runner>& r
                          , const net::handle h
                          , const ip::address& a
                          , int p)
      {
        if (r)
          r->inc();

        std::cout << "Connect from [" << static_cast<std::string>(a) << "]:" << p << "\n";
        std::unique_lock<std::mutex> l(m);
         srv_stream = std::make_shared<async::net::stream>(
            std::weak_ptr<test_runner>(r2)
          , h
          , [&srv_size, &m, &cv, &buffer] (const std::shared_ptr<test_runner>&, void*&, std::size_t& size_)
            {
              srv_size += size_;
              if (srv_size >= sizeof(buffer))
              {
                std::unique_lock<std::mutex> l(m);
                cv.notify_one();
              }
            }
          , [&srv_written, &m, &cv, &srv_stream] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
            {
              std::unique_lock<std::mutex> l(m);
              srv_written = true;
              cv.notify_one();
            }
          , [&srv_connected, &m, &cv] (const std::shared_ptr<test_runner>&, async::net::stream::oob_event evt)
            {
              std::cout << "server received event " << static_cast<int>(evt) << "\n";
              switch (evt)
              {
                case async::net::stream::oob_event::connected:
                  srv_connected = true;
                  cv.notify_one();
                  break;

                case async::net::stream::oob_event::disconnected:
                  srv_connected = false;
                  cv.notify_one();
                  break;

                default:
                  break;
              }
            }
        );

        srv_connected = true;
        cv.notify_one();
        return true;
      }
  );

  server.start();

  {
    auto client = std::make_shared<async::net::stream>(
        std::weak_ptr<test_runner>(r2)
      , ipv4::loopback
      , 22228
      , [&clt_size, &m, &cv, &buffer] (const std::shared_ptr<test_runner>&, void*&, std::size_t& size)
        {
          clt_size += size;
          if (clt_size >= sizeof(buffer))
          {
            std::unique_lock<std::mutex> l(m);
            cv.notify_one();
          }
        }
      , [&clt_written, &m, &cv] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
        {
            std::unique_lock<std::mutex> l(m);
            clt_written = true;;
            cv.notify_one();
        }
      , [&m, &cv, &client_connected] (const std::shared_ptr<test_runner>& r, async::net::stream::oob_event evt)
        {
          if (r)
            r->inc();

          std::unique_lock<std::mutex> l(m);
          std::cout << "client received event " << static_cast<int>(evt) << "\n";
          switch (evt)
          {
            case async::net::stream::oob_event::connected:
              client_connected = true;
              cv.notify_one();
              break;

            case async::net::stream::oob_event::disconnected:
              client_connected = false;
              cv.notify_one();
              break;

            default:
              break;
          }
        }
    );

    std::unique_lock<std::mutex> l(m);
    cv.wait_for(l, ms(500), [&client_connected, &srv_connected]() { return client_connected && srv_connected; } );
    BOOST_CHECK_EQUAL(true, srv_connected);
    BOOST_CHECK_EQUAL(true, !!srv_stream);
    BOOST_CHECK_EQUAL(true, client_connected);
    BOOST_CHECK_EQUAL(1, r->counter());
    BOOST_CHECK_EQUAL(1, r2->counter());

    std::cout << "server stream: " << srv_stream->name() << "\n";
    std::cout << "client stream: " << client->name() << "\n";

    client->write(buffer, sizeof(buffer));
    srv_stream->write(buffer, sizeof(buffer));

    cv.wait_for(l, ms(1000), [&clt_size, &srv_size, &buffer, &srv_written, &clt_written]()
      { return clt_written && srv_written &&
               clt_size >= sizeof(buffer) && srv_size >= sizeof(buffer);

      }
    );
    BOOST_CHECK_EQUAL(sizeof(buffer), srv_size);
    BOOST_CHECK_EQUAL(sizeof(buffer), clt_size);
    BOOST_CHECK_EQUAL(true, srv_written);
    BOOST_CHECK_EQUAL(true, clt_written);

  }

  std::unique_lock<std::mutex> l(m);
  cv.wait_for(l, ms(500), [&client_connected]() { return !client_connected; } );
  cv.wait_for(l, ms(500), [&srv_connected]() { return !srv_connected; } );

  BOOST_CHECK_EQUAL(false, srv_connected);
  BOOST_CHECK_EQUAL(true, client_connected);
  BOOST_CHECK_EQUAL(1, r->counter());
  BOOST_CHECK_EQUAL(1, r2->counter());
}
#endif

#if 0
BOOST_AUTO_TEST_CASE(connect_disconnect_connect_disconnect_test)
{
  check_start_sockets();

  std::mutex m;
  std::condition_variable cv;
  bool srv_connected = false;
  bool clt_connected = false;
  std::shared_ptr<async::net::stream> srv_stream;
  std::string srv_peer_addr;

  auto r = std::make_shared<test_runner>();
  auto r2 = std::make_shared<test_runner>();

  auto server = async::net::server(
      std::weak_ptr<test_runner>(r)
    , ipv6::any
    , 22229
    , [&m, &cv, &srv_stream, &r2, &srv_peer_addr, &srv_connected](const std::shared_ptr<test_runner>& r
                          , const net::handle h
                          , const ip::address& a
                          , int p)
      {
        std::unique_lock<std::mutex> l(m);
        srv_stream = std::make_shared<async::net::stream>(
            std::weak_ptr<test_runner>(r2)
          , h
          , [] (const std::shared_ptr<test_runner>&, void*, std::size_t&)
            { }
          , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
            { }
          , [ &m, &cv, &srv_connected, &srv_stream] (const std::shared_ptr<test_runner>& r, async::net::stream::oob_event evt)
            {
              if (r)
                r->inc();
              std::cout << "server received event " << static_cast<int>(evt) << "\n";
              switch (evt)
              {
                case async::net::stream::oob_event::disconnected:
                  srv_connected = false;
                  srv_stream.reset();
                  cv.notify_one();
                  break;

                default:
                  break;
              }
            }
        );
        srv_peer_addr = static_cast<std::string>(a);
        srv_connected = true;
        cv.notify_one();
        return true;
      }
  );

  server.start();

  async::net::stream client(
      std::weak_ptr<test_runner>(r2)
    , [] (const std::shared_ptr<test_runner>&, void*&, std::size_t&)
      { }
    , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
      { }
    , [&m, &cv, &clt_connected] (const std::shared_ptr<test_runner>& r, async::net::stream::oob_event evt)
      {
        if (r)
          r->inc();

        std::unique_lock<std::mutex> l(m);
        std::cout << "client received event " << static_cast<int>(evt) << "\n";
        switch (evt)
        {
          case async::net::stream::oob_event::connected:
            clt_connected = true;
            cv.notify_one();
            break;

          case async::net::stream::oob_event::disconnected:
            clt_connected = false;
            cv.notify_one();
            break;

          default:
            break;
        }
      }
  );

  client.connect(cool::ng::net::ipv6::loopback, 22229);

  std::unique_lock<std::mutex> l(m);
  cv.wait_for(l, ms(500), [&clt_connected, &srv_connected] () { return clt_connected && srv_connected; } );
  BOOST_CHECK_EQUAL("::1", srv_peer_addr);
  BOOST_CHECK_EQUAL(true, clt_connected);
  BOOST_CHECK_EQUAL(true, srv_connected);
  BOOST_CHECK_EQUAL(true, !!srv_stream);
  BOOST_CHECK_THROW(client.connect(cool::ng::net::ipv6::loopback, 22229), cool::ng::exception::invalid_state);

  client.disconnect();
  cv.wait_for(l, ms(500), [&clt_connected, &srv_connected] () { return clt_connected && !srv_connected; } );
  BOOST_CHECK_EQUAL(true, clt_connected);
  BOOST_CHECK_EQUAL(false, srv_connected);
  BOOST_CHECK_EQUAL(false, !!srv_stream);

  clt_connected = false;
  client.connect(cool::ng::net::ipv4::loopback, 22229);
  cv.wait_for(l, ms(500), [&clt_connected, &srv_connected] () { return clt_connected && srv_connected; } );
  BOOST_CHECK_EQUAL("::ffff:127.0.0.1", srv_peer_addr);
  BOOST_CHECK_EQUAL(true, clt_connected);
  BOOST_CHECK_EQUAL(true, srv_connected);
  BOOST_CHECK_EQUAL(true, !!srv_stream);

  client.disconnect();
}
#endif

BOOST_AUTO_TEST_CASE(ctor_failures)
{
  auto r = std::make_shared<test_runner>();
  std::weak_ptr<test_runner> wr = r;

// TODO: handle std::bad_alloc and test
#if 0
  BOOST_CHECK_THROW(
      async::net::stream client(
          std::weak_ptr<test_runner>(r)
        , cool::ng::net::ipv6::loopback
        , 12345
        , [] (const std::shared_ptr<test_runner>&, void*&, std::size_t&)
          { }
        , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
          { }
        , [] (const std::shared_ptr<test_runner>& r, async::net::stream::oob_event evt)
          {}
        , nullptr
        , 50000000000
      )
    , std::exception
  );
#endif

}

#if 1
BOOST_AUTO_TEST_CASE(failed_connect_ctor)
{

// linux loopback connect will fails immediatelly
#if defined(LINUX_TARGET)
  auto r = std::make_shared<test_runner>();
  BOOST_CHECK_THROW(
      async::net::stream client(
          std::weak_ptr<test_runner>(r)
        , cool::ng::net::ipv6::loopback
        , 12345
        , [] (const std::shared_ptr<test_runner>&, void*&, std::size_t&)
          { }
        , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
          { }
        , [] (const std::shared_ptr<test_runner>& r, async::net::stream::oob_event evt)
          {}
      )
    , cool::ng::exception::socket_failure
  );
#endif

}
#endif


BOOST_AUTO_TEST_SUITE_END()




