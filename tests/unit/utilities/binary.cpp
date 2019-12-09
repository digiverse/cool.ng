
#include <stdint.h>
#include <string.h>

#define BOOST_TEST_MODULE Binary
#include <unit_test_common.h>

#include "cool/ng/binary.h"

using namespace cool::ng::util;


BOOST_AUTO_TEST_SUITE(binary_)

COOL_AUTO_TEST_CASE(T001,
  *utf::description("ctor binary()"))
{
  {
    binary<4> b;
    uint8_t aux[] = { 0, 0, 0, 0 };
    BOOST_CHECK(!::memcmp(b.data(), aux, b.size()));
  }
}

COOL_AUTO_TEST_CASE(T002,
  *utf::description("ctor binary(const uint8_t[])"))
{
  {
    uint8_t aux[] = { 1, 2, 3, 4 };
    binary<4> b(aux);
    BOOST_CHECK(!::memcmp(b.data(), aux, b.size()));
  }
}

COOL_AUTO_TEST_CASE(T003,
  *utf::description("ctor binary(const uint8_t[], std::size_t)"))
{
  {
    uint8_t aux[] = { 1, 2, 3, 4, 5, 6 };
    binary<4> b(aux, 6);
    BOOST_CHECK(!::memcmp(b.data(), aux + 2, b.size()));
  }
  {
    uint8_t aux[] = { 0, 1, 2, 3, 4, 5, 6 };
    binary<4> b(aux + 1, 3);
    BOOST_CHECK(!::memcmp(b.data(), aux, b.size()));
  }
  {
    uint8_t aux[] = { 1, 2, 3, 4 };
    binary<4> b(aux, 4);
    BOOST_CHECK(!::memcmp(b.data(), aux, b.size()));
  }
}

COOL_AUTO_TEST_CASE(T004,
  *utf::description("ctor binary(std::initializer_list<uint8_t>)"))
{
  {
    uint8_t aux[] = { 6, 7, 8, 9 };
    binary<4> b({ 6, 7, 8, 9});
    BOOST_CHECK(!::memcmp(b.data(), aux, b.size()));
  }
  {
    uint8_t aux[] = { 8, 9, 10, 11 };
    binary<4> b({ 6, 7, 8, 9, 10, 11});
    BOOST_CHECK(!::memcmp(b.data(), aux, b.size()));
  }
  {
    uint8_t aux[] = { 0, 6, 7, 8 };
    binary<4> b({6, 7, 8});
    BOOST_CHECK(!::memcmp(b.data(), aux, b.size()));
  }
}

COOL_AUTO_TEST_CASE(T005,
  *utf::description("template <OSize> ctor binary(const std::array<uint8_t, OSize>&)"))
{
  {
    binary<4> aux({ 6, 7, 8, 9 });
    binary<4> b(aux);
    BOOST_CHECK(!::memcmp(b.data(), aux.data(), b.size()));
  }
  {
    uint8_t aux[] = { 8, 9, 10, 11 };
    binary<6> a({ 6, 7, 8, 9, 10, 11});
    binary<4> b(a);
    BOOST_CHECK(!::memcmp(b.data(), aux, b.size()));
  }
  {
    uint8_t aux[] = { 0, 6, 7, 8 };
    binary<3> a({6, 7, 8});
    binary<4> b(a);
    BOOST_CHECK(!::memcmp(b.data(), aux, b.size()));
  }
}

COOL_AUTO_TEST_CASE(T006,
  *utf::description("operator =(const uint8_t[])"))
{
  {
    uint8_t aux[] = { 1, 2, 3, 4 };
    binary<4> b({ 0xb, 0xe, 0xe, 0xf});
    BOOST_CHECK_NO_THROW(b = aux);
    BOOST_CHECK(!::memcmp(b.data(), aux, b.size()));
  }
}

