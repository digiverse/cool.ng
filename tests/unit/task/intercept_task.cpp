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

#include <vector>
#include <list>
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

BOOST_AUTO_TEST_SUITE(intercept)


class abc { };
class cde { };
class fgh { };

COOL_AUTO_TEST_CASE(T001,
  * utf::description("nested try-catch within one exception type not caught by inner"))
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

COOL_AUTO_TEST_CASE(T002,
  * utf::description("catch-all catch task"))
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


#if 0
COOL_AUTO_TEST_CASE(catch_all_second_try)
{
  auto runner = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner
    , [] (const std::shared_ptr<my_runner>&) -> int
      {
        return 42;
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

}



COOL_AUTO_TEST_CASE(stupid_vs_wont_compile)
{
  auto runner = std::make_shared<my_runner>();

  auto c_t = cool::ng::async::factory::create(
      runner
    , [](const std::shared_ptr<my_runner>& r) -> void
    {

    }
  );

  auto r_t = cool::ng::async::factory::create(
      runner
    , [](const std::shared_ptr<my_runner>& r, const  char* ptr) -> void
    {

    }
  );

  auto catch_t = cool::ng::async::factory::create(
              runner
            , [] (const std::shared_ptr<my_runner>& r_, const std::exception_ptr& ex_) -> void
              {
              }
          );

auto outer = cool::ng::async::factory::try_catch(
          c_t
        , catch_t
      );

  auto task = cool::ng::async::factory::sequence(
      r_t
    , outer);


}
#endif


COOL_AUTO_TEST_CASE(T003,
  * utf::description("std::list input (type for which runtime will use move-ctor for internal value transfer)"))
{
  std::list<int> ints{1, 2, 3};
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;

  auto t = cool::ng::async::factory::try_catch(
      cool::ng::async::factory::create(
          runner
        , [&counter](const std::shared_ptr<my_runner>& self, const std::list<int>& b)
          {
            counter = b.size();
          }
        )
      , cool::ng::async::factory::create(
            runner
          , [&counter](const std::shared_ptr<my_runner>& self, const std::exception_ptr& ex)
            {
              counter = 1;
            }
        )
  );

  t.run(ints);

  spin_wait(100, [&counter] { return counter != 0; });
  BOOST_CHECK_EQUAL(3, counter);
}

struct my_struct
{
  my_struct() : a(0), b(0)                 { /* noop */ }
  my_struct(int a_, int b_) : a(a_), b(b_) { /* noop */ }

  int a;
  int b;
};

COOL_AUTO_TEST_CASE(T004,
  * utf::description("custom non-simple type with simple members"))
{
  my_struct a(2, 3);
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;


  cool::ng::async::factory::try_catch(
      cool::ng::async::factory::create(
          runner
        , [&counter](const std::shared_ptr<my_runner>& self, const my_struct& v)
          {
            counter = v.a + v.b;
          }
        )
      , cool::ng::async::factory::create(
            runner
          , [&counter](const std::shared_ptr<my_runner>& self, const std::exception_ptr& ex)
            {
              counter = 1;
            }
        )
  ).run(a);

  spin_wait(100, [&counter] { return counter != 0; });
  BOOST_CHECK_EQUAL(5, counter);
}

COOL_AUTO_TEST_CASE(T005,
  * utf::description("std::vector input of custom structs (type for which runtime will use move-ctor for internal value transfer)"))
{
  std::vector<my_struct> ints{ my_struct(1, 2), my_struct(3, 4), my_struct(5, 6) };
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;
  std::atomic<int> sum;
  sum = 0;

  cool::ng::async::factory::try_catch(
      cool::ng::async::factory::create(
          runner
        , [&counter, &sum](const std::shared_ptr<my_runner>& self, const std::vector<my_struct>& v)
          {
            counter = v.size();
            for (auto&& item : v)
              sum += item.a + item.b;
          }
        )
      , cool::ng::async::factory::create(
            runner
          , [&counter](const std::shared_ptr<my_runner>& self, const std::exception_ptr& ex)
            {
              counter = 1;
            }
        )
  ).run(ints);

  spin_wait(100, [&counter] { return counter != 0; });
  BOOST_CHECK_EQUAL(3, counter);
  BOOST_CHECK_EQUAL(21, sum);
}

template <typename T>
struct my_other_struct
{
  my_other_struct()                               { /* noop */ }
  my_other_struct(int a_, int b_) : v({ a_, b_ }) { /* noop */ }

  T v;
};

COOL_AUTO_TEST_CASE(T006,
  * utf::description("custom struct with std::vector member (where runtime still prefers std::move internally"))
{
  my_other_struct<std::vector<int>> a(2, 3);
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;
  std::atomic<int> sum;
  sum = 0;

  cool::ng::async::factory::try_catch(
      cool::ng::async::factory::create(
          runner
        , [&counter, &sum](const std::shared_ptr<my_runner>& self, const  my_other_struct<std::vector<int>>& v)
          {
            counter = v.v.size();
            for (auto&& item : v.v)
              sum += item;
          }
        )
      , cool::ng::async::factory::create(
            runner
          , [&counter](const std::shared_ptr<my_runner>& self, const std::exception_ptr& ex)
            {
              counter = 1;
            }
        )
  ).run(a);

  spin_wait(100, [&counter] { return counter != 0; });
  BOOST_CHECK_EQUAL(2, counter);
  BOOST_CHECK_EQUAL(5, sum);
}

COOL_AUTO_TEST_CASE(T007,
  * utf::description("custom struct with std::list member (where runtime still prefers std::move internally"))
{
  my_other_struct<std::list<int>> a(2, 3);
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;
  std::atomic<int> sum;
  sum = 0;

  cool::ng::async::factory::try_catch(
      cool::ng::async::factory::create(
          runner
        , [&counter, &sum](const std::shared_ptr<my_runner>& self, const  my_other_struct<std::list<int>>& v)
          {
            counter = v.v.size();
            for (auto&& item : v.v)
              sum += item;
          }
        )
      , cool::ng::async::factory::create(
            runner
          , [&counter](const std::shared_ptr<my_runner>& self, const std::exception_ptr& ex)
            {
              counter = 1;
            }
        )
  ).run(a);

  spin_wait(100, [&counter] { return counter != 0; });
  BOOST_CHECK_EQUAL(2, counter);
  BOOST_CHECK_EQUAL(5, sum);
}

COOL_AUTO_TEST_CASE(T008,
  * utf::description("std::pair input (runtime prefers copy  ctor)"))
{
  std::pair<int,int> a(2, 3);
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;
  std::atomic<int> sum;
  sum = 0;

  cool::ng::async::factory::try_catch(
      cool::ng::async::factory::create(
          runner
        , [&counter, &sum](const std::shared_ptr<my_runner>& self, const  std::pair<int, int>& v)
          {
            counter = 2;
            sum = v.first + v.second;
          }
        )
      , cool::ng::async::factory::create(
            runner
          , [&counter](const std::shared_ptr<my_runner>& self, const std::exception_ptr& ex)
            {
              counter = 1;
            }
        )
  ).run(a);

  spin_wait(100, [&counter] { return counter != 0; });
  BOOST_CHECK_EQUAL(2, counter);
  BOOST_CHECK_EQUAL(5, sum);
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

    cool::ng::async::factory::try_catch(
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
            , [&counter](const std::shared_ptr<my_runner>& self, const std::exception_ptr& ex)
              {
                counter = 1;
              }
          )
    ).run(a);

    spin_wait(100, [&counter] { return counter != 0; });
    BOOST_CHECK_EQUAL(2, counter);
    BOOST_CHECK_EQUAL(5, sum);

// TODO: is this bug in Boost.Test
#if 0
    BOOST_CHECK_EQUAL(3, counted_moveable::cnt_dtor);
    std::cout  << " ====== " << counted_moveable::cnt_dtor << "\n";
#endif
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

    cool::ng::async::factory::try_catch(
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
            , [&counter](const std::shared_ptr<my_runner>& self, const std::exception_ptr& ex)
              {
                counter = 1;
              }
          )
    ).run(a);

    spin_wait(100, [&counter] { return counter != 0; });
    BOOST_CHECK_EQUAL(2, counter);
    BOOST_CHECK_EQUAL(5, sum);

