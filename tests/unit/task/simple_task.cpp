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

#define BOOST_TEST_MODULE SimpleTask
#include <boost/test/unit_test.hpp>

#include "cool/ng/async.h"

using ms = std::chrono::milliseconds;

BOOST_AUTO_TEST_SUITE(simple_task)


class my_runner : public cool::ng::async::runner
{

};

BOOST_AUTO_TEST_CASE(basic_void_int)
{
  auto runner = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto task = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&, int value)
      {
        counter = value + 1;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
      }
  );

  std::unique_lock<std::mutex> l(m);
  task.run(5);
  cv.wait_for(l, ms(100), [&counter] { return counter == 6; });

  BOOST_CHECK_EQUAL(6, counter);
}

BOOST_AUTO_TEST_CASE(basic_int_int)
{
  auto runner = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto task = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&, int value)
      {
        counter = value + 1;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return value;
      }
  );

  std::unique_lock<std::mutex> l(m);
  task.run(5);
  cv.wait_for(l, ms(100), [&counter] { return counter == 6; });

  BOOST_CHECK_EQUAL(6, counter);
}

BOOST_AUTO_TEST_CASE(basic_int_void)
{
  auto runner = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto task = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&)
      {
        counter = 6;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        return 6;
      }
  );

  std::unique_lock<std::mutex> l(m);
  task.run();
  cv.wait_for(l, ms(100), [&counter] { return counter == 6; });

  BOOST_CHECK_EQUAL(6, counter);
}

BOOST_AUTO_TEST_CASE(basic_void_void)
{
  auto runner = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto task = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&)
      {
        counter = 6;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
      }
  );

  std::unique_lock<std::mutex> l(m);
  task.run();
  cv.wait_for(l, ms(100), [&counter] { return counter == 6; });

  BOOST_CHECK_EQUAL(6, counter);
}

BOOST_AUTO_TEST_CASE(basic_with_exception)
{
  auto runner = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto task = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&, int value)
      {
        counter = value;
        throw std::runtime_error("something");
        ++counter;
      }
  );

  std::unique_lock<std::mutex> l(m);
  task.run(10);
  cv.wait_for(l, ms(100), [&counter] { return counter == 10; });

  BOOST_CHECK_EQUAL(10, counter);
}

class no_copy_ctor
{
  public:
   no_copy_ctor(int v ) { m_value = v; }
   no_copy_ctor(no_copy_ctor&& o) { m_value = o.m_value; o.m_value = 0;}
   no_copy_ctor& operator =(no_copy_ctor&& o) { m_value = o.m_value; o.m_value = 0; return *this; };
   no_copy_ctor(const no_copy_ctor&) = delete;
   no_copy_ctor& operator =(const no_copy_ctor&); // = delete;

   int m_value;
};

BOOST_AUTO_TEST_CASE(rvalue_reference)
{
  auto runner = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  int counter = 0;

  auto task = cool::ng::async::factory::create(
      runner
    , [] (const std::shared_ptr<my_runner>&, no_copy_ctor&& val_)
      {
      }
  );
/*
  auto task2 = cool::ng::async::factory::create(
      runner
    , [] (const std::shared_ptr<my_runner>&, no_copy_ctor val_)
      {
      }
  );
  */
  no_copy_ctor a(42);
  task.run(std::move(a));
}

BOOST_AUTO_TEST_SUITE_END()
