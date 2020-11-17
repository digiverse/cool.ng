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

#define BOOST_TEST_MODULE TaskTraits
#include "unit_test_common.h"

#include "cool/ng/impl/async/task_traits.h"

BOOST_AUTO_TEST_SUITE(task_traits)

COOL_AUTO_TEST_CASE(T001,
  *utf::description("is_same test - check if all template types are exactly the same"))
{
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<int, int&>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<int, const int&>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<int&, const int&>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<int*, const int*>::value));
  BOOST_CHECK(  (cool::ng::async::detail::traits::is_same<int,int,int>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<int,void,int>::value));
  BOOST_CHECK(  (cool::ng::async::detail::traits::is_same<std::decay<const int&>::type,int,int>::value));
  BOOST_CHECK(  (cool::ng::async::detail::traits::is_same<std::decay<int&>::type,int>::value));
  // even number of parms
  BOOST_CHECK(  (cool::ng::async::detail::traits::is_same<double, double, double, double, double, double, double>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<double, double, double, double, double, double, int>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<double, double, double, double, double, int, double>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<double, double, double, double, int, double, double>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<double, double, double, int, double, double, double>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<double, double, int, double, double, double, double>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<double, int, double, double, double, double, double>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<int, double, double, double, double, double, double>::value));
  // odd number of parms
  BOOST_CHECK(  (cool::ng::async::detail::traits::is_same<double, double, double, double, double, double>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<double, double, double, double, double, int>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<double, double, double, double, int, double>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<double, double, double, int, double, double>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<double, double, int, double, double, double>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<double, int, double, double, double, double>::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_same<int, double, double, double, double, double>::value));
}

template <typename T, typename Y>
struct C
{
  using result_type = T;
  using input_type = Y;
};

COOL_AUTO_TEST_CASE(T002,
  *utf::description("is_chain test - check that the result type of preceding callable matches the parameter type of next"))
{
  BOOST_CHECK(  (cool::ng::async::detail::traits::is_chain<C<int,int>, C<void, int>>::result::value));
  BOOST_CHECK(  (cool::ng::async::detail::traits::is_chain<C<void,int>, C<int,void>>::result::value));
  BOOST_CHECK(  (cool::ng::async::detail::traits::is_chain<C<int,double>, C<void,int>, C<char, void>, C<bool, char>, C<double, bool>, C<void, double>>::result::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_chain<C<int,double>, C<void,int>, C<char, void>, C<bool, bool>, C<double, bool>, C<void, double>>::result::value));
  BOOST_CHECK(  (cool::ng::async::detail::traits::is_chain<C<int,double>, C<void,int>, C<char, void>>::result::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_chain<C<int,double>, C<int,int>, C<char, void>>::result::value));
  BOOST_CHECK(  (cool::ng::async::detail::traits::is_chain<C<int,int>, C<int,int>, C<int, int>, C<int, int>, C<int, int>, C<int, int>>::result::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_chain<C<int,int>, C<int,int>, C<int, int>, C<int, int>, C<int, int>, C<int, void>>::result::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_chain<C<int,int>, C<int,int>, C<int, int>, C<int, int>, C<int, void>, C<int, int>>::result::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_chain<C<int,int>, C<int,int>, C<int, int>, C<int, void>, C<int, int>, C<int, int>>::result::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_chain<C<int,int>, C<int,int>, C<int, void>, C<int, int>, C<int, int>, C<int, int>>::result::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_chain<C<int,int>, C<int,void>, C<int, int>, C<int, int>, C<int, int>, C<int, int>>::result::value));

  BOOST_CHECK( !(cool::ng::async::detail::traits::is_chain<C<int,int>, C<int,int>, C<int, int>, C<int, int>, C<void, int>, C<int, int>>::result::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_chain<C<int,int>, C<int,int>, C<int, int>, C<void, int>, C<int, int>, C<int, int>>::result::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_chain<C<int,int>, C<int,int>, C<void, int>, C<int, int>, C<int, int>, C<int, int>>::result::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_chain<C<int,int>, C<void,int>, C<int, int>, C<int, int>, C<int, int>, C<int, int>>::result::value));
  BOOST_CHECK( !(cool::ng::async::detail::traits::is_chain<C<void,int>, C<int,int>, C<int, int>, C<int, int>, C<int, int>, C<int, int>>::result::value));
}

template <typename T>
struct A
{
  using result_type = T;
};