// TODO: is this bug in Boost.Test
#if 0
    BOOST_CHECK_EQUAL(3, counted_moveable::cnt_dtor);
    std::cout  << " ====== " << counted_moveable::cnt_dtor << "\n";
#endif
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

    cool::ng::async::factory::try_catch(
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
            , [&counter](const std::shared_ptr<my_runner>& self, const std::exception_ptr& ex)
              {
                counter = 1;
              }
          )
    ).run(std::move(a));

    spin_wait(100, [&counter] { return counter != 0; });
    BOOST_CHECK_EQUAL(2, counter);
    BOOST_CHECK_EQUAL(5, sum);

// TODO: is this bug in Boost.Test
#if 0
    BOOST_CHECK_EQUAL(3, counted_moveable::cnt_dtor);
    std::cout  << " ====== " << counted_moveable::cnt_dtor << "\n";
#endif
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

#if 0
COOL_AUTO_TEST_CASE(test_107)
{
  counted_moveable a(2, 3);
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;
  std::atomic<int> sum;
  sum = 0;

  std::cout << "==================================== 1\n";

  cool::ng::async::factory::try_catch(
      cool::ng::async::factory::create(
          runner
        , [&counter, &sum](const std::shared_ptr<my_runner>& self, const counted_moveable& v)
          {
            std::cout << "==================================== 2\n";
            counter = v.size();
            for (auto&& item : v)
              sum += item;
          }
        )
      , cool::ng::async::factory::create(
            runner
          , [&counter](const std::shared_ptr<my_runner>& self, const std::exception_ptr& e)
            {
              if (counter == 0)
                counter = 1;
            }
        )
  ).run(a);

  spin_wait(100, [&counter] { return counter != 0; });
  BOOST_CHECK_EQUAL(2, counter);
  BOOST_CHECK_EQUAL(5, sum);
  spin_wait(20, [] { return false; });

}
#endif
BOOST_AUTO_TEST_SUITE_END()
