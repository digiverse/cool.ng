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

BOOST_AUTO_TEST_SUITE(conditional)



class abc { };
class cde { };
class fgh { };

COOL_AUTO_TEST_CASE(T001,
  * utf::description("basic conditional task"))
{
  auto runner_1 = std::make_shared<my_runner>();
  auto runner_2 = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  int global_value = 5;

  auto t1 = cool::ng::async::factory::create(
      runner_1
    , [&counter, &global_value] (const std::shared_ptr<my_runner>& r, int value) -> bool
      {
        r->inc();
        counter = 0;
        return global_value == value;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner_2
    , [&counter] (const std::shared_ptr<my_runner>& r, int value) -> int
      {
        r->inc();
        counter += 42 + value;
        return counter;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner_1
    , [&counter] (const std::shared_ptr<my_runner>& r, int value) -> int
      {
        r->inc();
        counter += 84 + value;
        return counter;
      }
  );

  auto task = cool::ng::async::factory::conditional(t1, t2, t3);

  task.run(5);
  spin_wait(100, [&counter] { return counter != 0; });
  BOOST_CHECK_EQUAL(47, counter);
  BOOST_CHECK_EQUAL(1, runner_1->counter);
  BOOST_CHECK_EQUAL(1, runner_2->counter);

  task.run(10);
  spin_wait(100, [&counter] { return counter == 94; });
  BOOST_CHECK_EQUAL(94, counter);
  BOOST_CHECK_EQUAL(3, runner_1->counter);
  BOOST_CHECK_EQUAL(1, runner_2->counter);
}

COOL_AUTO_TEST_CASE(T002,
  * utf::description("basic conditional task, no input"))
{
  auto runner_1 = std::make_shared<my_runner>();
  auto runner_2 = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner_1
    , [&counter] (const std::shared_ptr<my_runner>& r) -> bool
      {
        r->inc();
        return counter == 5;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner_2
    , [&counter] (const std::shared_ptr<my_runner>& r) -> int
      {
        r->inc();
        counter += 42;
        return counter;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner_1
    , [&counter] (const std::shared_ptr<my_runner>& r) -> int
      {
        r->inc();
        counter += 84;
        return counter;
      }
  );

  auto task = cool::ng::async::factory::conditional(t1, t2, t3);

  counter = 5;
  task.run();
  spin_wait(100, [&counter] { return counter != 5; });
  BOOST_CHECK_EQUAL(47, counter);
  BOOST_CHECK_EQUAL(1, runner_1->counter);
  BOOST_CHECK_EQUAL(1, runner_2->counter);

  counter = 10;
  task.run();
  spin_wait(100, [&counter] { return counter != 10; });
  BOOST_CHECK_EQUAL(94, counter);
  BOOST_CHECK_EQUAL(3, runner_1->counter);
  BOOST_CHECK_EQUAL(1, runner_2->counter);

  auto t4 = cool::ng::async::factory::create(
      runner_2
    , [&counter] (const std::shared_ptr<my_runner>& r, int value) -> int
      {
        r->inc();
        counter += value - 5;
        return counter;
      }
  );

  auto task2 = cool::ng::async::factory::sequence(task, t4);

  counter = 5;
  task2.run();
  spin_wait(100, [&counter] { return counter == 89; });
  BOOST_CHECK_EQUAL(89, counter);
  BOOST_CHECK_EQUAL(4, runner_1->counter);
  BOOST_CHECK_EQUAL(3, runner_2->counter);

  counter = 10;
  task2.run();
  spin_wait(100, [&counter] { return counter == 183; });
  BOOST_CHECK_EQUAL(183, counter);
  BOOST_CHECK_EQUAL(6, runner_1->counter);
  BOOST_CHECK_EQUAL(4, runner_2->counter);
}

COOL_AUTO_TEST_CASE(T003,
  * utf::description("basic conditional task, no 'else' part"))
{
  auto runner_1 = std::make_shared<my_runner>();
  auto runner_2 = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner_1
    , [&counter] (const std::shared_ptr<my_runner>& r, int value) -> bool
      {
        r->inc();
        counter = 0;
        return value == 5;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner_2
    , [&counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        counter += 42 + value;
      }
  );

  auto t4 = cool::ng::async::factory::create(
      runner_2
    , [&counter] (const std::shared_ptr<my_runner>& r)
      {
        r->inc();
        counter += 100;
      }
  );

  auto task = cool::ng::async::factory::sequence(
      cool::ng::async::factory::conditional(t1, t2)
    , t4);

  task.run(5);
  spin_wait(100, [&counter] { return counter == 147; });
  BOOST_CHECK_EQUAL(147, counter);
  BOOST_CHECK_EQUAL(1, runner_1->counter);
  BOOST_CHECK_EQUAL(2, runner_2->counter);

  task.run(10);
  spin_wait(100, [&counter] { return counter == 100; });

  BOOST_CHECK_EQUAL(100, counter);
  BOOST_CHECK_EQUAL(2, runner_1->counter);
  BOOST_CHECK_EQUAL(3, runner_2->counter);

}

COOL_AUTO_TEST_CASE(T004,
  * utf::description("basic conditional task, no input parameter, no else part"))
{
  auto runner_1 = std::make_shared<my_runner>();
  auto runner_2 = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner_1
    , [&counter] (const std::shared_ptr<my_runner>& r) -> bool
      {
        r->inc();
        return counter == 5;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner_2
    , [&counter] (const std::shared_ptr<my_runner>& r)
      {
        r->inc();
        counter += 42;
      }
  );

  auto t4 = cool::ng::async::factory::create(
      runner_2
    , [&counter] (const std::shared_ptr<my_runner>& r)
      {
        r->inc();
        counter += 100;
      }
  );

  auto task = cool::ng::async::factory::sequence(
      cool::ng::async::factory::conditional(t1, t2)
    , t4);

  counter = 5;
  task.run();
  spin_wait(100, [&counter] { return counter == 147; });
  BOOST_CHECK_EQUAL(147, counter);
  BOOST_CHECK_EQUAL(1, runner_1->counter);
  BOOST_CHECK_EQUAL(2, runner_2->counter);

  counter = 0;
  task.run();

  spin_wait(100, [&counter] { return counter == 100; });

  BOOST_CHECK_EQUAL(100, counter);
  BOOST_CHECK_EQUAL(2, runner_1->counter);
  BOOST_CHECK_EQUAL(3, runner_2->counter);

}

COOL_AUTO_TEST_CASE(T100,
  * utf::description("custom struct with both move and copy ctor, ctor and dtor counters"))
{
  // Part A-1: both then/else tasks, predicate true
  {
    counted_moveable::clear();

    {
      counted_moveable a(2, 3);
      auto runner = std::make_shared<my_runner>();
      std::atomic<int> counter;
      std::atomic<int> sum;
      std::atomic<int> path;  // 0 - none, 1 - then, 2 - else
      counter = 0;
      sum = 0;
      path = 0;

      cool::ng::async::factory::conditional(
          cool::ng::async::factory::create(
              runner
            , [](const std::shared_ptr<my_runner>& self, const counted_moveable& v)
              {
                return true;
              }
          )
        , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, const counted_moveable& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 1;
              }
           )
         , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, const counted_moveable& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 2;
              }
          )
     ).run(a);

      spin_wait(100, [&counter] { return counter != 0; });
      BOOST_CHECK_EQUAL(2, counter);
      BOOST_CHECK_EQUAL(5, sum);
      BOOST_CHECK_EQUAL(1, path);
    }

    std::this_thread::sleep_for(ms(50));
    BOOST_CHECK_EQUAL(7, counted_moveable::instance_counter);
    BOOST_CHECK_EQUAL(3, counted_moveable::cnt_move_ctor);
    BOOST_CHECK_EQUAL(3, counted_moveable::cnt_copy_ctor);
    BOOST_CHECK_EQUAL(0, counted_moveable::cnt_def_ctor);
    BOOST_CHECK_EQUAL(1, counted_moveable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(7, counted_moveable::cnt_dtor);

    BOOST_CHECK_EQUAL(counted_moveable::instance_counter, counted_moveable::cnt_move_ctor + counted_moveable::cnt_def_ctor + counted_moveable::cnt_copy_ctor + counted_moveable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(counted_moveable::cnt_dtor, counted_moveable::cnt_move_ctor + counted_moveable::cnt_def_ctor + counted_moveable::cnt_copy_ctor + counted_moveable::cnt_el_ctor);
  }
  // Part A-2: both then/else tasks, predicate false
  {
    counted_moveable::clear();

    {
      counted_moveable a(2, 3);
      auto runner = std::make_shared<my_runner>();
      std::atomic<int> counter;
      std::atomic<int> sum;
      std::atomic<int> path;  // 0 - none, 1 - then, 2 - else
      counter = 0;
      sum = 0;
      path = 0;

      cool::ng::async::factory::conditional(
          cool::ng::async::factory::create(
              runner
            , [](const std::shared_ptr<my_runner>& self, const counted_moveable& v)
              {
                return false;
              }
          )
        , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, const counted_moveable& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 1;
              }
           )
         , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, const counted_moveable& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 2;
              }
          )
     ).run(a);

      spin_wait(100, [&counter] { return counter != 0; });
      BOOST_CHECK_EQUAL(2, counter);
      BOOST_CHECK_EQUAL(5, sum);
      BOOST_CHECK_EQUAL(2, path);
    }

    std::this_thread::sleep_for(ms(50));
    BOOST_CHECK_EQUAL(7, counted_moveable::instance_counter);
    BOOST_CHECK_EQUAL(3, counted_moveable::cnt_move_ctor);
    BOOST_CHECK_EQUAL(3, counted_moveable::cnt_copy_ctor);
    BOOST_CHECK_EQUAL(0, counted_moveable::cnt_def_ctor);
    BOOST_CHECK_EQUAL(1, counted_moveable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(7, counted_moveable::cnt_dtor);

    BOOST_CHECK_EQUAL(counted_moveable::instance_counter, counted_moveable::cnt_move_ctor + counted_moveable::cnt_def_ctor + counted_moveable::cnt_copy_ctor + counted_moveable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(counted_moveable::cnt_dtor, counted_moveable::cnt_move_ctor + counted_moveable::cnt_def_ctor + counted_moveable::cnt_copy_ctor + counted_moveable::cnt_el_ctor);
  }
  // Part B-1: only then task, predicate true
  {
    counted_moveable::clear();

    {
      counted_moveable a(2, 3);
      auto runner = std::make_shared<my_runner>();
      std::atomic<int> counter;
      std::atomic<int> sum;
      std::atomic<int> path;  // 0 - none, 1 - then, 2 - else
      counter = 0;
      sum = 0;
      path = 0;

      cool::ng::async::factory::conditional(
          cool::ng::async::factory::create(
              runner
            , [](const std::shared_ptr<my_runner>& self, const counted_moveable& v)
              {
                return true;
              }
          )
        , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, const counted_moveable& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 1;
              }
           )
     ).run(a);

      spin_wait(100, [&counter] { return counter != 0; });
      BOOST_CHECK_EQUAL(2, counter);
      BOOST_CHECK_EQUAL(5, sum);
      BOOST_CHECK_EQUAL(1, path);
    }

    std::this_thread::sleep_for(ms(50));
    BOOST_CHECK_EQUAL(7, counted_moveable::instance_counter);
    BOOST_CHECK_EQUAL(3, counted_moveable::cnt_move_ctor);
    BOOST_CHECK_EQUAL(3, counted_moveable::cnt_copy_ctor);
    BOOST_CHECK_EQUAL(0, counted_moveable::cnt_def_ctor);
    BOOST_CHECK_EQUAL(1, counted_moveable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(7, counted_moveable::cnt_dtor);

    BOOST_CHECK_EQUAL(counted_moveable::instance_counter, counted_moveable::cnt_move_ctor + counted_moveable::cnt_def_ctor + counted_moveable::cnt_copy_ctor + counted_moveable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(counted_moveable::cnt_dtor, counted_moveable::cnt_move_ctor + counted_moveable::cnt_def_ctor + counted_moveable::cnt_copy_ctor + counted_moveable::cnt_el_ctor);
  }
  // Part B-2: only then task, predicate false
  {
    counted_moveable::clear();

    {
      counted_moveable a(2, 3);
      auto runner = std::make_shared<my_runner>();
      std::atomic<int> counter;
      std::atomic<int> sum;
      std::atomic<int> path;  // 0 - none, 1 - then, 2 - else
      counter = 0;
      sum = 0;
      path = 0;

      cool::ng::async::factory::conditional(
          cool::ng::async::factory::create(
              runner
            , [](const std::shared_ptr<my_runner>& self, const counted_moveable& v)
              {
                return false;
              }
          )
        , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, const counted_moveable& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 1;
              }
           )
     ).run(a);

      spin_wait(50, [&counter] { return counter != 0; });
      BOOST_CHECK_EQUAL(0, counter);
      BOOST_CHECK_EQUAL(0, sum);
      BOOST_CHECK_EQUAL(0, path);
    }

    std::this_thread::sleep_for(ms(50));
    BOOST_CHECK_EQUAL(5, counted_moveable::instance_counter);
    BOOST_CHECK_EQUAL(2, counted_moveable::cnt_move_ctor);
    BOOST_CHECK_EQUAL(2, counted_moveable::cnt_copy_ctor);
    BOOST_CHECK_EQUAL(0, counted_moveable::cnt_def_ctor);
    BOOST_CHECK_EQUAL(1, counted_moveable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(5, counted_moveable::cnt_dtor);

    BOOST_CHECK_EQUAL(counted_moveable::instance_counter, counted_moveable::cnt_move_ctor + counted_moveable::cnt_def_ctor + counted_moveable::cnt_copy_ctor + counted_moveable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(counted_moveable::cnt_dtor, counted_moveable::cnt_move_ctor + counted_moveable::cnt_def_ctor + counted_moveable::cnt_copy_ctor + counted_moveable::cnt_el_ctor);
  }
}