COOL_AUTO_TEST_CASE(T007,
  *utf::description("template <OSize> operator=(const std::array<uint8_t, OSize>&)"))
{
  {
    binary<4> aux({ 6, 7, 8, 9 });
    binary<4> b({ 0xb, 0xe, 0xe, 0xf});
    BOOST_CHECK_NO_THROW(b = aux);
    BOOST_CHECK(!::memcmp(b.data(), aux.data(), b.size()));
  }
  {
    uint8_t aux[] = { 0xc, 0xd, 0xe, 0xf };
    binary<6> a({ 0xa, 0xb, 0xc, 0xd, 0xe, 0xf });
    binary<4> b;
    BOOST_CHECK_NO_THROW(b = a);
    BOOST_CHECK(!::memcmp(b.data(), aux, b.size()));
  }
  {
    uint8_t aux[] = { 42, 0xb, 0xe, 0xe };
    binary<3> a({ 0xb, 0xe, 0xe });
    binary<4> b({ 42, 1, 2, 3});
    BOOST_CHECK_NO_THROW(b = a);
    BOOST_CHECK(!::memcmp(b.data(), aux, b.size()));
  }
}

COOL_AUTO_TEST_CASE(T008,
  *utf::description("comparison operators"))
{
  uint8_t ref[] = { 1, 2, 3, 4};
  uint8_t arr[] = { 1, 2, 3, 4, 5 };
  {
    binary<4> a = arr + 1;
    binary<4> b = arr;
    binary<4> c = ref;

    BOOST_CHECK(!(a <  arr));
    BOOST_CHECK(!(a <= arr));
    BOOST_CHECK( (a >  arr));
    BOOST_CHECK( (a >= arr));
    BOOST_CHECK(!(a == arr));
    BOOST_CHECK( (a != arr));

    BOOST_CHECK(!(arr >  a));
    BOOST_CHECK(!(arr >= a));
    BOOST_CHECK( (arr <  a));
    BOOST_CHECK( (arr <= a));
    BOOST_CHECK(!(arr == a));
    BOOST_CHECK( (arr != a));

    BOOST_CHECK(!(a <  arr + 1));
    BOOST_CHECK( (a <= arr + 1));
    BOOST_CHECK(!(a >  arr + 1));
    BOOST_CHECK( (a >= arr + 1));
    BOOST_CHECK( (a == arr + 1));
    BOOST_CHECK(!(a != arr + 1));

    BOOST_CHECK(!(arr + 1 >  a));
    BOOST_CHECK( (arr + 1 >= a));
    BOOST_CHECK(!(arr + 1 <  a));
    BOOST_CHECK( (arr + 1 <= a));
    BOOST_CHECK( (arr + 1 == a));
    BOOST_CHECK(!(arr + 1 != a));

    BOOST_CHECK( (b <  a));
    BOOST_CHECK( (b <= a));
    BOOST_CHECK(!(b >  a));
    BOOST_CHECK(!(b >= a));
    BOOST_CHECK(!(b == a));
    BOOST_CHECK( (b != a));

    BOOST_CHECK(!(b <  c));
    BOOST_CHECK( (b <= c));
    BOOST_CHECK(!(b >  c));
    BOOST_CHECK( (b >= c));
    BOOST_CHECK( (b == c));
    BOOST_CHECK(!(b != c));

    binary<3> bb = arr + 2;
    binary<5> cc = arr;

    BOOST_CHECK(!(a <  bb));
    BOOST_CHECK(!(a <= bb));
    BOOST_CHECK( (a >  bb));
    BOOST_CHECK( (a >= bb));
    BOOST_CHECK(!(a == bb));
    BOOST_CHECK( (a != bb));

    BOOST_CHECK( (a <  cc));
    BOOST_CHECK( (a <= cc));
    BOOST_CHECK(!(a >  cc));
    BOOST_CHECK(!(a >= cc));
    BOOST_CHECK(!(a == cc));
    BOOST_CHECK( (a != cc));
  }
}

