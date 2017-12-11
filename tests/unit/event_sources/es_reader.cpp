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
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

#define DELAY 100000

BOOST_AUTO_TEST_SUITE(net_sources)

namespace net = cool::ng::net;
namespace ip = cool::ng::net::ip;
namespace ipv4 = cool::ng::net::ipv4;
namespace ipv6 = cool::ng::net::ipv6;
namespace async = cool::ng::async;
using cool::ng::async::net::detail::oob_event;

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

    // Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h
    requested = MAKEWORD(2, 2);

    WSAStartup(requested, &data);
  }
#endif
}

template <typename RunnerT>
std::shared_ptr<async::net::stream> create_client(const std::shared_ptr<test_runner>& r, int msec)
{
  auto clt = std::make_shared<async::net::stream>(
      std::weak_ptr<test_runner>(r)
    , [] (const std::shared_ptr<test_runner>&, void*&, std::size_t&)
      { }
    , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
      { }
    , [] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
      { }
  );

  return clt;
}

template <typename RunnerT>
std::shared_ptr<async::net::stream> create_connected_client(const std::shared_ptr<test_runner>& r, const ip::address& addr, uint16_t port, int msec)
{
  std::mutex m;
  std::condition_variable cv;
  bool connected = false;

  auto clt = std::make_shared<async::net::stream>(
      std::weak_ptr<test_runner>(r)
    , addr
    , port
    , [] (const std::shared_ptr<test_runner>&, void*&, std::size_t&)
      { }
    , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
      { }
    , [&m, &cv, &connected] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
      {
        std::unique_lock<std::mutex> l(m);
        switch (evt)
        {
          case oob_event::connect:
            connected = true;
            cv.notify_one();
            break;

          default:
            break;
        }
      }
  );

  std::unique_lock<std::mutex> l(m);
  cv.wait_for(l, ms(msec), [&connected] () { return connected; } );
  if (!connected)
    clt.reset();
  return clt;
}

async::net::stream stream_factory(
    const std::shared_ptr<test_runner>& r_
  , const ip::address&
  , uint16_t
  , const std::weak_ptr<test_runner>& stream_r_
  , const async::net::detail::types<test_runner>::read_handler& rh_
  , const async::net::detail::types<test_runner>::write_handler& wh_
  , const async::net::detail::types<test_runner>::event_handler& eh_)
{
    return async::net::stream(stream_r_, rh_, wh_, eh_);
}

#if 1
// Just creates and destroys server to check that everything completes
// gracefully.
BOOST_AUTO_TEST_CASE(server_dtor)
{
  check_start_sockets();

  std::mutex m;
  std::condition_variable cv;
  int num_clients = 0;
  async::net::stream srv_stream;

  auto r = std::make_shared<test_runner>();
  auto r2 = std::make_shared<test_runner>();

  {
    auto server = async::net::server(
        std::weak_ptr<test_runner>(r)
      , ipv6::any
      , 22220
      , std::bind(stream_factory, _1, _2, _3, r2
          , [] (const std::shared_ptr<test_runner>&, void*, std::size_t&)
            { }
          , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
            { }
          , [&num_clients, &m, &cv] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
            {
              if (r)
                r->inc();
              std::cout << "server received event " << static_cast<int>(evt) << "\n";
              switch (evt)
              {
                case oob_event::disconnect:
                  --num_clients;
                  cv.notify_one();
                  break;

                default:
                  break;
              }
            }
        )
      , [&m, &cv, &srv_stream, &r2, &num_clients](const std::shared_ptr<test_runner>& r, const async::net::stream& s_)
        {
          if (r)
            r->inc();

          srv_stream = s_;
          ++num_clients;
          cv.notify_one();

          std::this_thread::sleep_for(ms(1000));

          if (num_clients % 2 == 1)
            srv_stream = async::net::stream();
          else
            srv_stream.disconnect();
          std::cout << "num clients: " << num_clients << "\n";
        }
    );

    server.start();
    std::this_thread::sleep_for(ms(100));
  }
  std::this_thread::sleep_for(ms(100));
  BOOST_CHECK_EQUAL(true, true);
}
#endif

