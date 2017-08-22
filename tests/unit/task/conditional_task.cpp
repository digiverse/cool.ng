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

#define BOOST_TEST_MODULE ConditionalTask
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

BOOST_AUTO_TEST_CASE(basic)
{
  auto runner_1 = std::make_shared<my_runner>();
  auto runner_2 = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner_1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value) -> bool
      {
        r->inc();
        counter = 0;
        return value == 5;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner_2
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value) -> int
      {
        r->inc();
        counter += 42 + value;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return counter;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner_1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value) -> int
      {
        r->inc();
        counter += 84 + value;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return counter;
      }
  );

  auto task = cool::ng::async::factory::conditional(t1, t2, t3);

  std::unique_lock<std::mutex> l(m);
  task.run(5);
  cv.wait_for(l, ms(100), [&counter] { return counter == 47; });
  BOOST_CHECK_EQUAL(47, counter);
  BOOST_CHECK_EQUAL(1, runner_1->counter);
  BOOST_CHECK_EQUAL(1, runner_2->counter);

  task.run(10);
  cv.wait_for(l, ms(100), [&counter] { return counter == 94; });
  BOOST_CHECK_EQUAL(94, counter);
  BOOST_CHECK_EQUAL(3, runner_1->counter);
  BOOST_CHECK_EQUAL(1, runner_2->counter);

}

BOOST_AUTO_TEST_CASE(basic_void_param)
{
  auto runner_1 = std::make_shared<my_runner>();
  auto runner_2 = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner_1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r) -> bool
      {
        r->inc();
        return counter == 5;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner_2
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r) -> int
      {
        r->inc();
        counter += 42;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return counter;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner_1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r) -> int
      {
        r->inc();
        counter += 84;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return counter;
      }
  );

  auto task = cool::ng::async::factory::conditional(t1, t2, t3);

  counter = 5;
  std::unique_lock<std::mutex> l(m);
  task.run();
  cv.wait_for(l, ms(100), [&counter] { return counter == 47; });
  BOOST_CHECK_EQUAL(47, counter);
  BOOST_CHECK_EQUAL(1, runner_1->counter);
  BOOST_CHECK_EQUAL(1, runner_2->counter);

  counter = 10;
  task.run();
  cv.wait_for(l, ms(100), [&counter] { return counter == 94; });
  BOOST_CHECK_EQUAL(94, counter);
  BOOST_CHECK_EQUAL(3, runner_1->counter);
  BOOST_CHECK_EQUAL(1, runner_2->counter);

  auto t4 = cool::ng::async::factory::create(
      runner_2
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value) -> int
      {
        r->inc();
        counter += value - 5;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return counter;
      }
  );

  auto task2 = cool::ng::async::factory::sequence(task, t4);

  counter = 5;
  task2.run();
  cv.wait_for(l, ms(100), [&counter] { return counter == 94; });
  BOOST_CHECK_EQUAL(89, counter);
  BOOST_CHECK_EQUAL(4, runner_1->counter);
  BOOST_CHECK_EQUAL(3, runner_2->counter);

  counter = 10;
  task2.run();
  cv.wait_for(l, ms(100), [&counter] { return counter == 183; });
  BOOST_CHECK_EQUAL(183, counter);
  BOOST_CHECK_EQUAL(6, runner_1->counter);
  BOOST_CHECK_EQUAL(4, runner_2->counter);

}

BOOST_AUTO_TEST_CASE(basic_no_else)
{
  auto runner_1 = std::make_shared<my_runner>();
  auto runner_2 = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner_1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value) -> bool
      {
        r->inc();
        counter = 0;
        return value == 5;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner_2
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        counter += 42 + value;
      }
  );

  auto t4 = cool::ng::async::factory::create(
      runner_2
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r)
      {
        r->inc();
        counter += 100;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
      }
  );

  auto task = cool::ng::async::factory::sequence(
      cool::ng::async::factory::conditional(t1, t2)
    , t4);

  std::unique_lock<std::mutex> l(m);
  task.run(5);
  cv.wait_for(l, ms(100), [&counter] { return counter == 147; });
  BOOST_CHECK_EQUAL(147, counter);
  BOOST_CHECK_EQUAL(1, runner_1->counter);
  BOOST_CHECK_EQUAL(2, runner_2->counter);

  task.run(10);

  cv.wait_for(l, ms(100), [&counter] { return counter == 100; });

  BOOST_CHECK_EQUAL(100, counter);
  BOOST_CHECK_EQUAL(2, runner_1->counter);
  BOOST_CHECK_EQUAL(3, runner_2->counter);

}

BOOST_AUTO_TEST_CASE(basic_no_else_void_param)
{
  auto runner_1 = std::make_shared<my_runner>();
  auto runner_2 = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner_1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r) -> bool
      {
        r->inc();
        return counter == 5;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner_2
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r)
      {
        r->inc();
        counter += 42;
      }
  );

  auto t4 = cool::ng::async::factory::create(
      runner_2
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r)
      {
        r->inc();
        counter += 100;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
      }
  );

  auto task = cool::ng::async::factory::sequence(
      cool::ng::async::factory::conditional(t1, t2)
    , t4);

  std::unique_lock<std::mutex> l(m);
  counter = 5;
  task.run();
  cv.wait_for(l, ms(100), [&counter] { return counter == 147; });
  BOOST_CHECK_EQUAL(147, counter);
  BOOST_CHECK_EQUAL(1, runner_1->counter);
  BOOST_CHECK_EQUAL(2, runner_2->counter);

  counter = 0;
  task.run();

  cv.wait_for(l, ms(100), [&counter] { return counter == 100; });

  BOOST_CHECK_EQUAL(100, counter);
  BOOST_CHECK_EQUAL(2, runner_1->counter);
  BOOST_CHECK_EQUAL(3, runner_2->counter);

}

BOOST_AUTO_TEST_SUITE_END()
