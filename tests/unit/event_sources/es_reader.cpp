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

#define TEST01 1
#define TEST02 1
#define TEST03 1
#define TEST04 1
#define TEST05 1
#define TEST06 1
#define TEST07 1
#define TEST08 1
#define TEST09 1
#define TEST10 1
#define TEST11 1
#define TEST12 0  // this test may require shutting  down network interfaces

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
using cool::ng::error::errc;

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
    wsa_started = true;
  }
#endif
}

std::atomic<bool> boolean_flag;

void spin_wait(unsigned int msec, const std::function<bool()>& lambda)
{
  auto start = std::chrono::system_clock::now();
  while (!lambda())
  {
    auto now = std::chrono::system_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() >= msec)
      return;
  }
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
      {
        switch (evt)
        {
          case oob_event::connect:
            boolean_flag = true;
            break;

          case oob_event::disconnect:
            boolean_flag = false;
            break;

          default:
            break;
        }
      }
  );

  return clt;
}

template <typename RunnerT>
std::shared_ptr<async::net::stream> create_connected_client(const std::shared_ptr<test_runner>& r, const ip::address& addr, uint16_t port, int msec)
{
  boolean_flag = false;

  auto clt = std::make_shared<async::net::stream>(
      std::weak_ptr<test_runner>(r)
    , addr
    , port
    , [] (const std::shared_ptr<RunnerT>&, void*&, std::size_t&)
      { }
    , [] (const std::shared_ptr<RunnerT>&, const void*, std::size_t)
      { }
    , [] (const std::shared_ptr<RunnerT>& r, oob_event evt, const std::error_code& e)
      {
        switch (evt)
        {
          case oob_event::connect:
            boolean_flag = true;
            break;

          case oob_event::disconnect:
            boolean_flag = false;
            break;

          default:
            break;
        }
      }
  );

  spin_wait(msec, [] () { return boolean_flag.load(); });
  if (!boolean_flag)
    clt.reset();
  return clt;
}

template <typename RunnerT>
std::shared_ptr<async::net::stream> create_connected_client(
    const std::shared_ptr<test_runner>& r
  , const ip::address& addr
  , uint16_t port
  , int msec
  , const std::function<void(const std::shared_ptr<RunnerT>& r, oob_event evt, const std::error_code& e)>& f)
{
  boolean_flag = false;

  auto clt = std::make_shared<async::net::stream>(
      std::weak_ptr<test_runner>(r)
    , addr
    , port
    , [] (const std::shared_ptr<RunnerT>&, void*&, std::size_t&)
      { }
    , [] (const std::shared_ptr<RunnerT>&, const void*, std::size_t)
      { }
    , [f] (const std::shared_ptr<RunnerT>& r, oob_event evt, const std::error_code& e)
      {
        switch (evt)
        {
          case oob_event::connect:
            boolean_flag = true;
            break;

          case oob_event::disconnect:
            boolean_flag = false;
            break;

          default:
            break;
        }
        f(r, evt, e);
      }
  );

  spin_wait(msec, [] () { return boolean_flag.load(); });
  if (!boolean_flag)
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
    return async::net::stream(stream_r_, rh_, wh_, eh_, nullptr, 100000);
}

BOOST_AUTO_TEST_CASE(empty_test)
{
  {
    auto r = std::make_shared<test_runner>();
  }
  std::this_thread::sleep_for(ms(100));
  BOOST_CHECK_EQUAL(true, true);
}

