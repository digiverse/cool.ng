#include <iostream>
#include <typeinfo>

#define BOOST_TEST_MODULE Functional
#include <boost/test/unit_test.hpp>

#include "cool/ng/traits.h"


BOOST_AUTO_TEST_SUITE(function_pointer)

using fp_void_0  = void (*)();
using fp_float_0 = float (*)();
using fp_void_1  = void (*)(double);
using fp_int_1 = int (*)(char*);
using fp_void_6  = void (*)(double, int, int*, int&, const int&, const int*);
using fp_ulong_6 = unsigned long (*)(char*, unsigned char*, char, unsigned char, double, void*);

BOOST_AUTO_TEST_CASE(result_type)
{
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_void_0>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_float_0>::result_type) == typeid(float));

  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_void_1>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_int_1>::result_type) == typeid(int));

  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_void_6>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_ulong_6>::result_type) == typeid(unsigned long));
}

BOOST_AUTO_TEST_CASE(arity)
{
  BOOST_CHECK_EQUAL(0, cool::ng::traits::functional<fp_void_0>::arity::value);
  BOOST_CHECK_EQUAL(0, cool::ng::traits::functional<fp_float_0>::arity::value);

  BOOST_CHECK_EQUAL(1, cool::ng::traits::functional<fp_void_1>::arity::value);
  BOOST_CHECK_EQUAL(1, cool::ng::traits::functional<fp_int_1>::arity::value);

  BOOST_CHECK_EQUAL(6, cool::ng::traits::functional<fp_void_6>::arity::value);
  BOOST_CHECK_EQUAL(6, cool::ng::traits::functional<fp_ulong_6>::arity::value);
}

BOOST_AUTO_TEST_CASE(arg_type)
{
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_void_1>::arg<0>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_int_1>::arg<0>::type) == typeid(char*));

  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_void_6>::arg<0>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_void_6>::arg<1>::type) == typeid(int));
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_void_6>::arg<2>::type) == typeid(int*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_void_6>::arg<3>::type) == typeid(int&));
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_void_6>::arg<4>::type) == typeid(const int&));
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_void_6>::arg<5>::type) == typeid(const int*));

  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_ulong_6>::arg<0>::type) == typeid(char*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_ulong_6>::arg<1>::type) == typeid(unsigned char *));
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_ulong_6>::arg<2>::type) == typeid(char));
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_ulong_6>::arg<3>::type) == typeid(unsigned char));
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_ulong_6>::arg<4>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<fp_ulong_6>::arg<5>::type) == typeid(void*));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(member_function_pointer)

class member_functions
{
 public:
  void void_0() { }
  float float_0() { return 0; }
  void void_1(double) { }
  int int_1(char*) { return 0; }
  void void_6(double, int, int*, int&, unsigned int*, short) { }
  unsigned long ulong_6(char*, unsigned char*, char, unsigned char, double, void*) { return 0; }
};

BOOST_AUTO_TEST_CASE(result_type)
{
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_0)>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::float_0)>::result_type) == typeid(float));

  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_1)>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::int_1)>::result_type) == typeid(int));

  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::result_type) == typeid(unsigned long));
}

// Note: member functions have one implicit parameter (this pointer)
BOOST_AUTO_TEST_CASE(arity)
{
  BOOST_CHECK_EQUAL(1, cool::ng::traits::functional<decltype(&member_functions::void_0)>::arity::value);
  BOOST_CHECK_EQUAL(1, cool::ng::traits::functional<decltype(&member_functions::float_0)>::arity::value);

  BOOST_CHECK_EQUAL(2, cool::ng::traits::functional<decltype(&member_functions::void_1)>::arity::value);
  BOOST_CHECK_EQUAL(2, cool::ng::traits::functional<decltype(&member_functions::int_1)>::arity::value);

  BOOST_CHECK_EQUAL(7, cool::ng::traits::functional<decltype(&member_functions::void_6)>::arity::value);
  BOOST_CHECK_EQUAL(7, cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arity::value);
}

BOOST_AUTO_TEST_CASE(arg_type)
{
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_0)>::arg<0>::type) == typeid(member_functions*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::float_0)>::arg<0>::type) == typeid(member_functions*));

  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_1)>::arg<0>::type) == typeid(member_functions*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_1)>::arg<1>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::int_1)>::arg<0>::type) == typeid(member_functions*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::int_1)>::arg<1>::type) == typeid(char*));

  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::arg<0>::type) == typeid(member_functions*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::arg<1>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::arg<2>::type) == typeid(int));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::arg<3>::type) == typeid(int*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::arg<4>::type) == typeid(int&));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::arg<5>::type) == typeid(unsigned int*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::arg<6>::type) == typeid(short));

  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arg<0>::type) == typeid(member_functions*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arg<1>::type) == typeid(char*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arg<2>::type) == typeid(unsigned char *));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arg<3>::type) == typeid(char));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arg<4>::type) == typeid(unsigned char));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arg<5>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arg<6>::type) == typeid(void*));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(member_const_function_pointer)

