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

#include <boost/any.hpp>

#define BOOST_TEST_MODULE InterceptTask
#include <boost/test/unit_test.hpp>

#include "cool/ng/async.h"

using ms = std::chrono::milliseconds;

BOOST_AUTO_TEST_SUITE(intercept_task)

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



BOOST_AUTO_TEST_CASE(catch_all_second_try)
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



BOOST_AUTO_TEST_CASE(stupid_vs_wont_compile)
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

BOOST_AUTO_TEST_CASE(test_100)
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

BOOST_AUTO_TEST_CASE(test_101)
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

BOOST_AUTO_TEST_CASE(test_102)
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

BOOST_AUTO_TEST_CASE(test_103)
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

BOOST_AUTO_TEST_CASE(test_104)
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

BOOST_AUTO_TEST_CASE(test_105)
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

struct my_vector : public std::vector<int>
{
 public:
  my_vector() : vector(), m_instance(++ counter)
  {
//    std::cout << "==================================== default ctor\n";
  }
  my_vector(int a_, int b_) : vector({ a_, b_ }),  m_instance(++counter)
  {
//    std::cout << "==================================== two values ctor\n";
  }

  my_vector(const my_vector& other) : vector(other),   m_instance(++counter)
  {
//    std::cout << "==================================== copy ctor from " << other.m_instance << " to " << m_instance << "\n";
  }

  my_vector(my_vector&& other) : vector(std::move(other)), m_instance(++counter)
  {
//    std::cout << "==================================== move ctor from " << other.m_instance << " to " << m_instance << "\n";

  }
  static int counter;
  int m_instance;
};

int my_vector::counter = 0;

BOOST_AUTO_TEST_CASE(test_106)
{
  my_vector a(2, 3);
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;
  std::atomic<int> sum;
  sum = 0;

//  std::cout << "==================================== 1\n";

  cool::ng::async::factory::try_catch(
      cool::ng::async::factory::create(
          runner
        , [&counter, &sum](const std::shared_ptr<my_runner>& self, const my_vector& v)
          {
  //          std::cout << "==================================== 2\n";
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
}

BOOST_AUTO_TEST_CASE(test_107)
{
  my_vector a(2, 3);
  auto runner = std::make_shared<my_runner>();
  std::atomic<int> counter;
  counter = 0;
  std::atomic<int> sum;
  sum = 0;

//  std::cout << "==================================== 1\n";

  cool::ng::async::factory::sequence(
      cool::ng::async::factory::create(
          runner
        , [&counter, &sum](const std::shared_ptr<my_runner>& self, const my_vector& v)
          {
  //          std::cout << "==================================== 2\n";
            counter = v.size();
            for (auto&& item : v)
              sum += item;
          }
        )
      , cool::ng::async::factory::create(
            runner
          , [&counter](const std::shared_ptr<my_runner>& self)
            {
              if (counter == 0)
                counter = 1;
            }
        )
  ).run(a);

  spin_wait(100, [&counter] { return counter != 0; });
  BOOST_CHECK_EQUAL(2, counter);
  BOOST_CHECK_EQUAL(5, sum);
}



BOOST_AUTO_TEST_SUITE_END()
