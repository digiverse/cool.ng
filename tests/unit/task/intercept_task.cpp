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

#define BOOST_TEST_MODULE InterceptTask
#include <boost/test/unit_test.hpp>

#include "cool/ng/async.h"

using ms = std::chrono::milliseconds;

BOOST_AUTO_TEST_SUITE(intercept_task)


class my_runner : public cool::ng::async::runner
{
 public:
  void inc() { ++counter; }

  int counter = 0;
};

class abc { };
class cde { };

BOOST_AUTO_TEST_CASE(basic)
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

  auto task = cool::ng::async::factory::try_catch(t1, t2, t3);
  
  std::unique_lock<std::mutex> l(m);
  task.run(5);
  cv.wait_for(l, ms(100), [&counter] { return counter == 42; });

  BOOST_CHECK_EQUAL(42, counter);

  task.run(6);
  cv.wait_for(l, ms(100), [&counter] { return counter == 84; });

  BOOST_CHECK_EQUAL(84, counter);

  task.run(7);
  cv.wait_for(l, ms(100), [&counter] { return counter == 21; });

  BOOST_CHECK_EQUAL(21, counter);
}

BOOST_AUTO_TEST_SUITE_END()