#if TEST01 == 1
// Just creates and destroys server to check that everything completes
// gracefully.
BOOST_AUTO_TEST_CASE(server_dtor)
{
  check_start_sockets();

  try
  {
#if 1
    // destroy  server in initial state
    {
      auto r = std::make_shared<test_runner>();

      {
        auto server = async::net::server(
            std::weak_ptr<test_runner>(r)
          , ipv6::any
          , 22218
          , std::bind(stream_factory, _1, _2, _3, r
              , [] (const std::shared_ptr<test_runner>&, void*, std::size_t&)
                { }
              , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
                { }
              , [] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
                {
                }
            )
          , [](const std::shared_ptr<test_runner>& r, const async::net::stream& s_)
            {
            }
        );

        std::this_thread::sleep_for(ms(50));
      }
      std::this_thread::sleep_for(ms(100));
      BOOST_CHECK_EQUAL(false, false);
    }
#endif
#if 1
    // destroy  server in started state
    {
      auto r = std::make_shared<test_runner>();

      {
        auto server = async::net::server(
            std::weak_ptr<test_runner>(r)
          , ipv6::any
          , 22219
          , std::bind(stream_factory, _1, _2, _3, r
              , [] (const std::shared_ptr<test_runner>&, void*, std::size_t&)
                { }
              , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
                { }
              , [] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
                {
                }
            )
          , [](const std::shared_ptr<test_runner>& r, const async::net::stream& s_)
            {
            }
        );

        server.start();
        std::this_thread::sleep_for(ms(50));
      }
      std::this_thread::sleep_for(ms(100));
      BOOST_CHECK_EQUAL(false, false);
    }
#endif
#if 1
    // destroy  server in stopped state
    {
      auto r = std::make_shared<test_runner>();

      {
        auto server = async::net::server(
            std::weak_ptr<test_runner>(r)
          , ipv6::any
          , 22220
          , std::bind(stream_factory, _1, _2, _3, r
              , [] (const std::shared_ptr<test_runner>&, void*, std::size_t&)
                { }
              , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
                { }
              , [] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
                {
                }
            )
          , [](const std::shared_ptr<test_runner>& r, const async::net::stream& s_)
            {
            }
        );

        server.start();
        std::this_thread::sleep_for(ms(50));
        server.stop();
        std::this_thread::sleep_for(ms(50));
      }
      std::this_thread::sleep_for(ms(100));
      BOOST_CHECK_EQUAL(false, false);
    }
#endif
    std::this_thread::sleep_for(ms(100));
    BOOST_CHECK_EQUAL(true, true);
  }
  catch (const cool::ng::exception::base& e)
  {
    std::cout << "**************** Exception: " << e.code().message() << "\n";
  }
}
#endif

// test handling of errors
#if TEST02 == 1
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
    , cool::ng::exception::system_error);
  }
  std::this_thread::sleep_for(ms(100));
}
#endif

#if TEST03 == 1
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
    std::atomic<int> num_clients(0);
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
          , [&num_clients] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
            {
              r->inc();
              switch (evt)
              {
                case oob_event::disconnect:
                  --num_clients;
                  break;

                case oob_event::connect:
                  break;

                default:
                  break;
              }
            }
        )
      , [&srv_stream, &r2, &num_clients](const std::shared_ptr<test_runner>& r, const async::net::stream& s_)
        {
          r->inc();
          srv_stream[num_clients] = s_;
          ++num_clients;
        }
    );

    server.start();

    std::shared_ptr<async::net::stream> client1, client2;
//    BOOST_CHECK_NO_THROW(client1 = create_connected_client<test_runner>(r2, ipv4::loopback, 22220, 100));

    BOOST_CHECK_NO_THROW(client1 = create_connected_client<test_runner>(r2, ipv4::loopback, 22220, 100
          , [] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
            {
              r->inc();
              switch (evt)
              {
                case oob_event::disconnect:
                  break;

                case oob_event::connect:
                  break;

                default:
                  break;
              }
            }
    ));

    spin_wait(100, [&client1, &num_clients] () { return !!client1 && num_clients == 1; } );
    BOOST_CHECK_EQUAL(true, !!client1);
    BOOST_CHECK_EQUAL(1, num_clients);
    BOOST_CHECK_NO_THROW(client2 = create_client<test_runner>(r2, 100));
    boolean_flag = false;
    BOOST_CHECK_NO_THROW(client2->connect(ipv6::loopback, 22220));

    spin_wait(100, [] () { return boolean_flag.load(); });
    spin_wait(100, [&client2, &num_clients] () { return !!client2 && num_clients == 2; } );
    BOOST_CHECK_EQUAL(2, num_clients);
    BOOST_CHECK_EQUAL(true, !!client2);
    BOOST_CHECK_EQUAL(true, boolean_flag.load());

    client1->disconnect();
    client2.reset();
    spin_wait(300, [&num_clients] () { return num_clients == 0; } );
    BOOST_CHECK_EQUAL(0, num_clients);
    client1.reset();
  }
  std::this_thread::sleep_for(ms(100));
}
#endif