class member_functions
{
public:
  void void_0() const { }
  float float_0() const { return 0; }
  void void_1(double) const { }
  int int_1(char*) const { return 0; }
  void void_6(double, int, int*, int&, unsigned int*, short) const { }
  unsigned long ulong_6(char*, unsigned char*, char, unsigned char, double, void*) const { return 0; }
};

BOOST_AUTO_TEST_CASE(result_type)
{
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_0)>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::float_0)>::result_type) == typeid(float));

  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_1)>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::int_1)>::result_type) == typeid(int));

  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::result_type) == typeid(unsigned long));
}

// Note: member functions have one implicit parameter (this pointer)
BOOST_AUTO_TEST_CASE(arity)
{
  BOOST_CHECK_EQUAL(1, cool::ng::traits::functional<decltype(&member_functions::void_0)>::arity::value);
  BOOST_CHECK_EQUAL(1, cool::ng::traits::functional<decltype(&member_functions::float_0)>::arity::value);

  BOOST_CHECK_EQUAL(2, cool::ng::traits::functional<decltype(&member_functions::void_1)>::arity::value);
  BOOST_CHECK_EQUAL(2, cool::ng::traits::functional<decltype(&member_functions::int_1)>::arity::value);

  BOOST_CHECK_EQUAL(7, cool::ng::traits::functional<decltype(&member_functions::void_6)>::arity::value);
  BOOST_CHECK_EQUAL(7, cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arity::value);
}

BOOST_AUTO_TEST_CASE(arg_type)
{
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_0)>::arg<0>::type) == typeid(const member_functions*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::float_0)>::arg<0>::type) == typeid(const member_functions*));

  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_1)>::arg<0>::type) == typeid(const member_functions*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_1)>::arg<1>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::int_1)>::arg<0>::type) == typeid(const member_functions*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::int_1)>::arg<1>::type) == typeid(char*));

  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::arg<0>::type) == typeid(const member_functions*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::arg<1>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::arg<2>::type) == typeid(int));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::arg<3>::type) == typeid(int*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::arg<4>::type) == typeid(int&));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::arg<5>::type) == typeid(unsigned int*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::void_6)>::arg<6>::type) == typeid(short));

  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arg<0>::type) == typeid(const member_functions*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arg<1>::type) == typeid(char*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arg<2>::type) == typeid(unsigned char *));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arg<3>::type) == typeid(char));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arg<4>::type) == typeid(unsigned char));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arg<5>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&member_functions::ulong_6)>::arg<6>::type) == typeid(void*));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(member_pointers)

class members {
 public:
  void* member;
};

BOOST_AUTO_TEST_CASE(member_type)
{
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&members::member)>::result_type) == typeid(void*));
}

BOOST_AUTO_TEST_CASE(arity)
{
  BOOST_CHECK_EQUAL(1, cool::ng::traits::functional<decltype(&members::member)>::arity::value);
}

