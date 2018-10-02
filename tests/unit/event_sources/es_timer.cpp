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


#define BOOST_TEST_MODULE TimerEventSources
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

BOOST_AUTO_TEST_SUITE(timer_sources)

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
  std::atomic<int> m_counter;
};

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


#if 1
BOOST_AUTO_TEST_CASE(basic)
{
  auto r1 = std::make_shared<test_runner>();
  auto r2 = std::make_shared<test_runner>();

  {
    auto t = cool::ng::async::factory::create(
        r1
      , [] (const std::shared_ptr<test_runner>& r)
        {
          r->inc();
        }
    );

    async::timer timer(t, ms(100));

    spin_wait(150, [&r1] () { return r1->counter() == 1; });
    BOOST_CHECK_EQUAL(0 , r1->counter());

    timer.start();
    spin_wait(250, [&r1] () { return r1->counter() == 2; });
    timer.stop();

    BOOST_CHECK_EQUAL(2, r1->counter());

    timer.period(ms(200));
    spin_wait(250, [&r1] () { return r1->counter() == 1; });
    BOOST_CHECK_EQUAL(2 , r1->counter());

    timer.start();
    spin_wait(300, [&r1] () { return r1->counter() == 3; });
    timer.stop();

    BOOST_CHECK_EQUAL(3, r1->counter());
  }
  std::this_thread::sleep_for(ms(200)); // give time for cleanup
}
#endif


BOOST_AUTO_TEST_SUITE_END()