#if TEST04 == 1
BOOST_AUTO_TEST_CASE(start_stop)
{
  check_start_sockets();

  auto r = std::make_shared<test_runner>();
  auto r2 = std::make_shared<test_runner>();

  // local scope to help debugging
  {
    std::atomic<int> num_clients(0);
    std::array<async::net::stream, 2> srv_stream;

    auto server = async::net::server(
        std::weak_ptr<test_runner>(r)
      , ipv6::any
      , 11220
      , std::bind(stream_factory, _1, _2, _3, r
          , [] (const std::shared_ptr<test_runner>&, void*, std::size_t&)
            { }
          , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
            { }
          , [&num_clients] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
            {
              r->inc();
              switch (evt)
              {
                case oob_event::disconnect:
                  --num_clients;
                  break;

                default:
                  break;
              }
            }
        )
      , [&srv_stream, &r2, &num_clients](const std::shared_ptr<test_runner>& r, const async::net::stream& s_)
        {
          r->inc();
          srv_stream[num_clients] = s_;
          ++num_clients;
        }
    );

    server.start();

    std::shared_ptr<async::net::stream> client1, client2;

    BOOST_CHECK_NO_THROW(client1 = create_connected_client<test_runner>(r2, ipv4::loopback, 11220, 100));

    spin_wait(100, [&client1, &num_clients] () { return !!client1 && num_clients == 1; } );
    BOOST_CHECK_EQUAL(true, !!client1);
    BOOST_CHECK_EQUAL(1, num_clients);

    // stop server
    server.stop();
    std::this_thread::sleep_for(ms(100));

    // despite server being stopped client receives connect immediatelly ...
    boolean_flag = false;
    BOOST_CHECK_NO_THROW(client2 = create_client<test_runner>(r2, 100));
    BOOST_CHECK_NO_THROW(client2->connect(ipv6::loopback, 11220));

    spin_wait(1000, [] () { return boolean_flag.load(); });
    BOOST_CHECK_EQUAL(true, boolean_flag.load());
    BOOST_CHECK_EQUAL(1, num_clients);
    BOOST_CHECK_EQUAL(true, !!client2);

    // start server
    server.start();

    // ... but server callback is triggered only after the server is started again
    spin_wait(2000, [&client2, &num_clients] () { return !!client2 && num_clients == 2; } );
    BOOST_CHECK_EQUAL(2, num_clients);
    BOOST_CHECK_EQUAL(true, !!client2);
    BOOST_CHECK_THROW(client2->connect(ipv6::loopback, 11220), cool::ng::exception::invalid_state);


    client1.reset();
    client2.reset();

    spin_wait(300, [&num_clients] () { return num_clients == 0; } );
    BOOST_CHECK_EQUAL(0, num_clients);
  }

  std::this_thread::sleep_for(ms(200));
}

#endif