COOL_AUTO_TEST_CASE(T003,
  *utf::description("get_parallel_result_type test - determines the result type of the paralel composition"))
{
  BOOST_CHECK(typeid(std::tuple<int, cool::ng::async::detail::traits::void_type, double>) == typeid(cool::ng::async::detail::traits::get_parallel_result_type<A<int>, A<void>, A<double>>::type));
  BOOST_CHECK(typeid(std::tuple<int, void*, double>) == typeid(cool::ng::async::detail::traits::get_parallel_result_type<A<int>, A<void*>, A<double>>::type));
  BOOST_CHECK(typeid(std::tuple<int, void**, double>) == typeid(cool::ng::async::detail::traits::get_parallel_result_type<A<int>, A<void**>, A<double>>::type));

  using tricky_t = void;
  using const_tricky_t = const tricky_t;
  BOOST_CHECK(typeid(std::tuple<int, cool::ng::async::detail::traits::void_type, double>) == typeid(cool::ng::async::detail::traits::get_parallel_result_type<A<int>, A<tricky_t>, A<double>>::type));
  BOOST_CHECK(typeid(std::tuple<int, cool::ng::async::detail::traits::void_type, double>) == typeid(cool::ng::async::detail::traits::get_parallel_result_type<A<int>, A<const_tricky_t>, A<double>>::type));
}

COOL_AUTO_TEST_CASE(T004,
  *utf::description("get_sequence_result_type test - determines the result type of the sequential composition"))
{
  BOOST_CHECK(typeid(double) == typeid(cool::ng::async::detail::traits::get_sequence_result_type<A<int>, A<void>, A<double>>::type));
  BOOST_CHECK(typeid(void) == typeid(cool::ng::async::detail::traits::get_sequence_result_type<A<int>, A<void>, A<void>>::type));
}

COOL_AUTO_TEST_CASE(T005,
  *utf::description("get_first type extractor test - determines the type of the first parameter"))
{
  BOOST_CHECK(typeid(void) == typeid(cool::ng::async::detail::traits::get_first<void>::type));
  BOOST_CHECK(typeid(void) == typeid(cool::ng::async::detail::traits::get_first<void, int>::type));
  BOOST_CHECK(typeid(int) == typeid(cool::ng::async::detail::traits::get_first<int, void>::type));
  BOOST_CHECK(typeid(const char&) == typeid(cool::ng::async::detail::traits::get_first<const char&, int, void>::type));
  BOOST_CHECK(typeid(char&) == typeid(cool::ng::async::detail::traits::get_first<char&, int, void>::type));
  BOOST_CHECK(typeid(char*) == typeid(cool::ng::async::detail::traits::get_first<char*, int, void>::type));
  BOOST_CHECK(typeid(const char*) == typeid(cool::ng::async::detail::traits::get_first<const char*, int, void>::type));
  BOOST_CHECK(!(typeid(char*) == typeid(cool::ng::async::detail::traits::get_first<const char*, int, void>::type)));
  BOOST_CHECK(!(typeid(const char*) == typeid(cool::ng::async::detail::traits::get_first<char*, int, void>::type)));
  BOOST_CHECK(typeid(char) == typeid(cool::ng::async::detail::traits::get_first<char, int, void>::type));

}

COOL_AUTO_TEST_CASE(T006,
  *utf::description("get_last type extractor test - determines the type of the last parameter"))
{
  BOOST_CHECK(typeid(void) == typeid(cool::ng::async::detail::traits::get_last<void>::type));
  BOOST_CHECK(typeid(void) == typeid(cool::ng::async::detail::traits::get_last<int, void>::type));
  BOOST_CHECK(typeid(int) == typeid(cool::ng::async::detail::traits::get_last<void, int>::type));
  BOOST_CHECK(typeid(const char&) == typeid(cool::ng::async::detail::traits::get_last<int, void, const char&>::type));
  BOOST_CHECK(typeid(char&) == typeid(cool::ng::async::detail::traits::get_last<int, void, char&>::type));
  BOOST_CHECK(typeid(char*) == typeid(cool::ng::async::detail::traits::get_last<int, void, char*>::type));
  BOOST_CHECK(typeid(const char*) == typeid(cool::ng::async::detail::traits::get_last<int, void, const char*>::type));
  BOOST_CHECK(!(typeid(char*) == typeid(cool::ng::async::detail::traits::get_last<int, void, const char*>::type)));
  BOOST_CHECK(!(typeid(const char*) == typeid(cool::ng::async::detail::traits::get_last<int, void, char*>::type)));
  BOOST_CHECK(typeid(char) == typeid(cool::ng::async::detail::traits::get_last<int, void, char>::type));
}

BOOST_AUTO_TEST_SUITE_END()
