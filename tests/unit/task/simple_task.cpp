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
#include <boost/version.hpp>
#include <boost/test/unit_test.hpp>

#if !defined(COOL_AUTO_TEST_CASE)
#  if BOOST_VERSION < 106200
#    define COOL_AUTO_TEST_CASE(a, b) BOOST_AUTO_TEST_CASE(a)
#  else
#    define COOL_AUTO_TEST_CASE(a, b) BOOST_AUTO_TEST_CASE(a, b)
     namespace utf = boost::unit_test;
#  endif
#endif

#include "cool/ng/async.h"

using ms = std::chrono::milliseconds;


class my_runner : public cool::ng::async::runner
{ };

bool spin_wait(unsigned int msec, const std::function<bool()>& lambda)
{
  auto start = std::chrono::system_clock::now();
  while (!lambda())
  {
    auto now = std::chrono::system_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() >= msec)
      return false;
  }
  return true;
}

BOOST_AUTO_TEST_SUITE(mem)

COOL_AUTO_TEST_CASE(T001,
  * utf::description("single task, single runner, normal execution, check memory leaks")
  * utf::disabled())
{
  try
  {
    std::atomic<int> counter;
    counter = 0;

    cool::ng::async::task<int, void> task;
    auto runner = std::make_shared<my_runner>();

    {
      task = cool::ng::async::factory::create(
          runner
        , [&counter] (const std::shared_ptr<my_runner>&, int value)
          {
            counter = value + 1;
          }
      );
    }

    task.run(5);
    BOOST_CHECK(spin_wait(50000, [&counter] { return counter == 6; }));

  }
  catch (const std::exception& e)
  {
    BOOST_CHECK(false);
  }

  spin_wait(100, [] { return false; });
}


COOL_AUTO_TEST_CASE(T002,
  * utf::description("single task, single runner. runner gone before task exec, check leaks")
  * utf::disabled())
{
  try
  {
    std::atomic<int> counter;
    counter = 0;

    cool::ng::async::task<int, void> task;

    {
      auto runner = std::make_shared<my_runner>();

      task = cool::ng::async::factory::create(
          runner
        , [&counter] (const std::shared_ptr<my_runner>&, int value)
          {
            counter = value + 1;
          }
      );
    }

    // run the task with the runner no longer being around. It should throw
    // an exception and any external tool should report no memory leakage

    task.run(5);
    BOOST_CHECK(false);
  }
  catch(const std::exception& e)
  {
    BOOST_CHECK(true);
  }

  spin_wait(100, [] { return false; });
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(simple)


BOOST_AUTO_TEST_CASE(task_ind_void)
{
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  auto task = cool::ng::async::factory::create(
      runner
    , [&counter] (const std::shared_ptr<my_runner>&, int value)
      {
        counter = value + 1;
      }
  );

  task.run(5);
  spin_wait(100, [&counter] { return counter != 0; });

  BOOST_CHECK_EQUAL(6, counter);
}

BOOST_AUTO_TEST_CASE(basic_int_int)
{
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  auto task = cool::ng::async::factory::create(
      runner
    , [&counter] (const std::shared_ptr<my_runner>&, int value)
      {
        counter = value + 1;
        return value;
      }
  );

  task.run(5);
  spin_wait(100, [&counter] { return counter != 0; });

  BOOST_CHECK_EQUAL(6, counter);
}

BOOST_AUTO_TEST_CASE(basic_int_void)
{
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  auto task = cool::ng::async::factory::create(
      runner
    , [&counter] (const std::shared_ptr<my_runner>&)
      {
        counter = 6;
        return 6;
      }
  );

  task.run();
  spin_wait(100, [&counter] { return counter != 0; });

  BOOST_CHECK_EQUAL(6, counter);
}

BOOST_AUTO_TEST_CASE(basic_void_void)
{
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  auto task = cool::ng::async::factory::create(
      runner
    , [&counter] (const std::shared_ptr<my_runner>&)
      {
        counter = 6;
      }
  );

  task.run();
  spin_wait(100, [&counter] { return counter != 0; });

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
    , [&counter] (const std::shared_ptr<my_runner>&, int value)
      {
        counter = value;
        throw std::runtime_error("something");
        ++counter;
      }
  );

  task.run(10);
  spin_wait(100, [&counter] { return counter != 0; });

  BOOST_CHECK_EQUAL(10, counter);
}

class nocopy
{
  public:
   nocopy(int v ) { m_value = v; }
   nocopy(nocopy&& o) { m_value = o.m_value; o.m_value = 0;}
   nocopy& operator =(nocopy&& o) { m_value = o.m_value; o.m_value = 0; return *this; };
   nocopy(const nocopy&) = delete;
   nocopy& operator =(const nocopy&) = delete;

   int m_value;
};

BOOST_AUTO_TEST_CASE(rvalue_reference_input)
{
  auto runner = std::make_shared<my_runner>();
  int counter = 0;

  auto task = cool::ng::async::factory::create(
      runner
    , [&counter] (const std::shared_ptr<my_runner>&, nocopy&& val_)
      {
        counter = val_.m_value;
      }
  );

  nocopy a(42);
  task.run(std::move(a));
  spin_wait(100, [&counter] { return counter != 0; });
  std::this_thread::sleep_for(ms(10));
  BOOST_CHECK_EQUAL(42, counter);
  std::this_thread::sleep_for(ms(10));
  BOOST_CHECK_EQUAL(42, counter);
}

BOOST_AUTO_TEST_SUITE_END()