#if TEST05 == 1
BOOST_AUTO_TEST_CASE(read_write_test_1)
{
  std::vector<uint8_t> buffer;
  buffer.resize(500000);

  check_start_sockets();

  std::atomic<bool> client_connected(false);
  std::atomic<bool> srv_connected(false);
  std::atomic<std::size_t> srv_size(0);
  std::atomic<std::size_t> clt_size(0);
  std::atomic<bool> srv_written(false);
  std::atomic<bool> clt_written(false);

  async::net::stream srv_stream;

  auto r = std::make_shared<test_runner>();
  auto r2 = std::make_shared<test_runner>();
  {
    auto server = async::net::server(
        std::weak_ptr<test_runner>(r)
      , ipv6::any
      , 22228
      , std::bind(stream_factory, _1, _2, _3, r2
          , [&srv_size, &buffer] (const std::shared_ptr<test_runner>&, void*&, std::size_t& size_)
            {
              srv_size += size_;
            }
          , [&srv_written] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
            {
              srv_written = true;
            }
          , [&srv_connected, &srv_stream] (const std::shared_ptr<test_runner>&, oob_event evt, const std::error_code& e)
            {
              switch (evt)
              {
                case oob_event::connect:
                  srv_connected = true;
                  break;

                case oob_event::disconnect:
                  srv_connected = false;
                  srv_stream = async::net::stream();
                  break;

                default:
                  break;
              }
            }
        )
      , [&srv_stream, &r2, &srv_connected](const std::shared_ptr<test_runner>& r, const async::net::stream& s_)
        {
          if (r)
            r->inc();

          srv_stream = s_;
          srv_connected = true;
        }
    );

    server.start();

    {
      auto client = std::make_shared<async::net::stream>(
          std::weak_ptr<test_runner>(r2)
        , ipv4::loopback
        , 22228
        , [&clt_size, &buffer] (const std::shared_ptr<test_runner>&, void*&, std::size_t& size)
          {
            clt_size += size;
          }
        , [&clt_written] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
          {
            clt_written = true;
          }
        , [&client_connected] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
          {
            if (r)
              r->inc();

            switch (evt)
            {
              case oob_event::connect:
                client_connected = true;
                break;

              case oob_event::disconnect:
                client_connected = false;
                break;

              default:
                break;
            }
          }
      );

      spin_wait(2000, [&client_connected, &srv_connected]() { return client_connected && srv_connected; } );
      BOOST_CHECK_EQUAL(true, srv_connected);
      BOOST_CHECK_EQUAL(true, !!srv_stream);
      BOOST_CHECK_EQUAL(true, client_connected);
      BOOST_CHECK_EQUAL(1, r->counter());
      BOOST_CHECK_EQUAL(1, r2->counter());

      client->write(buffer.data(), buffer.capacity());
      srv_stream.write(buffer.data(), buffer.capacity());
      spin_wait(5000,
        [&clt_size, &srv_size, &buffer, &srv_written,&clt_written] ()
        {
          return srv_written && clt_written && clt_size >= buffer.capacity() && srv_size >= buffer.capacity();
        }
      );
      BOOST_CHECK_EQUAL(buffer.capacity(), srv_size);
      BOOST_CHECK_EQUAL(buffer.capacity(), clt_size);
      BOOST_CHECK_EQUAL(true, srv_written);
      BOOST_CHECK_EQUAL(true, clt_written);

    }

    spin_wait(2000, [&client_connected]() { return !!client_connected; } );
    spin_wait(2000, [&srv_connected]() { return !srv_connected; } );

    BOOST_CHECK_EQUAL(false, srv_connected);
    BOOST_CHECK_EQUAL(true, client_connected);
    BOOST_CHECK_EQUAL(1, r->counter());
    BOOST_CHECK_EQUAL(1, r2->counter());
  }
  spin_wait(100, [] () { return false; });
}
#endif

#if TEST06 == 1
BOOST_AUTO_TEST_CASE(connect_disconnect_connect_disconnect_test)
{
  check_start_sockets();
  {
    std::atomic<bool> srv_connected(false);
    std::atomic<bool> clt_connected(false);
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
          , [&srv_connected, &srv_stream] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
            {
              r->inc();
              switch (evt)
              {
                case oob_event::disconnect:
                  srv_connected = false;
                  srv_stream = async::net::stream();
                  break;

                default:
                  break;
              }
            }
        )
      , [&srv_stream, &r2, &srv_connected](const std::shared_ptr<test_runner>& r, const async::net::stream& s_)
        {
          srv_stream = s_;
          srv_connected = true;
        }
    );

    server.start();

    async::net::stream client(
        std::weak_ptr<test_runner>(r2)
      , [] (const std::shared_ptr<test_runner>&, void*&, std::size_t&)
        { }
      , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
        { }
      , [&clt_connected] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
        {
          r->inc();

          switch (evt)
          {
            case oob_event::connect:
              clt_connected = true;
              break;

            case oob_event::disconnect:
              clt_connected = false;
              break;

            default:
              break;
          }
        }
    );

    std::cout << "client's name is " << client.name() << "\n";

    client.connect(cool::ng::net::ipv6::loopback, 22229);

    spin_wait(500, [&clt_connected, &srv_connected] () { return clt_connected && srv_connected; } );
    BOOST_CHECK_EQUAL(true, clt_connected);
    BOOST_CHECK_EQUAL(true, srv_connected);
    BOOST_CHECK_EQUAL(true, !!srv_stream);
    BOOST_CHECK_THROW(client.connect(cool::ng::net::ipv6::loopback, 22229), cool::ng::exception::invalid_state);

    client.disconnect();
    spin_wait(500, [&clt_connected, &srv_connected] () { return clt_connected && !srv_connected; } );
    BOOST_CHECK_EQUAL(true, clt_connected);
    BOOST_CHECK_EQUAL(false, srv_connected);
    BOOST_CHECK_EQUAL(false, !!srv_stream);

    clt_connected = false;
    client.connect(cool::ng::net::ipv4::loopback, 22229);
    spin_wait(500, [&clt_connected, &srv_connected] () { return clt_connected && srv_connected; } );
    BOOST_CHECK_EQUAL(true, clt_connected);
    BOOST_CHECK_EQUAL(true, srv_connected);
    BOOST_CHECK_EQUAL(true, !!srv_stream);

    client.disconnect();
    std::this_thread::sleep_for(ms(100));  // make sure scoped variables remain in life for server's lambda
  }
  std::this_thread::sleep_for(ms(100));
}
#endif

