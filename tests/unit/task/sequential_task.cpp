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

#define BOOST_TEST_MODULE SequentialTask
#include <boost/test/unit_test.hpp>

#include "cool/ng/async.h"

using ms = std::chrono::milliseconds;

BOOST_AUTO_TEST_SUITE(sequential_task)


class my_runner : public cool::ng::async::runner
{
 public:
  void inc() { ++counter; }

  int counter = 0;
};

BOOST_AUTO_TEST_CASE(basic)
{
  auto runner = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&, int value)
      {
        return value + 1;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&, int value)
      {
        return value + 1;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>&, int value)
      {
        counter = value + 1;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
      }
  );

  auto seq = cool::ng::async::factory::sequential(t1, t2, t3);
  std::unique_lock<std::mutex> l(m);
  seq.run(5);
  cv.wait_for(l, ms(100), [&counter] { return counter == 8; });

  BOOST_CHECK_EQUAL(8, counter);
}

BOOST_AUTO_TEST_CASE(basic_two_runners)
{
  auto runner1 = std::make_shared<my_runner>();
  auto runner2 = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        return value + 1;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner2
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        return value + 1;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        counter = value + 1;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
      }
  );

  auto seq = cool::ng::async::factory::sequential(t1, t2, t3);
  std::unique_lock<std::mutex> l(m);
  seq.run(5);
  cv.wait_for(l, ms(100), [&counter] { return counter == 8; });

  BOOST_CHECK_EQUAL(8, counter);
  BOOST_CHECK_EQUAL(2, runner1->counter);
  BOOST_CHECK_EQUAL(1, runner2->counter);
}

BOOST_AUTO_TEST_CASE(sequence_of_sequence)
{
  auto runner1 = std::make_shared<my_runner>();
  auto runner2 = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  auto t1 = cool::ng::async::factory::create(
      runner1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        return value + 1;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner2
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        return value + 1;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        counter = value + 1;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
      }
  );

  auto seq1 = cool::ng::async::factory::sequential(t1, t2);
  auto seq2 = cool::ng::async::factory::sequential(t1, t2, t3);

  auto seq = cool::ng::async::factory::sequential(seq1, seq2);

  std::unique_lock<std::mutex> l(m);
  seq.run(5);
  cv.wait_for(l, ms(100), [&counter] { return counter == 10; });

  BOOST_CHECK_EQUAL(10, counter);
  BOOST_CHECK_EQUAL(3, runner1->counter);
  BOOST_CHECK_EQUAL(2, runner2->counter);
}

BOOST_AUTO_TEST_CASE(deep_sequence)
{
  auto runner1 = std::make_shared<my_runner>();
  auto runner2 = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  std::unique_lock<std::mutex> l(m);
  auto t1 = cool::ng::async::factory::create(
      runner1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        return value + 1;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner2
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        return value + 2;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        counter = value + 1;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
      }
  );

  cool::ng::async::factory::sequential(
      cool::ng::async::factory::sequential(
          cool::ng::async::factory::sequential(t1, t2)
        , t1
      )
    , t3
  ).run(5);

  cv.wait_for(l, ms(100), [&counter] { return counter == 10; });


  BOOST_CHECK_EQUAL(10, counter);
  BOOST_CHECK_EQUAL(3, runner1->counter);
  BOOST_CHECK_EQUAL(1, runner2->counter);

  counter = 0;
  runner1->counter = 0;
  runner2->counter = 0;

  cool::ng::async::factory::sequential(
    t2
    , cool::ng::async::factory::sequential(
          cool::ng::async::factory::sequential(
              cool::ng::async::factory::sequential(t1, t2)
            , t1
          )
        , t2
      )
    , t3
  ).run(5);

  cv.wait_for(l, ms(100), [&counter] { return counter == 10; });


  BOOST_CHECK_EQUAL(14, counter);
  BOOST_CHECK_EQUAL(3, runner1->counter);
  BOOST_CHECK_EQUAL(3, runner2->counter);

}

BOOST_AUTO_TEST_CASE(deep_sequence_exception)
{
  auto runner1 = std::make_shared<my_runner>();
  auto runner2 = std::make_shared<my_runner>();
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> counter;
  counter = 0;

  std::unique_lock<std::mutex> l(m);
  auto t1 = cool::ng::async::factory::create(
      runner1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        return value + 1;
      }
  );
  auto t2 = cool::ng::async::factory::create(
      runner2
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value) -> int
      {
        r->inc();
        throw 2;
      }
  );
  auto t3 = cool::ng::async::factory::create(
      runner1
    , [&m, &cv, &counter] (const std::shared_ptr<my_runner>& r, int value)
      {
        r->inc();
        counter = value + 1;
        std::unique_lock<std::mutex> l(m);
        cv.notify_one();
      }
  );

  cool::ng::async::factory::sequential(
    t1
    , cool::ng::async::factory::sequential(
          cool::ng::async::factory::sequential(
              cool::ng::async::factory::sequential(t1, t1)
            , t2
          )
        , t1
      )
    , t3
  ).run(5);

  cv.wait_for(l, ms(100), [&counter] { return counter == 10; });


  BOOST_CHECK_EQUAL(0, counter);
  BOOST_CHECK_EQUAL(3, runner1->counter);
  BOOST_CHECK_EQUAL(1, runner2->counter);

}

BOOST_AUTO_TEST_SUITE_END()