COOL_AUTO_TEST_CASE(T101,
  * utf::description("custom struct w/o move ctor, ctor and dtor counters"))
{
  // Part A-1: both then/else tasks, predicate true
  {
    counted_copyable::clear();

    {
      counted_copyable a(2, 3);
      auto runner = std::make_shared<my_runner>();
      std::atomic<int> counter;
      std::atomic<int> sum;
      std::atomic<int> path;  // 0 - none, 1 - then, 2 - else
      counter = 0;
      sum = 0;
      path = 0;

      cool::ng::async::factory::conditional(
          cool::ng::async::factory::create(
              runner
            , [](const std::shared_ptr<my_runner>& self, const counted_copyable& v)
              {
                return true;
              }
          )
        , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, const counted_copyable& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 1;
              }
           )
         , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, const counted_copyable& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 2;
              }
          )
     ).run(a);

      spin_wait(100, [&counter] { return counter != 0; });
      BOOST_CHECK_EQUAL(2, counter);
      BOOST_CHECK_EQUAL(5, sum);
      BOOST_CHECK_EQUAL(1, path);
    }

    std::this_thread::sleep_for(ms(50));
    BOOST_CHECK_EQUAL(7, counted_copyable::instance_counter);
    BOOST_CHECK_EQUAL(6, counted_copyable::cnt_copy_ctor);
    BOOST_CHECK_EQUAL(0, counted_copyable::cnt_def_ctor);
    BOOST_CHECK_EQUAL(1, counted_copyable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(7, counted_copyable::cnt_dtor);

    BOOST_CHECK_EQUAL(counted_copyable::instance_counter, counted_copyable::cnt_def_ctor + counted_copyable::cnt_copy_ctor + counted_copyable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(counted_copyable::cnt_dtor, counted_copyable::cnt_def_ctor + counted_copyable::cnt_copy_ctor + counted_copyable::cnt_el_ctor);
  }
  // Part A-2: both then/else tasks, predicate false
  {
    counted_copyable::clear();

    {
      counted_copyable a(2, 3);
      auto runner = std::make_shared<my_runner>();
      std::atomic<int> counter;
      std::atomic<int> sum;
      std::atomic<int> path;  // 0 - none, 1 - then, 2 - else
      counter = 0;
      sum = 0;
      path = 0;

      cool::ng::async::factory::conditional(
          cool::ng::async::factory::create(
              runner
            , [](const std::shared_ptr<my_runner>& self, const counted_copyable& v)
              {
                return false;
              }
          )
        , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, const counted_copyable& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 1;
              }
           )
         , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, const counted_copyable& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 2;
              }
          )
     ).run(a);

      spin_wait(100, [&counter] { return counter != 0; });
      BOOST_CHECK_EQUAL(2, counter);
      BOOST_CHECK_EQUAL(5, sum);
      BOOST_CHECK_EQUAL(2, path);
    }

    std::this_thread::sleep_for(ms(50));
    BOOST_CHECK_EQUAL(7, counted_copyable::instance_counter);
    BOOST_CHECK_EQUAL(6, counted_copyable::cnt_copy_ctor);
    BOOST_CHECK_EQUAL(0, counted_copyable::cnt_def_ctor);
    BOOST_CHECK_EQUAL(1, counted_copyable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(7, counted_copyable::cnt_dtor);

    BOOST_CHECK_EQUAL(counted_copyable::instance_counter, counted_copyable::cnt_def_ctor + counted_copyable::cnt_copy_ctor + counted_copyable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(counted_copyable::cnt_dtor, counted_copyable::cnt_def_ctor + counted_copyable::cnt_copy_ctor + counted_copyable::cnt_el_ctor);
  }
  // Part B-1: only then task, predicate true
  {
    counted_copyable::clear();

    {
      counted_copyable a(2, 3);
      auto runner = std::make_shared<my_runner>();
      std::atomic<int> counter;
      std::atomic<int> sum;
      std::atomic<int> path;  // 0 - none, 1 - then, 2 - else
      counter = 0;
      sum = 0;
      path = 0;

      cool::ng::async::factory::conditional(
          cool::ng::async::factory::create(
              runner
            , [](const std::shared_ptr<my_runner>& self, const counted_copyable& v)
              {
                return true;
              }
          )
        , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, const counted_copyable& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 1;
              }
           )
     ).run(a);

      spin_wait(100, [&counter] { return counter != 0; });
      BOOST_CHECK_EQUAL(2, counter);
      BOOST_CHECK_EQUAL(5, sum);
      BOOST_CHECK_EQUAL(1, path);
    }

    std::this_thread::sleep_for(ms(50));
    BOOST_CHECK_EQUAL(7, counted_copyable::instance_counter);
    BOOST_CHECK_EQUAL(6, counted_copyable::cnt_copy_ctor);
    BOOST_CHECK_EQUAL(0, counted_copyable::cnt_def_ctor);
    BOOST_CHECK_EQUAL(1, counted_copyable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(7, counted_copyable::cnt_dtor);

    BOOST_CHECK_EQUAL(counted_copyable::instance_counter, counted_copyable::cnt_def_ctor + counted_copyable::cnt_copy_ctor + counted_copyable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(counted_copyable::cnt_dtor, counted_copyable::cnt_def_ctor + counted_copyable::cnt_copy_ctor + counted_copyable::cnt_el_ctor);
  }
  // Part B-2: only then task, predicate false
  {
    counted_copyable::clear();

    {
      counted_copyable a(2, 3);
      auto runner = std::make_shared<my_runner>();
      std::atomic<int> counter;
      std::atomic<int> sum;
      std::atomic<int> path;  // 0 - none, 1 - then, 2 - else
      counter = 0;
      sum = 0;
      path = 0;

      cool::ng::async::factory::conditional(
          cool::ng::async::factory::create(
              runner
            , [](const std::shared_ptr<my_runner>& self, const counted_copyable& v)
              {
                return false;
              }
          )
        , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, const counted_copyable& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 1;
              }
           )
     ).run(a);

      spin_wait(50, [&counter] { return counter != 0; });
      BOOST_CHECK_EQUAL(0, counter);
      BOOST_CHECK_EQUAL(0, sum);
      BOOST_CHECK_EQUAL(0, path);
    }

    std::this_thread::sleep_for(ms(50));
    BOOST_CHECK_EQUAL(5, counted_copyable::instance_counter);
    BOOST_CHECK_EQUAL(4, counted_copyable::cnt_copy_ctor);
    BOOST_CHECK_EQUAL(0, counted_copyable::cnt_def_ctor);
    BOOST_CHECK_EQUAL(1, counted_copyable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(5, counted_copyable::cnt_dtor);

    BOOST_CHECK_EQUAL(counted_copyable::instance_counter, counted_copyable::cnt_def_ctor + counted_copyable::cnt_copy_ctor + counted_copyable::cnt_el_ctor);
    BOOST_CHECK_EQUAL(counted_copyable::cnt_dtor, counted_copyable::cnt_def_ctor + counted_copyable::cnt_copy_ctor + counted_copyable::cnt_el_ctor);
  }
}