#if TEST07 == 1
BOOST_AUTO_TEST_CASE(buffer_size_failures)
{
  check_start_sockets();

  auto r = std::make_shared<test_runner>();
  {
    BOOST_CHECK_THROW(
        async::net::stream client(
            std::weak_ptr<test_runner>(r)
          , cool::ng::net::ipv6::loopback
          , 12346
          , [] (const std::shared_ptr<test_runner>&, void*&, std::size_t&)
            { }
          , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
            { }
          , [] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
            {}
          , nullptr
          , 0
        )
      , cool::ng::exception::illegal_argument
    );
  }
  {
    std::shared_ptr<async::net::stream> srv;
    BOOST_CHECK_NO_THROW(srv = std::make_shared<async::net::stream>(
        std::weak_ptr<test_runner>(r)
      , [] (const std::shared_ptr<test_runner>&, void*&, std::size_t&)
        { }
      , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
        { }
      , [] (const std::shared_ptr<test_runner>& r, oob_event evt, const std::error_code& e)
        {}
      , nullptr
      , 0
    ));

    BOOST_CHECK_THROW(srv->connect(ipv6::loopback, 12346), cool::ng::exception::illegal_argument);
  }
  std::this_thread::sleep_for(ms(100));
}
#endif


#if TEST08 == 1
BOOST_AUTO_TEST_CASE(valid_custom_read_buffer)
{
  check_start_sockets();

  const char* send = "0123456789";
  char receive[10];
  auto r = std::make_shared<test_runner>();
  auto r2 = std::make_shared<test_runner>();

  {
    cool::ng::async::net::stream srv_stream;
    std::atomic<bool> connected(false);
    std::atomic<bool> read_complete(false);
    std::atomic<bool> write_complete(false);
    std::atomic<bool> srv_connect(false);
    std::atomic<bool> clt_connect(false);

    auto server = async::net::server(
        std::weak_ptr<test_runner>(r)
      , ipv4::any
      , 12121
      , std::bind(stream_factory, _1, _2, _3, r
            ,[](const std::shared_ptr<test_runner>& r_, void*& b_, std::size_t& s_)
             { }
            ,[&write_complete](const std::shared_ptr<test_runner>& r_, const void* b_, std::size_t s_)
             {
               write_complete = true;
             }
            ,[](const std::shared_ptr<test_runner>& r_, oob_event evt_, const std::error_code& e_)
             { }
        )
      , [&srv_stream, &srv_connect](const std::shared_ptr<test_runner>& r_, const async::net::stream& s_)
        {
          srv_stream = s_;
          srv_connect = true;
        }
    );

    server.start();

    memset(receive, 0, sizeof(receive));
    auto clt_stream = std::make_shared<async::net::stream>(
          std::weak_ptr<test_runner>(r2)
        , [&receive, &read_complete] (const std::shared_ptr<test_runner>&, void*& b_, std::size_t& s_)
          {
            if (b_ == receive)
            {
              b_ = reinterpret_cast<void *>(receive + 6);
              s_ = 4;
            }
            else if (b_ == receive + 6)
             read_complete = true;
          }
        , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
          { }
        , [&clt_connect] (const std::shared_ptr<test_runner>& r_, oob_event evt_, const std::error_code& e_)
          {
            clt_connect = true;
          }
        , reinterpret_cast<void*>(receive)
        , 6
      );

    clt_stream->connect(cool::ng::net::ipv4::loopback, 12121);

    spin_wait(2000, [&clt_connect, &srv_connect]() { return clt_connect.load() && srv_connect.load();});
    BOOST_CHECK_EQUAL(true, srv_connect.load());
    BOOST_CHECK_EQUAL(true, clt_connect.load());
    std::cout << "client stream " << clt_stream->name() << "\nserver stream " << srv_stream.name() << "\n";

    srv_stream.write(send, 10);

    // wait until everything is read
    spin_wait(2000, [&read_complete, &write_complete]() { return read_complete.load() && write_complete.load(); });
    BOOST_CHECK_EQUAL(true, read_complete.load());
    BOOST_CHECK_EQUAL(true, write_complete.load());
    BOOST_CHECK_EQUAL(0, memcmp(receive,  send, 10));
  }
  std::this_thread::sleep_for(ms(100));
}
#endif

