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

#define BOOST_TEST_MODULE RepeatTask
#include <boost/test/unit_test.hpp>

#include "cool/ng/async.h"

using ms = std::chrono::milliseconds;

BOOST_AUTO_TEST_SUITE(intercept_task)

using lock = std::unique_lock<std::mutex>;

class my_runner : public cool::ng::async::runner
{
 public:
  void inc() { ++counter; }

  int counter = 0;
};

BOOST_AUTO_TEST_CASE(basic_void)
{
  auto runner_1 = std::make_shared<my_runner>();
  auto runner_2 = std::make_shared<my_runner>();

  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  std::size_t result;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner_1
    , [&m, &cv, &counter, &result] (const std::shared_ptr<my_runner>& r, std::size_t value) -> void
      {
        ++counter;
        result = value;
       }
  );

  auto t2 = cool::ng::async::factory::create(
      runner_2
    , [&m, &cv] (const std::shared_ptr<my_runner>& r) -> void
      {
        lock l(m);
        cv.notify_one();
      }
  );

  auto task = cool::ng::async::factory::sequence(cool::ng::async::factory::repeat(t1), t2);

  {
    lock l(m);

    task.run(100);

    cv.wait_for(l, ms(100), [&counter] () { return counter == 100; });
  }

  BOOST_CHECK_EQUAL(100, counter);
  BOOST_CHECK_EQUAL(99, result);
  counter = 0;
  result = 0;

  {
    lock l(m);

    task.run(0);

    cv.wait_for(l, ms(100), [&counter] () { return counter == 100; });
  }

  BOOST_CHECK_EQUAL(0, counter);
  BOOST_CHECK_EQUAL(0, result);
}

BOOST_AUTO_TEST_CASE(basic_non_void)
{
  auto runner_1 = std::make_shared<my_runner>();
  auto runner_2 = std::make_shared<my_runner>();

  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  int result;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner_1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, std::size_t value) -> int
      {
        ++counter;
        return static_cast<int>(value);
      }
  );

  auto t2 = cool::ng::async::factory::create(
      runner_2
    , [&m, &cv, &result] (const std::shared_ptr<my_runner>& r, int input) -> void
      {
        result = input;
        lock l(m);
        cv.notify_one();
      }
  );

  auto task = cool::ng::async::factory::sequence(cool::ng::async::factory::repeat(t1), t2);

  {
    lock l(m);

    task.run(100);

    cv.wait_for(l, ms(100), [&counter] () { return counter == 100; });
  }

  BOOST_CHECK_EQUAL(100, counter);
  BOOST_CHECK_EQUAL(99, result);
  counter = 0;
  result = 0;

  {
    lock l(m);

    task.run(0);

    cv.wait_for(l, ms(100), [&counter] () { return counter == 100; });
  }
  BOOST_CHECK_EQUAL(0, counter);

}

BOOST_AUTO_TEST_SUITE_END()