// test handling of errors
#if 1
BOOST_AUTO_TEST_CASE(server_errors)
{
 check_start_sockets();
  std::shared_ptr<async::net::server> server1;

  {
    // no runner
    auto r = std::make_shared<test_runner>();
    std::weak_ptr<test_runner> wr(r);
    r.reset();
    BOOST_CHECK_THROW(server1 = std::make_shared<async::net::server>(
          wr
        , ipv4::any
        , 22211
        , std::bind(stream_factory, _1, _2, _3, r
            , [] (const std::shared_ptr<test_runner>&, void*, std::size_t&) { }
            , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t) { }
            , [] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e) { }
          )
        , [] (const std::shared_ptr<test_runner>& r, const async::net::stream& s_) { }
      )
    , cool::ng::exception::runner_not_available);
  }
  {
    // want bind to fail ... use netstat -an to find busy listen socket
    auto r = std::make_shared<test_runner>();
    std::weak_ptr<test_runner> wr(r);

    uint16_t port =
#if defined(WINDOWS_TARGET)
        1025;
#else
        22;
#endif
    BOOST_CHECK_THROW(server1 = std::make_shared<async::net::server>(
          wr
        , ipv4::any
        , port
        , std::bind(stream_factory, _1, _2, _3, r
            , [] (const std::shared_ptr<test_runner>&, void*, std::size_t&) { }
            , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t) { }
            , [] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e) { }
          )
        , [] (const std::shared_ptr<test_runner>& r, const async::net::stream& s_) { }
      )
    , cool::ng::exception::socket_failure);
  }
}
#endif
#if 1
// Tests:
//   - accept calls from the server
//   - both client-side ctors of the stream
BOOST_AUTO_TEST_CASE(accept_test)
{
  check_start_sockets();

  std::array<std::string, 2> srv_addr;
  auto r = std::make_shared<test_runner>();
  auto r2 = std::make_shared<test_runner>();


  // local scope to help debugging
  {
    std::mutex m;
    std::condition_variable cv;
    int num_clients = 0;
    std::array<async::net::stream, 2> srv_stream;

    auto server = async::net::server(
        std::weak_ptr<test_runner>(r)
      , ipv6::any
      , 22220
      , std::bind(stream_factory, _1, _2, _3, r2
          , [] (const std::shared_ptr<test_runner>&, void*, std::size_t&)
            { }
          , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
            { }
          , [&num_clients, &m, &cv] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
            {
              r->inc();
              std::cout << "server received event " << static_cast<int>(evt) << "\n";
              switch (evt)
              {
                case oob_event::disconnect:
                  --num_clients;
                  cv.notify_one();
                  break;

                default:
                  break;
              }
            }
        )
      , [&m, &cv, &srv_stream, &r2, &num_clients, &srv_addr](const std::shared_ptr<test_runner>& r, const async::net::stream& s_)
        {
          r->inc();
          srv_stream[num_clients] = s_;
          ++num_clients;
        }
    );

    server.start();

    auto client1 = create_connected_client<test_runner>(r2, ipv4::loopback, 22220, 100);


    std::unique_lock<std::mutex> l(m);
    cv.wait_for(l, ms(100), [&client1, &num_clients] () { return !!client1 && num_clients == 1; } );
    BOOST_CHECK_EQUAL(true, !!client1);
    BOOST_CHECK_EQUAL(1, num_clients);

    auto client2 = create_client<test_runner>(r2, 100);
    client2->connect(ipv6::loopback, 22220);

    cv.wait_for(l, ms(100), [&client2, &num_clients] () { return !!client2 && num_clients == 2; } );
    BOOST_CHECK_EQUAL(2, num_clients);
    BOOST_CHECK_EQUAL(true, !!client2);

    client1.reset();
    client2.reset();

    cv.wait_for(l, ms(100), [&num_clients] () { return num_clients == 0; } );
    BOOST_CHECK_EQUAL(0, num_clients);
  }
}
#endif

