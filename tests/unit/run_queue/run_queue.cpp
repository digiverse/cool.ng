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

#define BOOST_TEST_MODULE RunQueue
#include <boost/test/unit_test.hpp>

#include "run_queue.h"

using namespace cool::ng::async::impl;
using ms = std::chrono::milliseconds;

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

void void_to_function_exec(void* data)
{
  (**static_cast<void (**)()>(data))();
}

BOOST_AUTO_TEST_SUITE(run_queue_basics)


BOOST_AUTO_TEST_CASE(basic_run)
{
  static std::atomic_int aux;
  aux = 0;

  auto rq = run_queue::create();
  auto to_run = static_cast<void(*)()>([](){ ++aux; });

  rq->enqueue(void_to_function_exec, nullptr, &to_run);
  spin_wait(100, [] () { return aux > 0; });

  BOOST_CHECK_EQUAL(1, aux);
  run_queue::release(rq);
}

BOOST_AUTO_TEST_CASE(start_stop)
{
  static std::atomic_int aux;
  aux = 0;

  auto rq = run_queue::create();
  auto lambda = static_cast<void(*)()>([](){ ++aux; });

  rq->stop();
  BOOST_CHECK(!rq->is_active());
  rq->enqueue(void_to_function_exec, nullptr, &lambda);
  rq->enqueue(void_to_function_exec, nullptr, &lambda);
  BOOST_CHECK(!spin_wait(50, [] () { return aux > 0; }));
  BOOST_CHECK_EQUAL(0, aux);

  rq->start();
  BOOST_CHECK(spin_wait(50, [] () { return aux == 2; }));
  BOOST_CHECK_EQUAL(2, aux);
  run_queue::release(rq);
}

BOOST_AUTO_TEST_CASE(exec_on_delete)
{
  static std::atomic_int aux;
  std::weak_ptr<run_queue> test_gone;
  aux = 0;
  auto to_run = static_cast<void(*)()>([](){ ++aux; });

  {
    auto rq = run_queue::create();
    test_gone = rq;
    rq->stop();
    BOOST_CHECK(!rq->is_active());
    rq->enqueue(void_to_function_exec, nullptr, &to_run);
    rq->enqueue(void_to_function_exec, nullptr, &to_run);
    BOOST_CHECK(!spin_wait(50, [] () { return aux > 0; }));
    BOOST_CHECK_EQUAL(0, aux);

    run_queue::release(rq);
  }

  BOOST_CHECK(spin_wait(50, [] () { return aux == 2; }));
  BOOST_CHECK_EQUAL(2, aux);

  // prove that run_queue instance is now gone
  BOOST_CHECK(!test_gone.lock());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(advanced)

BOOST_AUTO_TEST_CASE(multi_thread_queue_feed)
{
  const int NUM_TASKS = 100000;
  const int NUM_THREADS = 10;

  auto queue = run_queue::create();
  static std::atomic_int aux;
  aux = 0;
  auto to_run = static_cast<void(*)()>([](){ ++aux; });

  std::mutex thread_m;
  std::condition_variable thread_cv;

  bool go_for_it = false;

  std::unique_ptr<std::thread> threads[NUM_THREADS];

  for (int i = 0; i < NUM_THREADS; ++i)
  {
    threads[i].reset(new std::thread(
      [&]
      {
        {
          std::unique_lock<std::mutex> l(thread_m);
          thread_cv.wait_for(l, ms(1000), [&go_for_it]{ return go_for_it; });
        }
        for (int i = 0; i < NUM_TASKS; ++i)
          queue->enqueue(void_to_function_exec, nullptr, &to_run);
      }
    ));
  }

  // notify threads to start sending tasks into runner
  {
    std::unique_lock<std::mutex> l(thread_m);
    thread_cv.notify_all();
  }

  for (int i = 0; i < NUM_THREADS; ++i)
    threads[i]->join();

  spin_wait(5000, [&] { return aux == NUM_TASKS * NUM_THREADS; } );

  run_queue::release(queue);

  BOOST_CHECK_EQUAL(aux, NUM_TASKS * NUM_THREADS);
}

#if 0
#if TEST2==1
BOOST_AUTO_TEST_CASE(deep_stack)
{
#if defined(WINDOWS_TARGET)
  const int NUM_TASKS = 100000;
#else
  const int NUM_TASKS = 1000000;
#endif

  {
    auto runner = std::make_shared<cool::ng::async::runner>();
    std::atomic_int aux;
    aux = 0;
    test_stack* stack = new test_stack;

    for (int i = 0; i < NUM_TASKS; ++i)
    {
      stack->push(new test_context(
          runner
        , [&] (const std::shared_ptr<cool::ng::async::runner>&)
          {
            ++aux;
            delete stack->pop(); // remove self from stack
          }
        )
      );
    }

    runner->impl()->run(stack);

    spin_wait(15000, [&] { return aux == NUM_TASKS; } );
    BOOST_CHECK_EQUAL(aux, NUM_TASKS);
  }
  std::this_thread::sleep_for(ms(100));
}
#endif

#if TEST3==1
BOOST_AUTO_TEST_CASE(many_stacks)
{
#if defined(WINDOWS_TARGET)
  const int NUM_TASKS = 100000;
#else
  const int NUM_TASKS = 1000000;
#endif
  auto runner = std::make_shared<cool::ng::async::runner>();
  std::atomic_int aux;
  aux = 0;


  for (int i = 0; i < NUM_TASKS; ++i)
  {
    runner->impl()->run(new test_simple(
        runner
      , [&] (const std::shared_ptr<cool::ng::async::runner>&)
        {
          ++aux;
        }
      )
    );
  }

  spin_wait(5000, [&] { return aux == NUM_TASKS; });
  BOOST_CHECK_EQUAL(aux, NUM_TASKS);
}
#endif

#if TEST4==1
BOOST_AUTO_TEST_CASE(multi_thread_run_feed)
{
  const int NUM_TASKS = 100000;
  const int NUM_THREADS = 10;

  auto runner = std::make_shared<cool::ng::async::runner>();
  std::atomic_int aux;
  aux = 0;
  std::mutex thread_m;
  std::condition_variable thread_cv;

  bool go_for_it = false;

  std::unique_ptr<std::thread> threads[NUM_THREADS];

  for (int i = 0; i < NUM_THREADS; ++i)
  {
    threads[i].reset(new std::thread(
      [&]
      {
        {
          std::unique_lock<std::mutex> l(thread_m);
          thread_cv.wait_for(l, ms(1000), [&go_for_it]{ return go_for_it; });
        }
        for (int i = 0; i < NUM_TASKS; ++i)
        {
          runner->impl()->run(new test_simple(
              runner
            , [&aux] (const std::shared_ptr<cool::ng::async::runner>&)
              {
                ++aux;
              }
          ));
        }
      }
    ));
  }

  // notify threads to start sending tasks into runner
  {
    std::unique_lock<std::mutex> l(thread_m);
    thread_cv.notify_all();
  }

  spin_wait(5000, [&] { return aux == NUM_TASKS * NUM_THREADS; } );

  for (int i = 0; i < NUM_THREADS; ++i)
    threads[i]->join();

  BOOST_CHECK_EQUAL(aux, NUM_TASKS * NUM_THREADS);
}

#endif

#endif

BOOST_AUTO_TEST_SUITE_END()
