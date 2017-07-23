#include <iostream>
#include <typeinfo>

#define BOOST_TEST_MODULE TaskTraits
#include <boost/test/unit_test.hpp>

#include "cool/ng/impl/task_traits.h"

BOOST_AUTO_TEST_SUITE(task_traits)

BOOST_AUTO_TEST_CASE(is_same)
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

BOOST_AUTO_TEST_CASE(is_chain)
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

BOOST_AUTO_TEST_CASE(parallel_result_type)
{
  BOOST_CHECK(typeid(std::tuple<int, cool::ng::async::detail::traits::void_type, double>) == typeid(cool::ng::async::detail::traits::get_parallel_result_type<A<int>, A<void>, A<double>>::type));
  BOOST_CHECK(typeid(std::tuple<int, void*, double>) == typeid(cool::ng::async::detail::traits::get_parallel_result_type<A<int>, A<void*>, A<double>>::type));
  BOOST_CHECK(typeid(std::tuple<int, void**, double>) == typeid(cool::ng::async::detail::traits::get_parallel_result_type<A<int>, A<void**>, A<double>>::type));

  using tricky_t = void;
  using const_tricky_t = const tricky_t;
  BOOST_CHECK(typeid(std::tuple<int, cool::ng::async::detail::traits::void_type, double>) == typeid(cool::ng::async::detail::traits::get_parallel_result_type<A<int>, A<tricky_t>, A<double>>::type));
  BOOST_CHECK(typeid(std::tuple<int, cool::ng::async::detail::traits::void_type, double>) == typeid(cool::ng::async::detail::traits::get_parallel_result_type<A<int>, A<const_tricky_t>, A<double>>::type));
}

BOOST_AUTO_TEST_CASE(sequence_result_type)
{
  BOOST_CHECK(typeid(double) == typeid(cool::ng::async::detail::traits::get_sequence_result_type<A<int>, A<void>, A<double>>::type));
  BOOST_CHECK(typeid(void) == typeid(cool::ng::async::detail::traits::get_sequence_result_type<A<int>, A<void>, A<void>>::type));
}

BOOST_AUTO_TEST_CASE(get_first)
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

BOOST_AUTO_TEST_CASE(get_last)
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
