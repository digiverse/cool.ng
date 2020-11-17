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

#include <tuple>
#include <array>

#include "cool/ng/impl/async/task.h"

#define BOOST_TEST_MODULE TaskTuple
#include "unit_test_common.h"

namespace impl = cool::ng::async::detail;

struct counters
{
  counters() : m_copy(0), m_move(0), m_copy_assign(0), m_move_assign(0)
  { /* noop */ }

  int m_copy;
  int m_move;
  int m_copy_assign;
  int m_move_assign;
};

struct counted_base : public counters
{
  counted_base()
  { /* noop */ }

  counted_base(const counted_base& o_)
  {
    m_copy = o_.m_copy + 1;
    m_move = o_.m_move;
    m_copy_assign = o_.m_copy_assign;
    m_move_assign = o_.m_move_assign;
  }
  counted_base(counted_base&& o_)
  {
    m_copy = o_.m_copy;
    m_move = o_.m_move + 1;
    m_copy_assign = o_.m_copy_assign;
    m_move_assign = o_.m_move_assign;

    o_.~counted_base();
    new (&o_) counted_base();
  }
  counted_base& operator =(const counted_base& o_)
  {
    m_copy = o_.m_copy;
    m_move = o_.m_move;
    m_copy_assign = o_.m_copy_assign + 1;
    m_move_assign = o_.m_move_assign;
    return *this;
  }
  counted_base& operator =(counted_base&& o_)
  {
    m_copy = o_.m_copy;
    m_move = o_.m_move;
    m_copy_assign = o_.m_copy_assign;
    m_move_assign = o_.m_move_assign + 1;

    o_.~counted_base();
    new (&o_) counted_base();
    return *this;
  }
};

struct first_type : public counted_base
{
  std::string type() const
  {
    return "first_type";
  }
};

struct second_type : public counted_base
{
  std::string type() const
  {
    return "second_type";
  }
};

struct third_type : public counted_base
{
  third_type() = default;
  third_type(const third_type&) = default;
  third_type& operator =(const third_type&) = default;
#if defined(__clang__)
  third_type(third_type&&) = delete;
  third_type& operator =(third_type&&) = default;
#else
  third_type(third_type&&) = default;
  third_type& operator =(third_type&&) = default;
#endif
  std::string type() const
  {
    return "third_type";
  }
};

struct fourth_type : public counted_base
{
  fourth_type() = default;
  fourth_type(const fourth_type&) = default;
  fourth_type& operator =(const fourth_type&) = default;
  fourth_type(fourth_type&&) = delete;
  fourth_type& operator =(fourth_type&&) = default;
  std::string type() const
  {
    return "fourth_type";
  }
};

template <typename T, typename Y>
counters which_counter(int index, const T& t_, const Y& y_)
{
  switch (index)
  {
    case 0: return t_;
    case 1: return y_;
  }
}


// ---
// --- A set of tests that make sure that std::tuple works correctly
// ---
BOOST_AUTO_TEST_SUITE(tuple_)

