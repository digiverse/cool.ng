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
BOOST_AUTO_TEST_CASE(accept_test)
{
  check_start_sockets();

  std::mutex m;
  std::condition_variable cv;
  bool client_connected = false;
  bool srv_connected = false;
  std::shared_ptr<async::net::stream> srv_stream;

  auto r = std::make_shared<test_runner>();
  auto r2 = std::make_shared<test_runner>();

  auto server = async::net::server(
      std::weak_ptr<test_runner>(r)
    , ipv6::any
    , 22220
    , [&m, &cv, &srv_stream, &r2, &srv_connected](const std::shared_ptr<test_runner>& r
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
          , [] (const std::shared_ptr<test_runner>&, void*, std::size_t&)
            { }
          , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
            { }
          , [&srv_connected, &m, &cv] (const std::shared_ptr<test_runner>& r, async::net::stream::oob_event evt)
            {
              if (r)
                r->inc();
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
      , 22220
      , [] (const std::shared_ptr<test_runner>&, void*&, std::size_t&)
        { }
      , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
        { }
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
    cv.wait_for(l, ms(1000000), [&client_connected, &srv_connected]() { return client_connected && srv_connected; } );
    BOOST_CHECK_EQUAL(true, srv_connected);
    BOOST_CHECK_EQUAL(true, !!srv_stream);
    BOOST_CHECK_EQUAL(true, client_connected);
    BOOST_CHECK_EQUAL(1, r->counter());
    BOOST_CHECK_EQUAL(1, r2->counter());

    std::this_thread::sleep_for(ms(100));  // 30 secs
  }

  std::unique_lock<std::mutex> l(m);
  cv.wait_for(l, ms(100), [&client_connected]() { return !client_connected; } );
  cv.wait_for(l, ms(100), [&srv_connected]() { return !srv_connected; } );
  std::this_thread::sleep_for(ms(100));  // 30 secs

  BOOST_CHECK_EQUAL(false, srv_connected);
  BOOST_CHECK_EQUAL(true, client_connected);
  BOOST_CHECK_EQUAL(1, r->counter());
  BOOST_CHECK_EQUAL(2, r2->counter());
}
#endif

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
          , [&srv_written, &m, &cv] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
            {
              std::unique_lock<std::mutex> l(m);
              srv_written = true;;
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
    cv.wait_for(l, ms(100), [&client_connected, &srv_connected]() { return client_connected && srv_connected; } );
    BOOST_CHECK_EQUAL(true, srv_connected);
    BOOST_CHECK_EQUAL(true, !!srv_stream);
    BOOST_CHECK_EQUAL(true, client_connected);
    BOOST_CHECK_EQUAL(1, r->counter());
    BOOST_CHECK_EQUAL(1, r2->counter());

    std::cout << "server stream: " << srv_stream->name() << "\n";
    std::cout << "client stream: " << client->name() << "\n";

    client->write(buffer, sizeof(buffer));
    srv_stream->write(buffer, sizeof(buffer));

    cv.wait_for(l, ms(1000000), [&clt_size, &srv_size, &buffer, &srv_written, &clt_written]()
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
  cv.wait_for(l, ms(100), [&client_connected]() { return !client_connected; } );
  cv.wait_for(l, ms(100), [&srv_connected]() { return !srv_connected; } );

  BOOST_CHECK_EQUAL(false, srv_connected);
  BOOST_CHECK_EQUAL(true, client_connected);
  BOOST_CHECK_EQUAL(1, r->counter());
  BOOST_CHECK_EQUAL(1, r2->counter());
}

BOOST_AUTO_TEST_SUITE_END()




































#if 0
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

std::size_t read_data(cool::ng::io::handle fd_, std::size_t size)
{
  char arr[200];
  return recv(fd_, arr, sizeof(arr), 0);
}

BOOST_AUTO_TEST_CASE(reader_basic)
{
  std::mutex m;
  std::condition_variable cv;
  bool client_connected = false;
  std::size_t received_data = 0;

  auto server = test::test_server::create(
      18576
      // on accept
    , [&m, &cv, &client_connected]()  // on_accept
      {
        std::unique_lock<std::mutex> l(m);
        client_connected = true;
        cv.notify_one();
      }
      // on receive
    , [&m, &cv, &received_data](cool::ng::io::handle fd_, std::size_t size_)
      {
        std::unique_lock<std::mutex> l(m);
        received_data += read_data(fd_, size_);
        if (received_data >= 1024)
          cv.notify_one();
      }
    , [](){}  // on_write_complete
      // on disconnect
    , [&m, &cv, &client_connected]()
      {
        std::unique_lock<std::mutex> l(m);
        client_connected = false;
        cv.notify_one();
      }
  );

  std::unique_lock<std::mutex> l(m);

  auto client = test::test_client::create(
      18576
    , [](cool::ng::io::handle fd_, std::size_t){} // on_receive
    , [](){}  // on_write_complete
  );

  cv.wait_for(l, ms(DELAY), [&client_connected]() { return client_connected; });
  BOOST_CHECK_EQUAL(true, client_connected);
  std::cout << "Connected\n";

  // at this point client is connected, now send some data
  {
    unsigned char data[1024];
    client->write(42, data, sizeof(data));
  }
  cv.wait_for(l, ms(DELAY), [&received_data]() { return received_data != 0; });
  BOOST_CHECK_EQUAL(1024, received_data);
  std::cout << "Received " << received_data << " bytes of data\n";

  // disconnect client and wait for server to notice
  client.reset();

  cv.wait_for(l, ms(DELAY), [&client_connected]() { return !client_connected; });
  std::cout << "Disconnected\n";

  BOOST_CHECK_EQUAL(false, client_connected);
}
#endif
