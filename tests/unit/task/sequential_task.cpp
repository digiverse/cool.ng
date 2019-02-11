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

#include "task_common.h"

#include "cool/ng/async.h"

using ms = std::chrono::milliseconds;

BOOST_AUTO_TEST_SUITE(sequential)


COOL_AUTO_TEST_CASE(T001,
  * utf::description("basic test with sequence of three tasks"))
{
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner
    , [] (const std::shared_ptr<my_runner>&, int value)
      {
        return value + 1;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner
    , [] (const std::shared_ptr<my_runner>&, int value)
      {
        return value + 1;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner
    , [&counter] (const std::shared_ptr<my_runner>&, int value)
      {
        counter = value + 1;
      }
  );

  auto seq = cool::ng::async::factory::sequence(t1, t2, t3);
  seq.run(5);

  BOOST_CHECK(spin_wait(100, [&counter] { return counter != 0; }));
  BOOST_CHECK_EQUAL(8, counter);
}

COOL_AUTO_TEST_CASE(T002,
  * utf::description("sequence with blocking task, not blocked runner must still run"))
{
  auto runner_a = std::make_shared<my_runner>();
  auto runner_b = std::make_shared<my_runner>();
  std::atomic<int> counter;
  std::atomic<int> counter_b;
  counter = 0;
  counter_b = 0;

  auto t1 = cool::ng::async::factory::create(
      runner_a
    , [] (const std::shared_ptr<my_runner>&, int value)
      {
        return value + 1;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner_b
    , [] (const std::shared_ptr<my_runner>&, int value)
      {
        std::this_thread::sleep_for(ms(500));
        return value + 1;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner_a
    , [&counter] (const std::shared_ptr<my_runner>&, int value)
      {
        counter = value + 1;
      }
  );
  auto t4 = cool::ng::async::factory::create(
      runner_a
    , [&counter_b] (const std::shared_ptr<my_runner>&, int value)
      {
        counter_b = value + 1;
      }
  );

  auto seq = cool::ng::async::factory::sequence(t1, t2, t3);

  seq.run(5);
  t4.run(42);

  // regardless of subtask t2 of task seq blocking runner_b for
  // half a second, runner_a must still be running and should
  // execute task t4 while seq waits for t2 to unblock
  BOOST_CHECK(spin_wait(200, [&counter_b] { return counter_b != 0; }));
  BOOST_CHECK_EQUAL(43, counter_b);

  BOOST_CHECK(spin_wait(1000, [&counter] { return counter != 0; }));
  BOOST_CHECK_EQUAL(8, counter);
}

COOL_AUTO_TEST_CASE(T003,
  *utf::description("basic sequence using two different runners"))
{
  auto runner1 = std::make_shared<my_runner>();
  auto runner2 = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner1
    , [] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        return value + 1;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner2
    , [] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        return value + 1;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner1
    , [&counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        counter = value + 1;
      }
  );

  auto seq = cool::ng::async::factory::sequence(t1, t2, t3);
  seq.run(5);
  spin_wait(100, [&counter] { return counter != 0; });

  BOOST_CHECK_EQUAL(8, counter);
  BOOST_CHECK_EQUAL(2, runner1->counter);
  BOOST_CHECK_EQUAL(1, runner2->counter);
}

COOL_AUTO_TEST_CASE(T004,
  *utf::description("sequence of sequences with reused tasks"))
{
  auto runner1 = std::make_shared<my_runner>();
  auto runner2 = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner1
    , [] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        return value + 1;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner2
    , [] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        return value + 1;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner1
    , [&counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        counter = value + 1;
      }
  );

  auto seq1 = cool::ng::async::factory::sequence(t1, t2);
  auto seq2 = cool::ng::async::factory::sequence(t1, t2, t3);

  auto seq = cool::ng::async::factory::sequence(seq1, seq2);

  seq.run(5);
  spin_wait(100, [&counter] { return counter !=0; });

  BOOST_CHECK_EQUAL(10, counter);
  BOOST_CHECK_EQUAL(3, runner1->counter);
  BOOST_CHECK_EQUAL(2, runner2->counter);
}

COOL_AUTO_TEST_CASE(T005,
 *utf::description("deep sequence (three levels)"))
{
  auto runner1 = std::make_shared<my_runner>();
  auto runner2 = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner1
    , [] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        return value + 1;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner2
    , [] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        return value + 2;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner1
    , [&counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        counter = value + 1;
      }
  );

  cool::ng::async::factory::sequence(
      cool::ng::async::factory::sequence(
          cool::ng::async::factory::sequence(t1, t2)
        , t1
      )
    , t3
  ).run(5);

  spin_wait(100, [&counter] { return counter != 0; });


  BOOST_CHECK_EQUAL(10, counter);
  BOOST_CHECK_EQUAL(3, runner1->counter);
  BOOST_CHECK_EQUAL(1, runner2->counter);

  counter = 0;
  runner1->counter = 0;
  runner2->counter = 0;

  cool::ng::async::factory::sequence(
    t2
    , cool::ng::async::factory::sequence(
          cool::ng::async::factory::sequence(
              cool::ng::async::factory::sequence(t1, t2)
            , t1
          )
        , t2
      )
    , t3
  ).run(5);

  spin_wait(100, [&counter] { return counter != 0; });


  BOOST_CHECK_EQUAL(14, counter);
  BOOST_CHECK_EQUAL(3, runner1->counter);
  BOOST_CHECK_EQUAL(3, runner2->counter);

}