#if TEST09 == 1
BOOST_AUTO_TEST_CASE(invalid_custom_read_buffer)
{
  check_start_sockets();

  const char* send = "0123456789";
  char receive[10];

  try
  {
    auto r = std::make_shared<test_runner>();
    auto r2 = std::make_shared<test_runner>();

    cool::ng::async::net::stream srv_stream;
    std::atomic<bool> connected(false);
    std::atomic<bool> buf_usage_error(false);

    auto server = async::net::server(
        std::weak_ptr<test_runner>(r)
      , ipv4::any
      , 12122
      , std::bind(stream_factory, _1, _2, _3, r
            ,[](const std::shared_ptr<test_runner>& r_, void*& b_, std::size_t& s_)
             { }
            ,[](const std::shared_ptr<test_runner>& r_, const void* b_, std::size_t s_)
             { }
            ,[](const std::shared_ptr<test_runner>& r_, oob_event evt_, const std::error_code& e_)
             { }
        )
      , [&srv_stream](const std::shared_ptr<test_runner>& r_, const async::net::stream& s_)
        {
          srv_stream = s_;
          boolean_flag = true;
        }
    );

    server.start();

    // buf moved 6 characters ahead, cnt set to zero - not valid; if buf is changed, size must be specified!
    {
      memset(receive, 'X', sizeof(receive));
      auto clt_stream = std::make_shared<async::net::stream>(
            std::weak_ptr<test_runner>(r2)
          , [&receive] (const std::shared_ptr<test_runner>&, void*& b_, std::size_t& s_)
            {
              static int count = 0;

              if (++count == 1)
              {
                b_ = reinterpret_cast<void *>(receive + 6);
                s_ = 0;
              }
              else
                boolean_flag = true;
            }
          , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
            { }
          , [&connected, &buf_usage_error] (const std::shared_ptr<test_runner>& r_, oob_event evt_, const std::error_code& e_)
            {
              // todo: check comparing of e_
              if (evt_ == oob_event::internal && e_ == cool::ng::error::make_error_code(cool::ng::error::errc::out_of_range) )
              {
                buf_usage_error = true;
              }
              else
              {
                connected = true;
              }

            }
          , reinterpret_cast<void*>(receive)
          , 6
        );

      boolean_flag = false;
      clt_stream->connect(cool::ng::net::ipv4::loopback, 12122);
      spin_wait(2000, [&connected]() { return boolean_flag.load() && connected.load();});
      BOOST_CHECK_EQUAL(true, connected.load());
      BOOST_CHECK_EQUAL(true, boolean_flag.load());

      srv_stream.write(send, 10);

      // wait until everything is read
      spin_wait(2000, [&buf_usage_error]() { return buf_usage_error.load(); });
      BOOST_CHECK_EQUAL(true, buf_usage_error);
    }

    // buf nullptr, should allocate new buffer
    {
      memset(receive, 'X', sizeof(receive));
      auto clt_stream = std::make_shared<async::net::stream>(
            std::weak_ptr<test_runner>(r2)
          , [&receive] (const std::shared_ptr<test_runner>&, void*& b_, std::size_t& s_)
            {
              static int count = 0;

              if (++count == 1)
              {
                b_ = nullptr;
              }
              else
                boolean_flag = true;
            }
          , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)
            { }
          , [&connected] (const std::shared_ptr<test_runner>& r_, oob_event evt_, const std::error_code& e_)
            {
              connected = true;
            }
          , reinterpret_cast<void*>(receive)
          , 6
        );

      boolean_flag = false;
      clt_stream->connect(cool::ng::net::ipv4::loopback, 12122);
      spin_wait(2000, [&connected]() { return boolean_flag.load() && connected.load();});
      BOOST_CHECK_EQUAL(true, connected.load());
      BOOST_CHECK_EQUAL(true, boolean_flag.load());

      boolean_flag.store(false);
      srv_stream.write(send, 10);

      // wait until everything is read
      spin_wait(2000, []() { return boolean_flag.load(); });
      BOOST_CHECK_EQUAL(true, boolean_flag.load());
      BOOST_CHECK_EQUAL(0, memcmp(receive,  "012345XXXX", 10));
    }
  }
  catch (const cool::ng::exception::base& e)
  {
    std::cout << "Exception: " << e.code().message() << "\n";
  }
  std::this_thread::sleep_for(ms(100));
}
#endif

