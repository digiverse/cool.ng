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

#include "task_common.h"

BOOST_AUTO_TEST_SUITE(repeat)

using lock = std::unique_lock<std::mutex>;

COOL_AUTO_TEST_CASE(T001,
  * utf::description("basic repeat with task<std::size_t, void>"))
{
  auto runner_1 = std::make_shared<my_runner>();
  auto runner_2 = std::make_shared<my_runner>();

  std::atomic<int> counter;
  std::atomic<int> result;
  std::atomic<bool> done;
  counter = 0;
  result = 0;
  done = false;

  auto t1 = cool::ng::async::factory::create(
      runner_1
    , [&counter, &result] (const std::shared_ptr<my_runner>& r, std::size_t value) -> void
      {
        result = value;
        ++counter;
      }
  );

  auto t2 = cool::ng::async::factory::create(
      runner_2
    , [&done] (const std::shared_ptr<my_runner>& r) -> void
      {
        done =  true;
      }
  );

  auto task = cool::ng::async::factory::sequence(cool::ng::async::factory::repeat(t1), t2);

  task.run(100);
  spin_wait(100, [&done]() -> bool { return done; });
  BOOST_CHECK_EQUAL(100, counter);
  BOOST_CHECK_EQUAL(99, result);

  counter = 0;
  result = 0;
  done = false;

  task.run(0);
  spin_wait(100, [&done]() -> bool { return done; });

  BOOST_CHECK_EQUAL(0, counter);
  BOOST_CHECK_EQUAL(0, result);
}

COOL_AUTO_TEST_CASE(T002,
  * utf::description("basic repeat with task<std::size_t, non-void>"))
{
  auto runner_1 = std::make_shared<my_runner>();
  auto runner_2 = std::make_shared<my_runner>();

  std::atomic<int> counter;
  std::atomic<int> result;
  std::atomic<bool> done;
  counter = 0;
  result = 0;
  done = false;

  auto t1 = cool::ng::async::factory::create(
      runner_1
    , [&counter] (const std::shared_ptr<my_runner>& r, std::size_t value) -> int
      {
        ++counter;
        return static_cast<int>(value);
      }
  );

  auto t2 = cool::ng::async::factory::create(
      runner_2
    , [&done, &result] (const std::shared_ptr<my_runner>& r, int input) -> void
      {
        result = input;
        done = true;
      }
  );

  auto task = cool::ng::async::factory::sequence(cool::ng::async::factory::repeat(t1), t2);

  task.run(100);
  spin_wait(100, [&done]() -> bool { return done; });
  BOOST_CHECK_EQUAL(100, counter);
  BOOST_CHECK_EQUAL(99, result);
  counter = 0;
  result = 0;


  task.run(0);
  spin_wait(100, [&done]() -> bool { return done; });

  BOOST_CHECK_EQUAL(0, counter);
  BOOST_CHECK_EQUAL(0, result);

}

BOOST_AUTO_TEST_SUITE_END()
