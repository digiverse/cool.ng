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
#include <typeinfo>
#include <memory>
#include <stack>
#include <functional>
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <exception>

#define BOOST_TEST_MODULE InterceptTask
#include <boost/test/unit_test.hpp>

#include "cool/ng/async.h"

using ms = std::chrono::milliseconds;

BOOST_AUTO_TEST_SUITE(intercept_task)


class my_runner : public cool::ng::async::runner
{
 public:
  void inc() { ++counter; }

  int counter = 0;
};

class abc { };
class cde { };
class fgh { };

BOOST_AUTO_TEST_CASE(basic_and_nexted)
{
  auto runner = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&, int value) -> int
      {
        if ( value == 5)
          throw abc();
        else if (value == 6)
          throw cde();
        else if (value == 7)
          throw fgh();

        counter = 21;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return counter + 1;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&, const abc& e) -> int
      {
        counter = 42;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return counter + 1;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&, const cde& e) -> int
      {
        counter = 84;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return counter + 1;
      }
  );

  auto t4 = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&, const fgh& e) -> int
      {
        counter = 168;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return counter + 1;
      }
  );

  auto task = cool::ng::async::factory::try_catch(
      cool::ng::async::factory::try_catch(t1, t2, t3)
    , t4);

  std::unique_lock<std::mutex> l(m);
  task.run(5);
  cv.wait_for(l, ms(100), [&counter] { return counter == 42; });
  BOOST_CHECK_EQUAL(42, counter);

  task.run(6);
  cv.wait_for(l, ms(100), [&counter] { return counter == 84; });
  BOOST_CHECK_EQUAL(84, counter);

  task.run(8);
  cv.wait_for(l, ms(100), [&counter] { return counter == 21; });
  BOOST_CHECK_EQUAL(21, counter);

  task.run(7);
  cv.wait_for(l, ms(100), [&counter] { return counter == 168; });
  BOOST_CHECK_EQUAL(168, counter);
}

BOOST_AUTO_TEST_CASE(catch_all)
{
  auto runner = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&, int value) -> int
      {
        if ( value == 5)
          throw abc();
        else if (value == 6)
          throw cde();
        else if (value == 7)
          throw fgh();

        counter = 21;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return counter + 1;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&, const abc& e) -> int
      {
        counter = 42;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return counter + 1;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&, const std::exception_ptr& e) -> int
      {
        counter = 84;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return counter + 1;
      }
  );

  auto t4 = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&, const fgh& e) -> int
      {
        counter = 168;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return counter + 1;
      }
  );

  auto task = cool::ng::async::factory::try_catch(
      cool::ng::async::factory::try_catch(t1, t2, t3)
    , t4);

  std::unique_lock<std::mutex> l(m);
  task.run(5);
  cv.wait_for(l, ms(100), [&counter] { return counter == 42; });
  BOOST_CHECK_EQUAL(42, counter);

  task.run(6);
  cv.wait_for(l, ms(100), [&counter] { return counter == 84; });
  BOOST_CHECK_EQUAL(84, counter);

  task.run(8);
  cv.wait_for(l, ms(100), [&counter] { return counter == 21; });
  BOOST_CHECK_EQUAL(21, counter);

  task.run(7);
  cv.wait_for(l, ms(100), [&counter] { return counter == 84; });
  BOOST_CHECK_EQUAL(84, counter);
}

BOOST_AUTO_TEST_SUITE_END()
