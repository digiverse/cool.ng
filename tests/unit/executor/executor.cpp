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

#define BOOST_TEST_MODULE Executor
#include <boost/test/unit_test.hpp>

#include "cool/ng/async/runner.h"
#include "cool/ng/impl/async/context.h"
#include "lib/async/executor.h"

using namespace cool::ng::async::detail;
using ms = std::chrono::milliseconds;

class test_stack : public context_stack
{
 public:
  void push(context* ctx) override
  {
    m_stack.push(ctx);
  }
  context* top() const override
  {
    return m_stack.top();
  }
  context* pop() override
  {
    auto ret = m_stack.top();
    m_stack.pop();
    return ret;
  }
  bool empty() const override
  {
    return m_stack.empty();
  }
 private:
  std::stack<context*> m_stack;
};

class test_context : public context
{
 public:
  test_context( const std::shared_ptr<cool::ng::async::runner>& r_, const std::function<void(const std::shared_ptr<cool::ng::async::runner>&)>& f_)
  {
    m_runner = r_;
    m_func = f_;
  }
  std::weak_ptr<cool::ng::async::runner> get_runner() const override
  {
    return m_runner;
  }
  void entry_point(const std::shared_ptr<cool::ng::async::runner>& r_, context*) override
  {
    m_func(r_);

  }
  const char* name() const override
  {
    return "anonymous";
  }
  bool will_execute() const override
  {
    return true;
  }
  void set_input(const boost::any&) override { }
  void set_res_reporter(const result_reporter& arg_) override { }
  void set_exc_reporter(const exception_reporter& arg_) override { }

 private:
  std::shared_ptr<cool::ng::async::runner> m_runner;
  std::function<void(const std::shared_ptr<cool::ng::async::runner>&)> m_func;
};

// Both context and context stack

class test_simple : public context, public context_stack
{
 public:
  test_simple(const std::shared_ptr<cool::ng::async::runner>& r_, const std::function<void(const std::shared_ptr<cool::ng::async::runner>&)>& f_)
  {
    m_runner = r_;
    m_func = f_;
  }
  // context interface
  std::weak_ptr<cool::ng::async::runner> get_runner() const override
  {
    return m_runner;
  }
  void entry_point(const std::shared_ptr<cool::ng::async::runner>& r_, context*) override
  {
    m_func(r_);

  }
  const char* name() const override
  {
    return "anonymous";
  }
  bool will_execute() const override
  {
    return true;
  }
  void set_input(const boost::any&) override { }
  void set_res_reporter(const result_reporter& arg_) override { }
  void set_exc_reporter(const exception_reporter& arg_) override { }

  // context stack interface

  void push(context* ctx) override
  { /* noop */}
  context* top() const override
  {
    return const_cast<test_simple*>(this);
  }
  context* pop() override
  {
    return this;
  }
  bool empty() const override
  {
    return true;
  }

 private:
  std::shared_ptr<cool::ng::async::runner> m_runner;
  std::function<void(const std::shared_ptr<cool::ng::async::runner>&)> m_func;
};

BOOST_AUTO_TEST_SUITE(executor)


BOOST_AUTO_TEST_CASE(basic)
{
  auto runner = std::make_shared<cool::ng::async::runner>();
  std::atomic_int aux;
  aux = 0;
  test_stack* stack = new test_stack;
  auto ctx = new test_context(
      runner
    , [&] (const std::shared_ptr<cool::ng::async::runner>&)
      {
        ++aux;
        delete stack->pop();  // remove itself from stack - simulation of real contexts
      }
  );

  stack->push(ctx);
  runner->impl()->run(stack);
  while (aux == 0)
  ;

  BOOST_CHECK_EQUAL(1, aux);
}