// -------------------------------------------------------------------
// ----
// ----  Stream fail connect tests
// ----
// -------------------------------------------------------------------

// the idea of this test is to shut down the stream while connecting;
// needs some service that doesn't respond (quickly) to connect request
#if TEST10 == 1
BOOST_AUTO_TEST_CASE(long_connect_interrupted)
{
  check_start_sockets();
  std::atomic<bool> rep_conn(false);
  std::atomic<bool> rep_disconn(false);
  std::atomic<bool> rep_fail(false);
  std::atomic<bool> rep_internal(false);

  std::error_code err = cool::ng::error::no_error();

  auto r = std::make_shared<test_runner>();

  auto clt_stream = std::make_shared<async::net::stream>(
        std::weak_ptr<test_runner>(r)
      , [] (const std::shared_ptr<test_runner>&, void*& b_, std::size_t& s_) { }
      , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)   { }
      , [&rep_conn, &rep_disconn, &rep_fail, &rep_internal, &err] (const std::shared_ptr<test_runner>& r_, oob_event evt_, const std::error_code& e_)
        {
          std::cout << "got event " << static_cast<int>(evt_) << ", code " << e_.value() << " (" << e_.message() << ")\n";
          switch (evt_)
          {
            case oob_event::connect:
              rep_conn = true; break;
            case oob_event::disconnect:
              rep_disconn = true; break;
            case oob_event::failure:
              rep_fail = true; err = e_; break;
            case oob_event::internal:
              rep_internal = true; err = e_; break;

          }
        }
    );

  try
  {
    // first attempt will be interrupted by disconnect, the user callbacks will
    // still exist - the callback should report to error handler
    rep_conn = false;
    rep_disconn = false;
    rep_fail = false;

//std::cout << "**** about to connect first time \n";
    clt_stream->connect(ipv4::host("10.17.0.0"), 12345);
    spin_wait(100, [] () { return false; });     // wait 100ms
    BOOST_CHECK_EQUAL(false, rep_conn.load());
    BOOST_CHECK_EQUAL(false, rep_disconn.load());
    BOOST_CHECK_EQUAL(false, rep_fail.load());

//std::cout << "**** about to discconnect first time \n";

    clt_stream->disconnect();
    spin_wait(100, [&rep_fail] () { return rep_fail.load(); });
//std::cout << "**** disconnected first time \n";
    BOOST_CHECK_EQUAL(false, rep_conn.load());
    BOOST_CHECK_EQUAL(false, rep_disconn.load());
    BOOST_CHECK_EQUAL(true, rep_fail.load());
    BOOST_CHECK_EQUAL(static_cast<int>(errc::request_aborted), err.value());


    rep_conn = false;
    rep_disconn = false;
    rep_fail = false;
    err = cool::ng::error::no_error();
    // second attempt will destroy stream - no callback expected as user callback
    // will no longer exist
//std::cout << "**** about to connect second time \n";
    clt_stream->connect(ipv4::host("10.17.0.0"), 12345);
    spin_wait(100, [] () { return false; });
    BOOST_CHECK_EQUAL(false, rep_conn.load());
    BOOST_CHECK_EQUAL(false, rep_disconn.load());
    BOOST_CHECK_EQUAL(false, rep_fail.load());
//std::cout << "**** about to discconnect second time \n";
    clt_stream.reset();
    spin_wait(100, [] () { return false; });

    BOOST_CHECK_EQUAL(false, rep_conn.load());
    BOOST_CHECK_EQUAL(false, rep_disconn.load());
    BOOST_CHECK_EQUAL(false, rep_fail.load());
//std::cout << "**** disconnected second time \n";
  }
  catch (const cool::ng::exception::base& e)
  {
    std::cout << "!!!!! Exception: " << e.code().message() << "\n";
  }
  catch (const std::exception& e)
  {
    std::cout << "!!!!! Exception: " << e.what() << "\n";
  }
}
#endif