COOL_AUTO_TEST_CASE(T102,
  * utf::description("custom struct with move ctor only, ctor and dtor counters"))
{
 // Part A-1: both then/else tasks, predicate true
  {
    counted_moveonly::clear();

    {
      counted_moveonly a(2, 3);
      auto runner = std::make_shared<my_runner>();
      std::atomic<int> counter;
      std::atomic<int> sum;
      std::atomic<int> path;  // 0 - none, 1 - then, 2 - else
      counter = 0;
      sum = 0;
      path = 0;

      cool::ng::async::factory::conditional(
          cool::ng::async::factory::create(
              runner
            , [](const std::shared_ptr<my_runner>& self, counted_moveonly&& v)
              {
                return true;
              }
          )
        , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, counted_moveonly&& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 1;
              }
           )
         , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, counted_moveonly&& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 2;
              }
          )
     ).run(std::move(a));

      spin_wait(100, [&counter] { return counter != 0; });
      BOOST_CHECK_EQUAL(2, counter);
      BOOST_CHECK_EQUAL(5, sum);
      BOOST_CHECK_EQUAL(1, path);
    }

    std::this_thread::sleep_for(ms(50));
    BOOST_CHECK_EQUAL(5, counted_moveonly::instance_counter);
    BOOST_CHECK_EQUAL(4, counted_moveonly::cnt_move_ctor);
    BOOST_CHECK_EQUAL(0, counted_moveonly::cnt_def_ctor);
    BOOST_CHECK_EQUAL(1, counted_moveonly::cnt_el_ctor);
    BOOST_CHECK_EQUAL(5, counted_moveonly::cnt_dtor);

    BOOST_CHECK_EQUAL(counted_moveonly::instance_counter, counted_moveonly::cnt_move_ctor + counted_moveonly::cnt_def_ctor + counted_moveonly::cnt_el_ctor);
    BOOST_CHECK_EQUAL(counted_moveonly::cnt_dtor, counted_moveonly::cnt_move_ctor + counted_moveonly::cnt_def_ctor + counted_moveonly::cnt_el_ctor);
  }
  // Part A-2: both then/else tasks, predicate false
  {
    counted_moveonly::clear();

    {
      counted_moveonly a(2, 3);
      auto runner = std::make_shared<my_runner>();
      std::atomic<int> counter;
      std::atomic<int> sum;
      std::atomic<int> path;  // 0 - none, 1 - then, 2 - else
      counter = 0;
      sum = 0;
      path = 0;

      cool::ng::async::factory::conditional(
          cool::ng::async::factory::create(
              runner
            , [](const std::shared_ptr<my_runner>& self, counted_moveonly&& v)
              {
                return false;
              }
          )
        , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, counted_moveonly&& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 1;
              }
           )
         , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, counted_moveonly&& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 2;
              }
          )
     ).run(std::move(a));

      spin_wait(100, [&counter] { return counter != 0; });
      BOOST_CHECK_EQUAL(2, counter);
      BOOST_CHECK_EQUAL(5, sum);
      BOOST_CHECK_EQUAL(2, path);
    }

    std::this_thread::sleep_for(ms(50));
    BOOST_CHECK_EQUAL(5, counted_moveonly::instance_counter);
    BOOST_CHECK_EQUAL(4, counted_moveonly::cnt_move_ctor);
    BOOST_CHECK_EQUAL(0, counted_moveonly::cnt_def_ctor);
    BOOST_CHECK_EQUAL(1, counted_moveonly::cnt_el_ctor);
    BOOST_CHECK_EQUAL(5, counted_moveonly::cnt_dtor);

    BOOST_CHECK_EQUAL(counted_moveonly::instance_counter, counted_moveonly::cnt_move_ctor + counted_moveonly::cnt_def_ctor + counted_moveonly::cnt_el_ctor);
    BOOST_CHECK_EQUAL(counted_moveonly::cnt_dtor, counted_moveonly::cnt_move_ctor + counted_moveonly::cnt_def_ctor + counted_moveonly::cnt_el_ctor);
  }
  // Part B-1: only then task, predicate true
  {
    counted_moveonly::clear();

    {
      counted_moveonly a(2, 3);
      auto runner = std::make_shared<my_runner>();
      std::atomic<int> counter;
      std::atomic<int> sum;
      std::atomic<int> path;  // 0 - none, 1 - then, 2 - else
      counter = 0;
      sum = 0;
      path = 0;

      cool::ng::async::factory::conditional(
          cool::ng::async::factory::create(
              runner
            , [](const std::shared_ptr<my_runner>& self, counted_moveonly&& v)
              {
                return true;
              }
          )
        , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, counted_moveonly&& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 1;
              }
           )
     ).run(std::move(a));

      spin_wait(100, [&counter] { return counter != 0; });
      BOOST_CHECK_EQUAL(2, counter);
      BOOST_CHECK_EQUAL(5, sum);
      BOOST_CHECK_EQUAL(1, path);
    }

    std::this_thread::sleep_for(ms(50));
    BOOST_CHECK_EQUAL(5, counted_moveonly::instance_counter);
    BOOST_CHECK_EQUAL(4, counted_moveonly::cnt_move_ctor);
    BOOST_CHECK_EQUAL(0, counted_moveonly::cnt_def_ctor);
    BOOST_CHECK_EQUAL(1, counted_moveonly::cnt_el_ctor);
    BOOST_CHECK_EQUAL(5, counted_moveonly::cnt_dtor);

    BOOST_CHECK_EQUAL(counted_moveonly::instance_counter, counted_moveonly::cnt_move_ctor + counted_moveonly::cnt_def_ctor + counted_moveonly::cnt_el_ctor);
    BOOST_CHECK_EQUAL(counted_moveonly::cnt_dtor, counted_moveonly::cnt_move_ctor + counted_moveonly::cnt_def_ctor + counted_moveonly::cnt_el_ctor);
  }
  // Part B-2: only then task, predicate false
  {
    counted_moveonly::clear();

    {
      counted_moveonly a(2, 3);
      auto runner = std::make_shared<my_runner>();
      std::atomic<int> counter;
      std::atomic<int> sum;
      std::atomic<int> path;  // 0 - none, 1 - then, 2 - else
      counter = 0;
      sum = 0;
      path = 0;

      cool::ng::async::factory::conditional(
          cool::ng::async::factory::create(
              runner
            , [](const std::shared_ptr<my_runner>& self, counted_moveonly&& v)
              {
                return false;
              }
          )
        , cool::ng::async::factory::create(
              runner
            , [&counter, &sum, &path](const std::shared_ptr<my_runner>& self, counted_moveonly&& v)
              {
                counter = v.size();
                for (auto&& item : v)
                  sum += item;

                path = 1;
              }
           )
     ).run(std::move(a));

      spin_wait(50, [&counter] { return counter != 0; });
      BOOST_CHECK_EQUAL(0, counter);
      BOOST_CHECK_EQUAL(0, sum);
      BOOST_CHECK_EQUAL(0, path);
    }

    std::this_thread::sleep_for(ms(50));
    BOOST_CHECK_EQUAL(4, counted_moveonly::instance_counter);
    BOOST_CHECK_EQUAL(3, counted_moveonly::cnt_move_ctor);
    BOOST_CHECK_EQUAL(0, counted_moveonly::cnt_def_ctor);
    BOOST_CHECK_EQUAL(1, counted_moveonly::cnt_el_ctor);
    BOOST_CHECK_EQUAL(4, counted_moveonly::cnt_dtor);

    BOOST_CHECK_EQUAL(counted_moveonly::instance_counter, counted_moveonly::cnt_move_ctor + counted_moveonly::cnt_def_ctor + counted_moveonly::cnt_el_ctor);
    BOOST_CHECK_EQUAL(counted_moveonly::cnt_dtor, counted_moveonly::cnt_move_ctor + counted_moveonly::cnt_def_ctor + counted_moveonly::cnt_el_ctor);
  }
}



BOOST_AUTO_TEST_SUITE_END()