COOL_AUTO_TEST_CASE(T009,
   *utf::description("operator &=(uint8_t const [])"))
{
  uint8_t ref[] = { 0x10, 0x46, 0x00, 0x01};
  uint8_t arr[] = { 0x90, 0xff, 0x01, 0x33 };

  {
    binary<4> b( { 0x51, 0x46, 0x00, 0x01 });
    b &= arr;
    BOOST_CHECK_EQUAL(b, ref);
  }
  {
    uint8_t b_a[] = { 0x51, 0x46, 0x00, 0x01 };
    binary<4> b(b_a);
    auto res = b & arr;
    BOOST_CHECK_EQUAL(res, ref);
    BOOST_CHECK_EQUAL(b, binary<4>(b_a));
  }
}

COOL_AUTO_TEST_CASE(T010,
   *utf::description("template <Size> operator &=(const std::array<uint8_t, Size>&)"))
{
  uint8_t ref[] = { 0x10, 0x46, 0x00, 0x01};
  uint8_t arr[] = { 0x90, 0xff, 0x01, 0x33 };
  // -- same size
  {
    binary<4> b( { 0x51, 0x46, 0x00, 0x01 });
    binary<4> a( arr );
    b &= a;
    BOOST_CHECK_EQUAL(b, binary<4>(ref));
  }
  {
    uint8_t b_a[] = { 0x51, 0x46, 0x00, 0x01 };
    binary<4> b(b_a);
    binary<4> a(arr);
    auto res = b & a;
    BOOST_CHECK_EQUAL(res, ref);
    BOOST_CHECK_EQUAL(b, binary<4>(b_a));
  }
  // -- other is larger
  {
    binary<4> b( { 0x51, 0x46, 0x00, 0x01 });
    binary<6> a( { 0x30, 0x21, 0x90, 0xff, 0x01, 0x33 } );
    b &= a;
    BOOST_CHECK_EQUAL(b, binary<4>(ref));
  }
  {
    uint8_t b_a[] = { 0x51, 0x46, 0x00, 0x01 };
    binary<4> b(b_a);
    binary<6> a({ 0x30, 0x21, 0x90, 0xff, 0x01, 0x33 });
    auto res = b & a;
    BOOST_CHECK_EQUAL(res, ref);
    BOOST_CHECK_EQUAL(b, binary<4>(b_a));
  }
  // -- other is smaller
  {
    binary<4> b( { 0x51, 0x46, 0x00, 0x01 });
    binary<3> a( {       0xff, 0x01, 0x33 } );
    b &= a;
    BOOST_CHECK_EQUAL(b, binary<4>({ 0x51, 0x46, 0x00, 0x01}));
  }
  {
    uint8_t b_a[] = { 0x51, 0x46, 0x80, 0x01 };
    binary<4> b(b_a);
    binary<2> a({ 0x01, 0x33 });
    auto res = b & a;
    BOOST_CHECK_EQUAL(res, binary<4>({ 0x51, 0x46,  0x00, 0x01}));
    BOOST_CHECK_EQUAL(b, binary<4>(b_a));
  }
  {
    uint8_t b_a[] = { 0x51, 0x46, 0x80, 0x01 };
    binary<4> b(b_a);
    binary<2> a({ 0x01, 0x33 });
    auto res = a & b;
    BOOST_CHECK_EQUAL(res, binary<2>({0x00, 0x01}));
    BOOST_CHECK_EQUAL(a, binary<2>({ 0x01, 0x33 }));
  }
}

COOL_AUTO_TEST_CASE(T011,
   *utf::description("operator |=(uint8_t const [])"))
{
  uint8_t ref[] = { 0xd1, 0xff, 0x01, 0x33};
  uint8_t arr[] = { 0x90, 0xff, 0x01, 0x33 };

  {
    binary<4> b( { 0x51, 0x46, 0x00, 0x01 });
    b |= arr;
    BOOST_CHECK_EQUAL(b, binary<4>(ref));
  }
  {
    uint8_t b_a[] = { 0x51, 0x46, 0x00, 0x01 };
    binary<4> b(b_a);
    auto res = b | arr;
    BOOST_CHECK_EQUAL(res, binary<4>(ref));
    BOOST_CHECK_EQUAL(b, binary<4>(b_a));
  }
}