COOL_AUTO_TEST_CASE(T001,
  *utf::description("std::tuple move and copy counts"))
{
  {
    first_type a;
    second_type b;
    third_type c;
    std::tuple<first_type, second_type, third_type> t(a, b, c);
//    auto t = std::make_tuple(a, b, c);
    BOOST_CHECK_EQUAL(1, std::get<0>(t).m_copy);
    BOOST_CHECK_EQUAL(0, std::get<0>(t).m_move);
    BOOST_CHECK_EQUAL(0, std::get<0>(t).m_copy_assign);
    BOOST_CHECK_EQUAL(0, std::get<0>(t).m_move_assign);
    BOOST_CHECK_EQUAL(1, std::get<1>(t).m_copy);
    BOOST_CHECK_EQUAL(0, std::get<1>(t).m_move);
    BOOST_CHECK_EQUAL(0, std::get<1>(t).m_copy_assign);
    BOOST_CHECK_EQUAL(0, std::get<1>(t).m_move_assign);
    BOOST_CHECK_EQUAL(1, std::get<2>(t).m_copy);
    BOOST_CHECK_EQUAL(0, std::get<2>(t).m_move);
    BOOST_CHECK_EQUAL(0, std::get<2>(t).m_copy_assign);
    BOOST_CHECK_EQUAL(0, std::get<2>(t).m_move_assign);
  }
  {
    first_type a;
    second_type b;
    auto t = std::make_tuple(std::move(a), std::move(b));
    BOOST_CHECK_EQUAL(0, std::get<0>(t).m_copy);
    BOOST_CHECK_EQUAL(1, std::get<0>(t).m_move);
    BOOST_CHECK_EQUAL(0, std::get<0>(t).m_copy_assign);
    BOOST_CHECK_EQUAL(0, std::get<0>(t).m_move_assign);
    BOOST_CHECK_EQUAL(0, std::get<1>(t).m_copy);
    BOOST_CHECK_EQUAL(1, std::get<1>(t).m_move);
    BOOST_CHECK_EQUAL(0, std::get<1>(t).m_copy_assign);
    BOOST_CHECK_EQUAL(0, std::get<1>(t).m_move_assign);
  }
  {
    first_type a;
    second_type b;
    third_type c;
    std::tuple<first_type, second_type, third_type> t(std::move(a), std::move(b), std::move(c));
#if defined(__clang__)
    // !!! since c is not moveable, all tuple elements will be copy constructed
    // !!! gcc and visual studio do not support tuple with non-move-constructible elements
    BOOST_CHECK_EQUAL(1, std::get<0>(t).m_copy);
    BOOST_CHECK_EQUAL(0, std::get<0>(t).m_move);
    BOOST_CHECK_EQUAL(1, std::get<1>(t).m_copy);
    BOOST_CHECK_EQUAL(0, std::get<1>(t).m_move);
    BOOST_CHECK_EQUAL(1, std::get<2>(t).m_copy);
    BOOST_CHECK_EQUAL(0, std::get<2>(t).m_move);
#else
    BOOST_CHECK_EQUAL(0, std::get<0>(t).m_copy);
    BOOST_CHECK_EQUAL(1, std::get<0>(t).m_move);
    BOOST_CHECK_EQUAL(0, std::get<1>(t).m_copy);
    BOOST_CHECK_EQUAL(1, std::get<1>(t).m_move);
    BOOST_CHECK_EQUAL(0, std::get<2>(t).m_copy);
    BOOST_CHECK_EQUAL(1, std::get<2>(t).m_move);
#endif
    BOOST_CHECK_EQUAL(0, std::get<0>(t).m_copy_assign);
    BOOST_CHECK_EQUAL(0, std::get<0>(t).m_move_assign);
    BOOST_CHECK_EQUAL(0, std::get<1>(t).m_copy_assign);
    BOOST_CHECK_EQUAL(0, std::get<1>(t).m_move_assign);
    BOOST_CHECK_EQUAL(0, std::get<2>(t).m_copy_assign);
    BOOST_CHECK_EQUAL(0, std::get<2>(t).m_move_assign);
  }
  {
    first_type a;
    second_type b;
    third_type c;
    std::tuple<first_type, second_type, third_type> t(a, b, c);
    BOOST_CHECK_EQUAL(1, std::get<0>(t).m_copy);
    BOOST_CHECK_EQUAL(0, std::get<0>(t).m_move);
    BOOST_CHECK_EQUAL(0, std::get<0>(t).m_copy_assign);
    BOOST_CHECK_EQUAL(0, std::get<0>(t).m_move_assign);
    BOOST_CHECK_EQUAL(1, std::get<1>(t).m_copy);
    BOOST_CHECK_EQUAL(0, std::get<1>(t).m_move);
    BOOST_CHECK_EQUAL(0, std::get<1>(t).m_copy_assign);
    BOOST_CHECK_EQUAL(0, std::get<1>(t).m_move_assign);
    BOOST_CHECK_EQUAL(1, std::get<2>(t).m_copy);
    BOOST_CHECK_EQUAL(0, std::get<2>(t).m_move);
    BOOST_CHECK_EQUAL(0, std::get<2>(t).m_copy_assign);
    BOOST_CHECK_EQUAL(0, std::get<2>(t).m_move_assign);

    {
      auto tt = std::move(t);
      BOOST_CHECK_EQUAL(1, std::get<0>(tt).m_copy);
      BOOST_CHECK_EQUAL(1, std::get<0>(tt).m_move);
      BOOST_CHECK_EQUAL(0, std::get<0>(tt).m_copy_assign);
      BOOST_CHECK_EQUAL(0, std::get<0>(tt).m_move_assign);
      BOOST_CHECK_EQUAL(1, std::get<1>(tt).m_copy);
      BOOST_CHECK_EQUAL(1, std::get<1>(tt).m_move);
      BOOST_CHECK_EQUAL(0, std::get<1>(tt).m_copy_assign);
      BOOST_CHECK_EQUAL(0, std::get<1>(tt).m_move_assign);
#if defined(__clang__)
      BOOST_CHECK_EQUAL(2, std::get<2>(tt).m_copy);
      BOOST_CHECK_EQUAL(0, std::get<2>(tt).m_move);
#else
      BOOST_CHECK_EQUAL(1, std::get<2>(tt).m_copy);
      BOOST_CHECK_EQUAL(1, std::get<2>(tt).m_move);
#endif
      BOOST_CHECK_EQUAL(0, std::get<2>(tt).m_copy_assign);
      BOOST_CHECK_EQUAL(0, std::get<2>(tt).m_move_assign);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(any_)


COOL_AUTO_TEST_CASE(T001,
  *utf::description("any of std::tuple parameters"))
{
  using value_type = std::tuple<first_type, second_type, third_type>;

  first_type a;
  second_type b;
  third_type c;
  {
    impl::any store(value_type(a, b, c));
    auto&& ref = impl::any_cast<const value_type&>(store);
    BOOST_CHECK_EQUAL(1, std::get<0>(ref).m_copy);
    BOOST_CHECK_EQUAL(1, std::get<0>(ref).m_move);
    BOOST_CHECK_EQUAL(0, std::get<0>(ref).m_copy_assign);
    BOOST_CHECK_EQUAL(0, std::get<0>(ref).m_move_assign);
    BOOST_CHECK_EQUAL(1, std::get<1>(ref).m_copy);
    BOOST_CHECK_EQUAL(1, std::get<1>(ref).m_move);
    BOOST_CHECK_EQUAL(0, std::get<1>(ref).m_copy_assign);
    BOOST_CHECK_EQUAL(0, std::get<1>(ref).m_move_assign);
#if defined(__clang__)
    BOOST_CHECK_EQUAL(2, std::get<2>(ref).m_copy);
    BOOST_CHECK_EQUAL(0, std::get<2>(ref).m_move);
#else
    BOOST_CHECK_EQUAL(1, std::get<2>(ref).m_copy);
    BOOST_CHECK_EQUAL(1, std::get<2>(ref).m_move);
#endif
    BOOST_CHECK_EQUAL(0, std::get<2>(ref).m_copy_assign);
    BOOST_CHECK_EQUAL(0, std::get<2>(ref).m_move_assign);
  }
}

COOL_AUTO_TEST_CASE(T002,
  *utf::description("std::array of any elements with parameters"))
{
  first_type a;
  second_type b;
  fourth_type c;

  {
    std::array<impl::any, 3> store({a, b, c});
    const decltype(a) ref_a = impl::any_cast<const decltype(a)&>(store[0]);
    const decltype(b) ref_b = impl::any_cast<const decltype(b)&>(store[1]);
    const decltype(c) ref_c = impl::any_cast<const decltype(c)&>(store[2]);

    BOOST_CHECK_EQUAL(1, ref_a.m_copy);
    BOOST_CHECK_EQUAL(1, ref_a.m_move);
    BOOST_CHECK_EQUAL(0, ref_a.m_copy_assign);
    BOOST_CHECK_EQUAL(0, ref_a.m_move_assign);
    BOOST_CHECK_EQUAL(1, ref_b.m_copy);
    BOOST_CHECK_EQUAL(1, ref_b.m_move);
    BOOST_CHECK_EQUAL(0, ref_b.m_copy_assign);
    BOOST_CHECK_EQUAL(0, ref_b.m_move_assign);
    BOOST_CHECK_EQUAL(2, ref_c.m_copy);
    BOOST_CHECK_EQUAL(0, ref_c.m_move);
    BOOST_CHECK_EQUAL(0, ref_c.m_copy_assign);
    BOOST_CHECK_EQUAL(0, ref_c.m_move_assign);
  }
}

class my_runner : public cool::ng::async::runner
{
};

COOL_AUTO_TEST_CASE(T003,
  *utf::description("simple task context"))
{
  first_type a;
  second_type b;
  third_type c;
  auto r = std::make_shared<my_runner>();

  // -- simple task context for task int(first_type, second_type, third_type)
  {
    bool called = false;
    int sum_copy = 0, sum_move = 0;

    using return_type = int;
    using context = cool::ng::async::detail::context_impl<
        cool::ng::async::detail::tag::simple
      , my_runner
      , return_type        // return type
      , first_type         // first, second and third parameters
      , second_type
      , third_type>;
    return_type result = 0;
    std::function<return_type(const std::shared_ptr<my_runner>&, const first_type&, const second_type&, const third_type&)> lambda =
      [&called, &sum_copy, &sum_move](const std::shared_ptr<my_runner>&, const first_type& a1, const second_type& a2, const third_type& a3)
      {
        called = true;
        sum_copy = a1.m_copy + a2.m_copy + a3.m_copy;
        sum_move = a1.m_move + a2.m_move + a3.m_move;
        return 3;
      };
    std::function<void(cool::ng::async::detail::context*, const return_type&)> reporter = [&result] (cool::ng::async::detail::context* ctx_, const return_type r_)
    {
      result = r_;
    };

    std::unique_ptr<context> ctx(context::create(
        nullptr
      , std::shared_ptr<cool::ng::async::detail::task>()
      , lambda
      , reporter
      , a, b, c)
    );

    ctx->entry_point(r, nullptr);

    BOOST_CHECK(called);
    BOOST_CHECK_EQUAL(3, sum_copy);
    BOOST_CHECK_EQUAL(0, sum_move);
    BOOST_CHECK_EQUAL(3, result);
  }
  {
    bool called = false;
    bool reported = false;
    int sum_copy = 0, sum_move = 0;

    using return_type = void;
    using context = cool::ng::async::detail::context_impl<
        cool::ng::async::detail::tag::simple
      , my_runner
      , return_type        // return type
      , first_type         // first, second and third parameters
      , second_type
      , third_type>;

    std::function<return_type(const std::shared_ptr<my_runner>&, const first_type&, const second_type&, const third_type&)> lambda =
      [&called, &sum_copy, &sum_move](const std::shared_ptr<my_runner>&, const first_type& a1, const second_type& a2, const third_type& a3) -> void
      {
        called = true;
        sum_copy = a1.m_copy + a2.m_copy + a3.m_copy;
        sum_move = a1.m_move + a2.m_move + a3.m_move;
      };

    std::function<void(cool::ng::async::detail::context*)> reporter = [&reported] (cool::ng::async::detail::context* ctx_)
    {
      reported = true;
    };


    std::unique_ptr<context> ctx(context::create(
        nullptr
      , std::shared_ptr<cool::ng::async::detail::task>()
      , lambda
      , reporter
      , a, b, c)
    );

    ctx->entry_point(r, nullptr);
    BOOST_CHECK(called);
    BOOST_CHECK(reported);
    BOOST_CHECK_EQUAL(3, sum_copy);
    BOOST_CHECK_EQUAL(0, sum_move);
  }
  {
    bool called = false;

    using return_type = int;
    return_type result = 0;
    using context = cool::ng::async::detail::context_impl<
        cool::ng::async::detail::tag::simple
      , my_runner
      , return_type>;

    std::function<return_type(const std::shared_ptr<my_runner>&)> lambda =
      [&called](const std::shared_ptr<my_runner>&)
      {
        called = true;
        return 42;
      };
    std::function<void(cool::ng::async::detail::context*, const return_type&)> reporter = [&result] (cool::ng::async::detail::context* ctx_, const return_type r_)
    {
      result = r_;
    };

    std::unique_ptr<context> ctx(context::create(
        nullptr
      , std::shared_ptr<cool::ng::async::detail::task>()
      , lambda
      , reporter)
    );

    ctx->entry_point(r, nullptr);
    BOOST_CHECK(called);
    BOOST_CHECK_EQUAL(42, result);

  }


}

BOOST_AUTO_TEST_SUITE_END()