// this test tries to connect to unserved port - rejected connection
#if TEST11 == 1
BOOST_AUTO_TEST_CASE(no_service)
{
  check_start_sockets();
  std::atomic<bool> rep_conn(false);
  std::atomic<bool> rep_disconn(false);
  std::atomic<bool> rep_fail(false);
  std::atomic<bool> rep_internal(false);

  std::error_code err = cool::ng::error::no_error();

  auto r = std::make_shared<test_runner>();

  auto clt_stream = std::make_shared<async::net::stream>(
        std::weak_ptr<test_runner>(r)
      , [] (const std::shared_ptr<test_runner>&, void*& b_, std::size_t& s_) { }
      , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)   { }
      , [&rep_conn, &rep_disconn, &rep_fail, &rep_internal, &err] (const std::shared_ptr<test_runner>& r_, oob_event evt_, const std::error_code& e_)
        {
          std::cout << "got event " << static_cast<int>(evt_) << ", code " << e_.value() << " (" << e_.message() << ")\n";
          switch (evt_)
          {
            case oob_event::connect:
              rep_conn = true; break;
            case oob_event::disconnect:
              rep_disconn = true; break;
            case oob_event::failure:
              rep_fail = true; err = e_; break;
            case oob_event::internal:
              rep_internal = true; err = e_; break;

          }
        }
    );

  // attempt to no service
  rep_conn = false;
  rep_disconn = false;
  rep_fail = false;
  err = cool::ng::error::no_error();

  clt_stream->connect(ipv4::loopback, 10101);
  spin_wait(3000, [&rep_fail] () { return rep_fail.load(); });
  BOOST_CHECK_EQUAL(false, rep_conn.load());
  BOOST_CHECK_EQUAL(false, rep_disconn.load());
  BOOST_CHECK_EQUAL(true, rep_fail.load());
  BOOST_CHECK_EQUAL(static_cast<int>(errc::request_failed), err.value());

  // discard client
  clt_stream.reset();
  spin_wait(100, [] () { return false; });

}
#endif

// this test tries to connect to  unreachable host (may have to disable network interface)
#if TEST12 == 1
BOOST_AUTO_TEST_CASE(destination_unreachable)
{
  check_start_sockets();
  std::atomic<bool> rep_conn(false);
  std::atomic<bool> rep_disconn(false);
  std::atomic<bool> rep_fail(false);

  std::error_code err = cool::ng::error::no_error();

  auto r = std::make_shared<test_runner>();

  auto clt_stream = std::make_shared<async::net::stream>(
        std::weak_ptr<test_runner>(r)
      , [] (const std::shared_ptr<test_runner>&, void*& b_, std::size_t& s_) { }
      , [] (const std::shared_ptr<test_runner>&, const void*, std::size_t)   { }
      , [&rep_conn, &rep_disconn, &rep_fail, &err] (const std::shared_ptr<test_runner>& r_, oob_event evt_, const std::error_code& e_)
        {
          std::cout << "got event " << static_cast<int>(evt_) << ", code " << e_.value() << " (" << e_.message() << ")\n";
          switch (evt_)
          {
            case oob_event::connect:
              rep_conn = true; break;
            case oob_event::disconnect:
              rep_disconn = true; break;
            case oob_event::failure:
              rep_fail = true; err = e_; break;
          }
        }
    );
  // attempt to unreachable host
  rep_conn = false;
  rep_disconn = false;
  rep_fail = false;
  errc = cool::ng::error::no_error();

  clt_stream->connect(ipv4::host("192.168.14.3"), 12345);
  spin_wait(3000, [&rep_fail] () { return rep_fail.load(); });
  BOOST_CHECK_EQUAL(false, rep_conn.load());
  BOOST_CHECK_EQUAL(false, rep_disconn.load());
  BOOST_CHECK_EQUAL(true, rep_fail.load());
  BOOST_CHECK_EQUAL(static_cast<int>(errc::request_failed), err.value());

  // discard client
  clt_stream.reset();
  spin_wait(100, [] () { return false; });

}
#endif

BOOST_AUTO_TEST_SUITE_END()