COOL_AUTO_TEST_CASE(T012,
*utf::description("template <Size> operator |=(const std::array<uint8_t, Size>&)"))
{
  binary<4> ref_4   = {             0xd1, 0xff, 0x01, 0x33 };
  binary<4> ref_4_3 = {             0x51, 0xff, 0x01, 0x33 };
  binary<4> ref_4_2 = {             0x51, 0x46, 0x01, 0x33 };
  binary<2> ref_2   = {                         0x01, 0x33 };
  binary<4> a_4     = {             0x90, 0xff, 0x01, 0x33 };
  binary<4> b_4     = {             0x51, 0x46, 0x00, 0x01 };
  binary<3> a_3     = {                   0xff, 0x01, 0x33 };
  binary<6> a_6     = { 0x30, 0x21, 0x90, 0xff, 0x01, 0x33 };
  binary<2> a_2     = {                         0x01, 0x33 };

  // -- same size
  {
    auto b = b_4;
    b |= a_4;
    BOOST_CHECK_EQUAL(b, ref_4);
  }
  {
    auto b = b_4;
    auto a = a_4;
    auto res = b | a;
    BOOST_CHECK_EQUAL(res, ref_4);
    BOOST_CHECK_EQUAL(b, b_4);
    BOOST_CHECK_EQUAL(a, a_4);
  }
  // -- other is larger
  {
    auto b = b_4;
    auto a = a_6;;
    b |= a;
    BOOST_CHECK_EQUAL(b, ref_4);
  }
  {
    auto b = b_4;
    auto a = a_6;;

    auto res = b | a;
    BOOST_CHECK_EQUAL(res, ref_4);
    BOOST_CHECK_EQUAL(b, b_4);
    BOOST_CHECK_EQUAL(a, a_6);
  }
  // -- other is smaller
  {
    auto b = b_4;
    auto a = a_3;
    b |= a;
    BOOST_CHECK_EQUAL(b, ref_4_3);
  }
  {
    auto b = b_4;
    auto a = a_2;
    auto res = b | a;
    BOOST_CHECK_EQUAL(res, ref_4_2);
    BOOST_CHECK_EQUAL(b, b_4);
    BOOST_CHECK_EQUAL(a, a_2);
  }
  {
    auto b = b_4;
    auto a = a_2;
    auto res = a | b;
    BOOST_CHECK_EQUAL(res, ref_2);
    BOOST_CHECK_EQUAL(b, b_4);
    BOOST_CHECK_EQUAL(a, a_2);
  }
}

COOL_AUTO_TEST_CASE(T013,
   *utf::description("operator ^=(uint8_t const [])"))
{
  uint8_t ref[] = { 0xc1, 0xb9, 0x01, 0x32};
  uint8_t arr[] = { 0x90, 0xff, 0x01, 0x33 };

  {
    binary<4> b( { 0x51, 0x46, 0x00, 0x01 });
    b ^= arr;
    BOOST_CHECK_EQUAL(b, binary<4>(ref));
  }
  {
    uint8_t b_a[] = { 0x51, 0x46, 0x00, 0x01 };
    binary<4> b(b_a);
    auto res = b ^ arr;
    BOOST_CHECK_EQUAL(res, binary<4>(ref));
    BOOST_CHECK_EQUAL(b, binary<4>(b_a));
  }
}

