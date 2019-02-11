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

using ms = std::chrono::milliseconds;


class my_other_runner : public cool::ng::async::runner
{
 public:
  my_other_runner(bool& flag) : m_on_destroy(flag) { }
  ~my_other_runner() { m_on_destroy = true; }
  bool& m_on_destroy;
};


BOOST_AUTO_TEST_SUITE(simple_memory)

COOL_AUTO_TEST_CASE(T001,
  * utf::description("Windows specific test: single task, single runner, normal execution, check memory leaks")
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
  * utf::description("Windows specific test: single task, single runner. runner gone before task exec, check leaks")
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

class my_task
{
 public:
  my_task(std::atomic<int>& cnt, int& rcnt) : m_count(cnt), m_run_count(rcnt) { }
  ~my_task() { ++m_count; }
  void operator () (const std::shared_ptr<my_other_runner>& self)
  {
    ++m_run_count;
  }

  std::atomic<int>& m_count;
  int&              m_run_count;
};

COOL_AUTO_TEST_CASE(T003,
  * utf::description("Windows specific test: multiple tasks, one block, destroy runner must produce no leaks")
  * utf::disabled())
{
  {
    int t2_run = 0;
    std::atomic<bool> t1_start;
    std::atomic<bool> t1_unblocked;
    std::atomic<int> t2_deleted;
    bool deleted;
    std::mutex m;
    std::condition_variable cv;

    t1_start = false;
    t1_unblocked = false;
    t2_deleted = 0;

    auto r = std::make_shared<my_other_runner>(deleted);


    // T1
    cool::ng::async::factory::create(
        r
      , [&m, &cv, &t1_start, &t1_unblocked](const std::shared_ptr<my_other_runner>& self)
        {
          t1_start = true;
          std::unique_lock<std::mutex> l(m);
          cv.wait(l, [&t1_unblocked]() { return t1_unblocked.load(); });
        }
    ).run();

    // three times T2
    cool::ng::async::factory::create(r, my_task(t2_deleted, t2_run)).run();
    cool::ng::async::factory::create(r, my_task(t2_deleted, t2_run)).run();
    cool::ng::async::factory::create(r, my_task(t2_deleted, t2_run)).run();

    // T2 delete count is > 0 because of copy construction so remember it
    auto current_t2_delete_count = t2_deleted.load();

    // now remove runner - note that T1 is still holding reference
    r.reset();

    // there is still one blocked task, unblock it now
    {
      std::unique_lock<std::mutex> l(m);
      t1_unblocked = true;
      cv.notify_one();
    }

    spin_wait(5000, [] { return false; });

    BOOST_CHECK(deleted);
    BOOST_CHECK_EQUAL(current_t2_delete_count + 3, t2_deleted.load());
    BOOST_CHECK_EQUAL(0, t2_run);

  }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(simple)


COOL_AUTO_TEST_CASE(T001,
  * utf::description("task<int, void>"))
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

COOL_AUTO_TEST_CASE(T002,
  * utf::description("task<int, int>"))
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

COOL_AUTO_TEST_CASE(T003,
  * utf::description("task<void, int>"))
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

COOL_AUTO_TEST_CASE(T004,
 * utf::description("task<void,void>"))
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

COOL_AUTO_TEST_CASE(T005,
  * utf::description("simple task throwing an exception"))
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

COOL_AUTO_TEST_CASE(T006,
  * utf::description("simple task, rvalue reference input"))
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

COOL_AUTO_TEST_CASE(T100,
  * utf::description("custom struct with both move and copy ctor, ctor and dtor counters"))
{
  counted_moveable::clear();

  {
    counted_moveable a(2, 3);
    auto runner = std::make_shared<my_runner>();
    std::atomic<int> counter;
    counter = 0;
    std::atomic<int> sum;
    sum = 0;

    cool::ng::async::factory::create(
        runner
      , [&counter, &sum](const std::shared_ptr<my_runner>& self, const counted_moveable& v)
        {
          counter = v.size();
          for (auto&& item : v)
            sum += item;
        }
    ).run(a);

    spin_wait(100, [&counter] { return counter != 0; });
    BOOST_CHECK_EQUAL(2, counter);
    BOOST_CHECK_EQUAL(5, sum);
  }

  std::this_thread::sleep_for(ms(50));
  BOOST_CHECK_EQUAL(4, counted_moveable::instance_counter);
  BOOST_CHECK_EQUAL(1, counted_moveable::cnt_move_ctor);
  BOOST_CHECK_EQUAL(2, counted_moveable::cnt_copy_ctor);
  BOOST_CHECK_EQUAL(0, counted_moveable::cnt_def_ctor);
  BOOST_CHECK_EQUAL(1, counted_moveable::cnt_el_ctor);
  BOOST_CHECK_EQUAL(4, counted_moveable::cnt_dtor);

  BOOST_CHECK_EQUAL(counted_moveable::instance_counter, counted_moveable::cnt_move_ctor + counted_moveable::cnt_def_ctor + counted_moveable::cnt_copy_ctor + counted_moveable::cnt_el_ctor);
  BOOST_CHECK_EQUAL(counted_moveable::cnt_dtor, counted_moveable::cnt_move_ctor + counted_moveable::cnt_def_ctor + counted_moveable::cnt_copy_ctor + counted_moveable::cnt_el_ctor);
}

COOL_AUTO_TEST_CASE(T101,
  * utf::description("custom struct w/o move ctor, ctor and dtor counters"))
{
  counted_copyable::clear();

  {
    counted_copyable a(2, 3);
    auto runner = std::make_shared<my_runner>();
    std::atomic<int> counter;
    counter = 0;
    std::atomic<int> sum;
    sum = 0;

    cool::ng::async::factory::create(
        runner
      , [&counter, &sum](const std::shared_ptr<my_runner>& self, const counted_copyable& v)
        {
          counter = v.size();
          for (auto&& item : v)
            sum += item;
        }
    ).run(a);

    spin_wait(100, [&counter] { return counter != 0; });
    BOOST_CHECK_EQUAL(2, counter);
    BOOST_CHECK_EQUAL(5, sum);
  }

  std::this_thread::sleep_for(ms(50));
  BOOST_CHECK_EQUAL(4, counted_copyable::instance_counter);
  BOOST_CHECK_EQUAL(3, counted_copyable::cnt_copy_ctor);
  BOOST_CHECK_EQUAL(0, counted_copyable::cnt_def_ctor);
  BOOST_CHECK_EQUAL(1, counted_copyable::cnt_el_ctor);
  BOOST_CHECK_EQUAL(4, counted_copyable::cnt_dtor);

  BOOST_CHECK_EQUAL(counted_copyable::instance_counter, counted_copyable::cnt_def_ctor + counted_copyable::cnt_copy_ctor + counted_copyable::cnt_el_ctor);
  BOOST_CHECK_EQUAL(counted_copyable::cnt_dtor, counted_copyable::cnt_def_ctor + counted_copyable::cnt_copy_ctor + counted_copyable::cnt_el_ctor);
}

COOL_AUTO_TEST_CASE(T102,
  * utf::description("custom struct with move ctor only, ctor and dtor counters"))
{
  counted_moveonly::clear();

  {
    counted_moveonly a(2, 3);
    auto runner = std::make_shared<my_runner>();
    std::atomic<int> counter;
    counter = 0;
    std::atomic<int> sum;
    sum = 0;

    cool::ng::async::factory::create(
        runner
      , [&counter, &sum](const std::shared_ptr<my_runner>& self, counted_moveonly&& v)
        {
          counter = v.size();
          for (auto&& item : v)
            sum += item;
        }
    ).run(std::move(a));

    spin_wait(100, [&counter] { return counter != 0; });
    BOOST_CHECK_EQUAL(2, counter);
    BOOST_CHECK_EQUAL(5, sum);
  }

  std::this_thread::sleep_for(ms(100));
  BOOST_CHECK_EQUAL(3, counted_moveonly::instance_counter);
  BOOST_CHECK_EQUAL(2, counted_moveonly::cnt_move_ctor);
  BOOST_CHECK_EQUAL(0, counted_moveonly::cnt_def_ctor);
  BOOST_CHECK_EQUAL(1, counted_moveonly::cnt_el_ctor);
  BOOST_CHECK_EQUAL(3, counted_moveonly::cnt_dtor);

  BOOST_CHECK_EQUAL(counted_moveonly::instance_counter, counted_moveonly::cnt_move_ctor + counted_moveonly::cnt_def_ctor  + counted_moveonly::cnt_el_ctor);
  BOOST_CHECK_EQUAL(counted_moveonly::cnt_dtor, counted_moveonly::cnt_move_ctor + counted_moveonly::cnt_def_ctor + counted_moveonly::cnt_el_ctor);
}


BOOST_AUTO_TEST_SUITE_END()