COOL_AUTO_TEST_CASE(T006,
  *utf::description("deep sequence with exception thrown at mid-level"))
{
  auto runner1 = std::make_shared<my_runner>();
  auto runner2 = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner1
    , [] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        return value + 1;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner2
    , [] (const std::shared_ptr<my_runner>& r, int value) -> int
      {
        r->inc();
        throw 2;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner1
    , [&counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        counter = value + 1;
      }
  );

  cool::ng::async::factory::sequence(
    t1
    , cool::ng::async::factory::sequence(
          cool::ng::async::factory::sequence(
              cool::ng::async::factory::sequence(t1, t1)
            , t2
          )
        , t1
      )
    , t3
  ).run(5);

  spin_wait(100, [&counter] { return counter != 0; });


  BOOST_CHECK_EQUAL(0, counter);
  BOOST_CHECK_EQUAL(3, runner1->counter);
  BOOST_CHECK_EQUAL(1, runner2->counter);
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

// Have first task return non-copyable object and the second task accept it
COOL_AUTO_TEST_CASE(T007,
  * utf::description("sequence with first task returnin move-only object and second acception it as input"))
{
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  auto task1 = cool::ng::async::factory::create(
      runner
    , [] (const std::shared_ptr<my_runner>&, int n)
      {
        return nocopy(n);
      }
  );

  auto task2 = cool::ng::async::factory::create(
      runner
    , [&counter] (const std::shared_ptr<my_runner>&, nocopy&& val_)
      {
        counter = val_.m_value;
      }
  );

  auto f_task = cool::ng::async::factory::sequence(task1, task2);

  f_task.run(42);
  spin_wait(100, [&counter] { return counter != 0; });

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
    std::atomic<int> sum;
    std::atomic<bool> done;
    counter = 0;
    sum = 0;
    done = false;

    cool::ng::async::factory::sequence(
        cool::ng::async::factory::create(
            runner
          , [&counter, &sum](const std::shared_ptr<my_runner>& self, const counted_moveable& v)
            {
              counter = v.size();
              for (auto&& item : v)
                sum += item;
            }
          )
        , cool::ng::async::factory::create(
              runner
            , [&done](const std::shared_ptr<my_runner>& self)
              {
                done = true;
              }
          )
    ).run(a);

    spin_wait(100, [&done] { return !!done; });
    BOOST_CHECK_EQUAL(2, counter);
    BOOST_CHECK_EQUAL(5, sum);
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

COOL_AUTO_TEST_CASE(T101,
  * utf::description("custom struct w/o move ctor, ctor and dtor counters"))
{
  counted_copyable::clear();

  {
    counted_copyable a(2, 3);
    auto runner = std::make_shared<my_runner>();
    std::atomic<int> counter;
    std::atomic<int> sum;
    std::atomic<bool> done;
    counter = 0;
    sum = 0;
    done = false;

    cool::ng::async::factory::sequence(
        cool::ng::async::factory::create(
            runner
          , [&counter, &sum](const std::shared_ptr<my_runner>& self, const counted_copyable& v)
            {
              counter = v.size();
              for (auto&& item : v)
                sum += item;
            }
          )
        , cool::ng::async::factory::create(
              runner
            , [&done](const std::shared_ptr<my_runner>& self)
              {
                done = true;
              }
          )
    ).run(a);

    spin_wait(100, [&done] { return !!done; });
    BOOST_CHECK_EQUAL(2, counter);
    BOOST_CHECK_EQUAL(5, sum);
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

COOL_AUTO_TEST_CASE(T102,
  * utf::description("custom struct with move ctor only, ctor and dtor counters"))
{
  counted_moveonly::clear();

  {
    counted_moveonly a(2, 3);
    auto runner = std::make_shared<my_runner>();
    std::atomic<int> counter;
    std::atomic<int> sum;
    std::atomic<bool> done;
    counter = 0;
    sum = 0;
    done = false;

    cool::ng::async::factory::sequence(
        cool::ng::async::factory::create(
            runner
          , [&counter, &sum](const std::shared_ptr<my_runner>& self, counted_moveonly&& v)
            {
              counter = v.size();
              for (auto&& item : v)
                sum += item;
            }
          )
        , cool::ng::async::factory::create(
              runner
            , [&done](const std::shared_ptr<my_runner>& self)
              {
                done = true;
              }
          )
    ).run(std::move(a));

    spin_wait(100, [&done] { return !!done; });
    BOOST_CHECK_EQUAL(2, counter);
    BOOST_CHECK_EQUAL(5, sum);

  }

  std::this_thread::sleep_for(ms(100));
  BOOST_CHECK_EQUAL(4, counted_moveonly::instance_counter);
  BOOST_CHECK_EQUAL(3, counted_moveonly::cnt_move_ctor);
  BOOST_CHECK_EQUAL(0, counted_moveonly::cnt_def_ctor);
  BOOST_CHECK_EQUAL(1, counted_moveonly::cnt_el_ctor);
  BOOST_CHECK_EQUAL(4, counted_moveonly::cnt_dtor);

  BOOST_CHECK_EQUAL(counted_moveonly::instance_counter, counted_moveonly::cnt_move_ctor + counted_moveonly::cnt_def_ctor  + counted_moveonly::cnt_el_ctor);
  BOOST_CHECK_EQUAL(counted_moveonly::cnt_dtor, counted_moveonly::cnt_move_ctor + counted_moveonly::cnt_def_ctor + counted_moveonly::cnt_el_ctor);
}

BOOST_AUTO_TEST_SUITE_END()