COOL_AUTO_TEST_CASE(T014,
*utf::description("template <Size> operator ^=(const std::array<uint8_t, Size>&)"))
{
  binary<4> ref_4   = {             0xc1, 0xb9, 0x01, 0x32 };
  binary<4> ref_4_3 = {             0x51, 0xb9, 0x01, 0x32 };
  binary<4> ref_4_2 = {             0x51, 0x46, 0x01, 0x32 };
  binary<2> ref_2   = {                         0x01, 0x32 };
  binary<4> a_4     = {             0x90, 0xff, 0x01, 0x33 };
  binary<4> b_4     = {             0x51, 0x46, 0x00, 0x01 };
  binary<3> a_3     = {                   0xff, 0x01, 0x33 };
  binary<6> a_6     = { 0x30, 0x21, 0x90, 0xff, 0x01, 0x33 };
  binary<2> a_2     = {                         0x01, 0x33 };

  // -- same size
  {
    auto b = b_4;
    b ^= a_4;
    BOOST_CHECK_EQUAL(b, ref_4);
  }
  {
    auto b = b_4;
    auto a = a_4;
    auto res = b ^ a;
    BOOST_CHECK_EQUAL(res, ref_4);
    BOOST_CHECK_EQUAL(b, b_4);
    BOOST_CHECK_EQUAL(a, a_4);
  }
  // -- other is larger
  {
    auto b = b_4;
    auto a = a_6;;
    b ^= a;
    BOOST_CHECK_EQUAL(b, ref_4);
  }
  {
    auto b = b_4;
    auto a = a_6;;

    auto res = b ^ a;
    BOOST_CHECK_EQUAL(res, ref_4);
    BOOST_CHECK_EQUAL(b, b_4);
    BOOST_CHECK_EQUAL(a, a_6);
  }
  // -- other is smaller
  {
    auto b = b_4;
    auto a = a_3;
    b ^= a;
    BOOST_CHECK_EQUAL(b, ref_4_3);
  }
  {
    auto b = b_4;
    auto a = a_2;
    auto res = b ^ a;
    BOOST_CHECK_EQUAL(res, ref_4_2);
    BOOST_CHECK_EQUAL(b, b_4);
    BOOST_CHECK_EQUAL(a, a_2);
  }
  {
    auto b = b_4;
    auto a = a_2;
    auto res = a ^ b;
    BOOST_CHECK_EQUAL(res, ref_2);
    BOOST_CHECK_EQUAL(b, b_4);
    BOOST_CHECK_EQUAL(a, a_2);
  }
}

COOL_AUTO_TEST_CASE(T015,
  *utf::description("operator ~(const std::array<uint8_t, ArrSize>&)"))
{
  {
    binary<4> a = { 0xc1, 0xb9, 0x01, 0x32 };
    auto b = ~a;
    BOOST_CHECK_EQUAL(b, binary<4>({ 0x3e, 0x46, 0xfe, 0xcd}));
  }
}

COOL_AUTO_TEST_CASE(T016,
  *utf::description("visualize(std::ostream&, style)"))
{
  using bin = binary<4>;

  const bin a = { 0xc1, 0xb9, 0x01, 0x32 };
  {
    std::stringstream ss;
    a.visualize(ss, bin::style::decimal);
    BOOST_CHECK_EQUAL(ss.str(), "{ 193, 185, 1, 50 }");
  }
  {
    std::stringstream ss;
    a.visualize(ss, bin::style::hex);
    BOOST_CHECK_EQUAL(ss.str(), "{ 0xc1, 0xb9, 0x01, 0x32 }");
  }
  {
    std::stringstream ss;
    a.visualize(ss, bin::style::hex_no_prefix);
    BOOST_CHECK_EQUAL(ss.str(), "{ c1, b9, 01, 32 }");
  }
  {
    std::stringstream ss;
    a.visualize(ss, bin::style::octal);
    BOOST_CHECK_EQUAL(ss.str(), "{ 0301, 0271, 01, 062 }");
  }
  {
    std::stringstream ss;
    a.visualize(ss, bin::style::octal_no_prefix);
    BOOST_CHECK_EQUAL(ss.str(), "{ 301, 271, 1, 62 }");
  }
}

COOL_AUTO_TEST_CASE(T017,
  *utf::description("operator <<(std::ostream&, const binary&)"))
{
  using bin = binary<4>;

  const bin a = { 0xc1, 0xb9, 0x01, 0x32 };
  {
    std::stringstream ss;
    ss << a;
    BOOST_CHECK_EQUAL(ss.str(), "{ 0xc1, 0xb9, 0x01, 0x32 }");
  }
}

BOOST_AUTO_TEST_SUITE_END()