#if 1
BOOST_AUTO_TEST_CASE(deep_stack)
{
#if defined(WINDOWS_TARGET)
  const int NUM_TASKS = 100000;
#else
  const int NUM_TASKS = 1000000;
#endif

  auto runner = std::make_shared<cool::ng::async::runner>();
  std::atomic_int aux;
  aux = 0;
  test_stack* stack = new test_stack;
  std::mutex m;
  std::condition_variable cv;

  // the last task to execute - notify it's complete
  stack->push(new test_context(
      runner
    , [&] (const std::shared_ptr<cool::ng::async::runner>&)
      {
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
        delete stack->pop(); // remove self from stack
      }
  ));

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
  {
    std::unique_lock<std::mutex> l(m);

    cv.wait_for(l, ms(15000), [&] { return aux == NUM_TASKS; } );
  }

  BOOST_CHECK_EQUAL(aux, NUM_TASKS);
}
#endif
#if 1
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
  std::mutex m;
  std::condition_variable cv;


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

  // the last task to execute - notify it's complete
  runner->impl()->run(new test_simple(
      runner
    , [&] (const std::shared_ptr<cool::ng::async::runner>&)
      {
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
      }
  ));

  BOOST_CHECK(aux > 0);
  {
    std::unique_lock<std::mutex> l(m);

    cv.wait_for(l, ms(5000), [&] { return aux == NUM_TASKS; } );
  }

  BOOST_CHECK_EQUAL(aux, NUM_TASKS);
}
#endif
#if 1
BOOST_AUTO_TEST_CASE(many_stacks_start_stop)
{
#if defined(WINDOWS_TARGET)
  const int NUM_TASKS = 100000;
#else
  const int NUM_TASKS = 1000000;
#endif
  auto runner = std::make_shared<cool::ng::async::runner>();
  std::atomic_int aux;
  aux = 0;
  std::mutex m;
  std::condition_variable cv;


  runner->impl()->stop();

  for (int i = 0; i < NUM_TASKS; ++i)
  {
    runner->impl()->run(new test_simple(
        runner
      , [&, i] (const std::shared_ptr<cool::ng::async::runner>&)
        {
          if (aux == i)
            ++aux;
        }
      )
    );
  }

  // the last task to execute - notify it's complete
  runner->impl()->run(new test_simple(
      runner
    , [&] (const std::shared_ptr<cool::ng::async::runner>&)
      {
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
      }
  ));

  BOOST_CHECK_EQUAL(0, aux);
  runner->impl()->start();

  {
    std::unique_lock<std::mutex> l(m);

    cv.wait_for(l, ms(5000), [&] { return aux == NUM_TASKS; } );
  }

  BOOST_CHECK_EQUAL(aux, NUM_TASKS);
}
#endif
#if 1
BOOST_AUTO_TEST_CASE(multi_thread_run_feed)
{
#if defined(WINDOWS_TARGET)
  const int NUM_TASKS = 10000;
#else
  const int NUM_TASKS = 100000;
#endif
  const int NUM_THREADS = 10;

  auto runner = std::make_shared<cool::ng::async::runner>();
  std::atomic_int aux;
  aux = 0;
  std::mutex thread_m;
  std::condition_variable thread_cv;
  std::mutex m;
  std::condition_variable cv;

  bool go_for_it = false;

  std::unique_ptr<std::thread> threads[NUM_THREADS];
  std::unique_lock<std::mutex> l(m);

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
        // the last task should notify the main thread that this thred's task
        // sequence is complete - that will wake up main thread NUM_THREADS
        // times but lamda should prevent premature completion
        runner->impl()->run(new test_simple(
            runner
          , [&] (const std::shared_ptr<cool::ng::async::runner>&)
            {
              std::unique_lock<std::mutex> l(m);
              cv.notify_one();
            }
        ));
      }
    ));
  }

  // notify threads to start sending tasks into runner
  {
    std::unique_lock<std::mutex> l(thread_m);
    thread_cv.notify_all();
  }

  // wait for all tasks to complete
  {
    cv.wait_for(l, ms(5000), [&] { return aux == NUM_TASKS * NUM_THREADS; } );
  }

  // join threads
  for (int i = 0; i < NUM_THREADS; ++i)
    threads[i]->join();

  BOOST_CHECK_EQUAL(aux, NUM_TASKS * NUM_THREADS);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