#if 1
BOOST_AUTO_TEST_CASE(start_stop)
{
  check_start_sockets();

  auto r = std::make_shared<test_runner>();
  auto r2 = std::make_shared<test_runner>();


  // local scope to help debugging
  {
    std::mutex m;
    std::condition_variable cv;
    int num_clients = 0;
    std::array<async::net::stream, 2> srv_stream;

    auto server = async::net::server(
        std::weak_ptr<test_runner>(r)
      , ipv6::any
      , 22220
      , std::bind(stream_factory, _1, _2, _3, r
          , [] (const std::shared_ptr<test_runner>&, void*, std::size_t&)
            { }
          , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
            { }
          , [&num_clients, &m, &cv] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
            {
              if (r)
                r->inc();
              std::cout << "server received event " << static_cast<int>(evt) << "\n";
              switch (evt)
              {
                case oob_event::disconnect:
                  --num_clients;
                  cv.notify_one();
                  break;

                default:
                  break;
              }
            }
        )
      , [&m, &cv, &srv_stream, &r2, &num_clients](const std::shared_ptr<test_runner>& r, const async::net::stream& s_)
        {
          if (r)
            r->inc();

          std::unique_lock<std::mutex> l(m);
          srv_stream[num_clients] = s_;
          ++num_clients;
          cv.notify_one();
        }
    );

    server.start();

    auto client1 = create_connected_client<test_runner>(r2, ipv4::loopback, 22220, 100);

    std::unique_lock<std::mutex> l(m);
    cv.wait_for(l, ms(100), [&client1, &num_clients] () { return !!client1 && num_clients == 1; } );
    BOOST_CHECK_EQUAL(true, !!client1);
    BOOST_CHECK_EQUAL(1, num_clients);

    // stop server
    server.stop();
    std::this_thread::sleep_for(ms(10));

    auto client2 = create_client<test_runner>(r2, 100);
    BOOST_CHECK_NO_THROW(client2->connect(ipv6::loopback, 22220));

    cv.wait_for(l, ms(100), [&client2, &num_clients] () { return !!client2 && num_clients == 2; } );
    BOOST_CHECK_EQUAL(1, num_clients);
    BOOST_CHECK_EQUAL(true, !!client2);

    // start server
    server.start();
    std::this_thread::sleep_for(ms(10));

    BOOST_CHECK_THROW(client2->connect(ipv6::loopback, 22220), cool::ng::exception::invalid_state);

    cv.wait_for(l, ms(100), [&client2, &num_clients] () { return !!client2 && num_clients == 2; } );
    BOOST_CHECK_EQUAL(2, num_clients);
    BOOST_CHECK_EQUAL(true, !!client2);

    client1.reset();
    client2.reset();

    cv.wait_for(l, ms(100), [&num_clients] () { return num_clients == 0; } );
    BOOST_CHECK_EQUAL(0, num_clients);
  }
  std::this_thread::sleep_for(ms(200));
}

#endif