BOOST_AUTO_TEST_CASE(arg_type)
{
  BOOST_CHECK(typeid(cool::ng::traits::functional<decltype(&members::member)>::arg<0>::type) == typeid(members*));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(functors)

struct void_0 { void operator () () { } };
struct float_0 { float operator () () { return 0; } };
struct void_1 { void operator () (double) { } };
struct int_1 { int operator() (char*) { return 0; } };
struct void_6 { void operator () (double, int, int*, int&, unsigned int*, short) const { } };
struct ulong_6 { unsigned long operator() (char*, unsigned char*, char, unsigned char, double, void*) const { return 0; } };

BOOST_AUTO_TEST_CASE(result_type)
{
  BOOST_CHECK(typeid(cool::ng::traits::functional<void_0>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<float_0>::result_type) == typeid(float));

  BOOST_CHECK(typeid(cool::ng::traits::functional<void_1>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<int_1>::result_type) == typeid(int));

  BOOST_CHECK(typeid(cool::ng::traits::functional<void_6>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<ulong_6>::result_type) == typeid(unsigned long));
}

BOOST_AUTO_TEST_CASE(arity)
{
  BOOST_CHECK_EQUAL(0, cool::ng::traits::functional<void_0>::arity::value);
  BOOST_CHECK_EQUAL(0, cool::ng::traits::functional<float_0>::arity::value);

  BOOST_CHECK_EQUAL(1, cool::ng::traits::functional<void_1>::arity::value);
  BOOST_CHECK_EQUAL(1, cool::ng::traits::functional<int_1>::arity::value);

  BOOST_CHECK_EQUAL(6, cool::ng::traits::functional<void_6>::arity::value);
  BOOST_CHECK_EQUAL(6, cool::ng::traits::functional<ulong_6>::arity::value);
}

BOOST_AUTO_TEST_CASE(arg_type)
{
  BOOST_CHECK(typeid(cool::ng::traits::functional<void_1>::arg<0>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<int_1>::arg<0>::type) == typeid(char*));

  BOOST_CHECK(typeid(cool::ng::traits::functional<void_6>::arg<0>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<void_6>::arg<1>::type) == typeid(int));
  BOOST_CHECK(typeid(cool::ng::traits::functional<void_6>::arg<2>::type) == typeid(int*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<void_6>::arg<3>::type) == typeid(int&));
  BOOST_CHECK(typeid(cool::ng::traits::functional<void_6>::arg<4>::type) == typeid(unsigned int*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<void_6>::arg<5>::type) == typeid(short));

  BOOST_CHECK(typeid(cool::ng::traits::functional<ulong_6>::arg<0>::type) == typeid(char*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<ulong_6>::arg<1>::type) == typeid(unsigned char *));
  BOOST_CHECK(typeid(cool::ng::traits::functional<ulong_6>::arg<2>::type) == typeid(char));
  BOOST_CHECK(typeid(cool::ng::traits::functional<ulong_6>::arg<3>::type) == typeid(unsigned char));
  BOOST_CHECK(typeid(cool::ng::traits::functional<ulong_6>::arg<4>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<ulong_6>::arg<5>::type) == typeid(void*));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(std_function_functors)

using void_0 = std::function<void()>;
using float_0 = std::function<float()>;
using void_1 = std::function<void(double)>;
using int_1 = std::function<int(char*)>;
using void_6 = std::function<void(double, int, int*, int&, unsigned int*, short)>;
using ulong_6 = std::function<unsigned long(char*, unsigned char*, char, unsigned char, double, void*)>;

BOOST_AUTO_TEST_CASE(result_type)
{
  BOOST_CHECK(typeid(cool::ng::traits::functional<void_0>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<float_0>::result_type) == typeid(float));

  BOOST_CHECK(typeid(cool::ng::traits::functional<void_1>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<int_1>::result_type) == typeid(int));

  BOOST_CHECK(typeid(cool::ng::traits::functional<void_6>::result_type) == typeid(void));
  BOOST_CHECK(typeid(cool::ng::traits::functional<ulong_6>::result_type) == typeid(unsigned long));
}

BOOST_AUTO_TEST_CASE(arity)
{
  BOOST_CHECK_EQUAL(0, cool::ng::traits::functional<void_0>::arity::value);
  BOOST_CHECK_EQUAL(0, cool::ng::traits::functional<float_0>::arity::value);

  BOOST_CHECK_EQUAL(1, cool::ng::traits::functional<void_1>::arity::value);
  BOOST_CHECK_EQUAL(1, cool::ng::traits::functional<int_1>::arity::value);

  BOOST_CHECK_EQUAL(6, cool::ng::traits::functional<void_6>::arity::value);
  BOOST_CHECK_EQUAL(6, cool::ng::traits::functional<ulong_6>::arity::value);
}

BOOST_AUTO_TEST_CASE(arg_type)
{
  BOOST_CHECK(typeid(cool::ng::traits::functional<void_1>::arg<0>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<int_1>::arg<0>::type) == typeid(char*));

  BOOST_CHECK(typeid(cool::ng::traits::functional<void_6>::arg<0>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<void_6>::arg<1>::type) == typeid(int));
  BOOST_CHECK(typeid(cool::ng::traits::functional<void_6>::arg<2>::type) == typeid(int*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<void_6>::arg<3>::type) == typeid(int&));
  BOOST_CHECK(typeid(cool::ng::traits::functional<void_6>::arg<4>::type) == typeid(unsigned int*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<void_6>::arg<5>::type) == typeid(short));

  BOOST_CHECK(typeid(cool::ng::traits::functional<ulong_6>::arg<0>::type) == typeid(char*));
  BOOST_CHECK(typeid(cool::ng::traits::functional<ulong_6>::arg<1>::type) == typeid(unsigned char *));
  BOOST_CHECK(typeid(cool::ng::traits::functional<ulong_6>::arg<2>::type) == typeid(char));
  BOOST_CHECK(typeid(cool::ng::traits::functional<ulong_6>::arg<3>::type) == typeid(unsigned char));
  BOOST_CHECK(typeid(cool::ng::traits::functional<ulong_6>::arg<4>::type) == typeid(double));
  BOOST_CHECK(typeid(cool::ng::traits::functional<ulong_6>::arg<5>::type) == typeid(void*));
}

BOOST_AUTO_TEST_SUITE_END()