#if 1
BOOST_AUTO_TEST_CASE(read_write_test_1)
{
  std::vector<uint8_t> buffer;
  buffer.resize(2500000);

  check_start_sockets();

  std::mutex m;
  std::condition_variable cv;
  bool client_connected = false;
  bool srv_connected = false;
  std::size_t srv_size = 0;
  std::size_t clt_size = 0;
  bool srv_written = false;
  bool clt_written = false;

  async::net::stream srv_stream;

  auto r = std::make_shared<test_runner>();
  auto r2 = std::make_shared<test_runner>();
  {
    auto server = async::net::server(
        std::weak_ptr<test_runner>(r)
      , ipv6::any
      , 22228
      , std::bind(stream_factory, _1, _2, _3, r2
          , [&srv_size, &m, &cv, &buffer] (const std::shared_ptr<test_runner>&, void*&, std::size_t& size_)
            {
              srv_size += size_;
//                std::cout << "received " << size_ << " bytes, total " << srv_size << " bytes\n";
              if (srv_size >= buffer.capacity())
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
          , [&srv_connected, &m, &cv] (const std::shared_ptr<test_runner>&, oob_event evt, const std::error_code& e)
            {
              std::cout << "server received event " << static_cast<int>(evt) << "\n";
              switch (evt)
              {
                case oob_event::connect:
                  srv_connected = true;
                  cv.notify_one();
                  break;

                case oob_event::disconnect:
                  srv_connected = false;
                  cv.notify_one();
                  break;

                default:
                  break;
              }
            }
        )
      , [&m, &cv, &srv_stream, &r2, &srv_connected](const std::shared_ptr<test_runner>& r, const async::net::stream& s_)
        {
          if (r)
            r->inc();

          std::unique_lock<std::mutex> l(m);
          srv_stream = s_;
          srv_connected = true;
          cv.notify_one();
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
            if (clt_size >= buffer.capacity())
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
        , [&m, &cv, &client_connected] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
          {
            if (r)
              r->inc();

            std::unique_lock<std::mutex> l(m);
            std::cout << "client received event " << static_cast<int>(evt) << "\n";
            switch (evt)
            {
              case oob_event::connect:
                client_connected = true;
                cv.notify_one();
                break;

              case oob_event::disconnect:
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

      client->write(buffer.data(), buffer.capacity());
      srv_stream.write(buffer.data(), buffer.capacity());
      cv.wait_for(l, ms(2000),
        [&clt_size, &srv_size, &buffer, &srv_written,&clt_written] ()
        {
          return clt_written && srv_written &&
                 clt_size >= buffer.capacity() && srv_size >= buffer.capacity();
        }
      );
      BOOST_CHECK_EQUAL(buffer.capacity(), srv_size);
      BOOST_CHECK_EQUAL(buffer.capacity(), clt_size);
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
  std::this_thread::sleep_for(ms(200));
}
#endif

#if 1
BOOST_AUTO_TEST_CASE(connect_disconnect_connect_disconnect_test)
{
  check_start_sockets();
  {
    std::mutex m;
    std::condition_variable cv;
    bool srv_connected = false;
    bool clt_connected = false;
    async::net::stream srv_stream;

    auto r = std::make_shared<test_runner>();
    auto r2 = std::make_shared<test_runner>();

    auto server = async::net::server(
        std::weak_ptr<test_runner>(r)
      , ipv6::any
      , 22229
      , std::bind(stream_factory, _1, _2, _3, r2
          , [] (const std::shared_ptr<test_runner>&, void*, std::size_t&)
            { }
          , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
            { }
          , [ &m, &cv, &srv_connected, &srv_stream] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
            {
              if (r)
                r->inc();
              std::cout << "server received event " << static_cast<int>(evt) << "\n";
              switch (evt)
              {
                case oob_event::disconnect:
                  srv_connected = false;
                  srv_stream = async::net::stream();
                  cv.notify_one();
                  break;

                default:
                  break;
              }
            }
        )
      , [&m, &cv, &srv_stream, &r2, &srv_connected](const std::shared_ptr<test_runner>& r, const async::net::stream& s_)
        {
          std::unique_lock<std::mutex> l(m);
          srv_stream = s_;
          srv_connected = true;
          cv.notify_one();
        }
    );

    server.start();

    async::net::stream client(
        std::weak_ptr<test_runner>(r2)
      , [] (const std::shared_ptr<test_runner>&, void*&, std::size_t&)
        { }
      , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
        { }
      , [&m, &cv, &clt_connected] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
        {
          if (r)
            r->inc();

          std::unique_lock<std::mutex> l(m);
          std::cout << "client received event " << static_cast<int>(evt) << "\n";
          switch (evt)
          {
            case oob_event::connect:
              clt_connected = true;
              cv.notify_one();
              break;

            case oob_event::disconnect:
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
    BOOST_CHECK_EQUAL(true, clt_connected);
    BOOST_CHECK_EQUAL(true, srv_connected);
    BOOST_CHECK_EQUAL(true, !!srv_stream);

    client.disconnect();
    std::this_thread::sleep_for(ms(100));  // make sure scoped variables remain in life for server's lambda
  }
  std::this_thread::sleep_for(ms(100));
}
#endif

#if 0
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
        , [] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
          {}
        , nullptr
        , 50000000000
      )
    , std::exception
  );
#endif

}
#endif
#if 0
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
        , [] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
          {}
      )
    , cool::ng::exception::socket_failure
  );
#endif

}
#endif


BOOST_AUTO_TEST_SUITE_END()




