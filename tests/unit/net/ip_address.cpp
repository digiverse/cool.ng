
#include <stdint.h>
#include <iostream>
#include <sstream>

#if !defined(WINDOWS_TARGET)

#include <arpa/inet.h>

#else

# pragma comment(lib, "Ws2_32.lib")

#endif

#define BOOST_TEST_MODULE IpAddress
#include <unit_test_common.h>

#include "cool/ng/ip_address.h"

using namespace cool::ng::ip;

ipv6::host ip6_host_examples[] = {
  { 0x02, 0x00, 0x00, 0x00,/**/ 0x00, 0x00, 0x12, 0x23,/**/ 0x34, 0x56, 0x78, 0x9a,/**/ 0xbc, 0xde, 0xf0, 0x01 },
  { 0x02, 0x03, 0x00, 0x00,/**/ 0x00, 0x00, 0x12, 0x23,/**/ 0x34, 0x00, 0x00, 0x00,/**/ 0x00, 0x00, 0xf0, 0x01 },
  { 0xab, 0x03, 0x00, 0x00,/**/ 0x00, 0x00, 0x12, 0x34,/**/ 0x56, 0x78, 0x00, 0x00,/**/ 0x00, 0x00, 0x00, 0x00 },
  { 0xab, 0x03, 0x00, 0x00,/**/ 0x00, 0x00, 0x12, 0x34,/**/ 0x56, 0x78, 0x00, 0x01,/**/ 0x00, 0x00, 0x00, 0x00 },
  { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0xff, 0xff, 0x00, 0x00,     0x0a, 0x0b, 0x0c, 0x0d },
  { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0x0a, 0x0b, 0x0c, 0x0d },
  { 0x00, 0x64, 0xff, 0x9b,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,      172,    4,   12,   75 }
};

const char* ip6_host_input[] = {
  "::1",
  "[::1]:80",
  "2605:2700:0:3::4713:93e3",
  "[2605:2700:0:3::4713:93e3]:80",
  "::ffff:192.168.173.22",
  "[::ffff:192.168.173.22]:80",
  "::192.168.173.22:80",
  "ffff::192.168.173.22:80",
  "1::",
  "[1::]:80",
  "::",
  "[::]:80",
  "2605-2700-0-3--4713-93e3",
  "2605-2700-0-3--4713-93e3.ipv6-literal.net",
  "2605:2700:abcd::",
  nullptr
};

std::string str(const cool::ng::ip::address& addr, style style_)
{
  std::stringstream ss;
  addr.visualize(ss, style_);
  return ss.str();
}

std::string visualize(const address& a_, style s_)
{
  std::stringstream ss;
  a_.visualize(ss, s_);
  return ss.str();
}

std::string visualize(const address& a_)
{
  std::stringstream ss;
  a_.visualize(ss);
  return ss.str();
}

BOOST_AUTO_TEST_SUITE(address_)

COOL_AUTO_TEST_CASE(T001,
  *utf::description("operator =(const address&)"))
{
  const ipv4::host ra4("192.168.3.99");
  const ipv6::host ra6("2605:2700:0:3::4713:93e3");
  const ipv6::host ra6_mapped("::ffff:172.14.3.1");
  const ipv6::host ra6_translated("64:ff9b::172.14.5.99");
  const ipv6::network rn6("2605:2700:0:3::4713:93e3:0/16");
  const ipv4::network rn4("192.168.0.0/16");

  {
    ipv4::host a4;

    BOOST_CHECK_NO_THROW(a4 = ra4);
    BOOST_CHECK_EQUAL(a4, ra4);

    BOOST_CHECK_THROW(a4 = ra6, cool::ng::exception::bad_conversion);
    BOOST_CHECK_THROW(a4 = rn4, cool::ng::exception::bad_conversion);
    BOOST_CHECK_THROW(a4 = rn6, cool::ng::exception::bad_conversion);
    BOOST_CHECK_NO_THROW(a4 = ra6_mapped);
    BOOST_CHECK_EQUAL(a4, ipv4::host("172.14.3.1"));
    BOOST_CHECK_NO_THROW(a4 = ra6_translated);
    BOOST_CHECK_EQUAL(a4, ipv4::host("172.14.5.99"));
  }
  {
    ipv6::host a6;

    BOOST_CHECK_NO_THROW(a6 = ra6);
    BOOST_CHECK_EQUAL(a6, ra6);

    BOOST_CHECK_THROW(a6 = rn4, cool::ng::exception::bad_conversion);
    BOOST_CHECK_THROW(a6 = rn6, cool::ng::exception::bad_conversion);
    BOOST_CHECK_NO_THROW(a6 = ra4);
    BOOST_CHECK(a6.in(ipv6::rfc_ipv4map));
  }
  {
    {
      ipv4::network n4(24);

      BOOST_CHECK_NO_THROW(n4 = rn4);
      BOOST_CHECK_EQUAL(n4, rn4);
      BOOST_CHECK_EQUAL(n4.mask(), 16);
    }
    {
      ipv4::network n4(24);

      BOOST_CHECK_NO_THROW(n4 = ra4);
      BOOST_CHECK_EQUAL(n4, ipv4::network("192.168.3.0/24"));
    }
    {
      ipv4::network n4(24);

      BOOST_CHECK_NO_THROW(n4 = ra6_mapped);
      BOOST_CHECK_EQUAL(n4, ipv4::network("172.14.3.0/24"));
      BOOST_CHECK_NO_THROW(n4 = ra6_translated);
      BOOST_CHECK_EQUAL(n4, ipv4::network("172.14.5.0/24"));
    }
    {
      ipv4::network n4(24);

      BOOST_CHECK_THROW(n4 = ra6, cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(n4 = rn6, cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(n4 = rn6, cool::ng::exception::bad_conversion);
    }
  }
  {
    {
      ipv6::network n6;

      BOOST_CHECK_NO_THROW(n6 = rn6);
      BOOST_CHECK_EQUAL(n6, rn6);
    }
    {
      ipv6::network n6(112);

      BOOST_CHECK_NO_THROW(n6 = ra6);
      BOOST_CHECK_EQUAL(n6, ipv6::network("2605:2700:0:3::4713:0/112"));
    }
    {
      ipv6::network n6;

      BOOST_CHECK_THROW(n6 = ra4, cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(n6 = rn4, cool::ng::exception::bad_conversion);
    }
  }
}

COOL_AUTO_TEST_CASE(T002,
  *utf::description("operator =(const in6_addr&)"))
{
  const std::string s1 = "2605:2700:0:3::4713:93e3";
  const std::string s2 = "64:ff9b::172.14.3.1";
  const std::string s3 = "::ffff:172.14.5.99";

  const in6_addr a1 = static_cast<in6_addr>(ipv6::host(s1));
  const in6_addr a2 = static_cast<in6_addr>(ipv6::host(s2));
  const in6_addr a3 = static_cast<in6_addr>(ipv6::host(s3));

  {
    ipv4::host a(ipv4::loopback);

    BOOST_CHECK_THROW(a = a1, cool::ng::exception::bad_conversion);
    BOOST_CHECK_EQUAL(a, ipv4::loopback);
    BOOST_CHECK_NO_THROW(a = a2);
    BOOST_CHECK_EQUAL(a, ipv4::host("172.14.3.1"));
    BOOST_CHECK_NO_THROW(a = a3);
    BOOST_CHECK_EQUAL(a, ipv4::host("172.14.5.99"));
  }
  {
    ipv6::host a(ipv6::loopback);

    BOOST_CHECK_NO_THROW(a = a1);
    BOOST_CHECK_EQUAL(a, ipv6::host(s1));
  }
  {
    ipv4::network a(24);
    BOOST_CHECK_EQUAL(a.mask(), 24);
    BOOST_CHECK_THROW(a = a1, cool::ng::exception::bad_conversion);
  }
  {
    {
      ipv6::network a(96);
      BOOST_CHECK_EQUAL(a.mask(), 96);
      BOOST_CHECK_NO_THROW(a = a1);
      BOOST_CHECK_EQUAL(a, ipv6::network("2605:2700:0:3::0:0/96"));
    }
    {
      ipv6::network a(111);
      BOOST_CHECK_EQUAL(a.mask(), 111);
      BOOST_CHECK_NO_THROW(a = a1);
      BOOST_CHECK_EQUAL(a, ipv6::network("2605:2700:0:3::4712:0/111"));
    }
  }
}

COOL_AUTO_TEST_CASE(T003,
  *utf::description("operator =(const in_addr&)"))
{
  const std::string s1 = "172.14.3.1";

  const in_addr a1 = static_cast<in_addr>(ipv4::host(s1));

  {
    ipv4::host a(ipv4::loopback);

    BOOST_CHECK_NO_THROW(a = a1);
    BOOST_CHECK_EQUAL(a, ipv4::host("172.14.3.1"));
  }
  {
    {
      ipv4::network a(24);

      BOOST_CHECK_NO_THROW(a = a1);
      BOOST_CHECK_EQUAL(a, ipv4::network("172.14.3.0/24"));
    }
    {
      ipv4::network a(23);

      BOOST_CHECK_NO_THROW(a = a1);
      BOOST_CHECK_EQUAL(a, ipv4::network("172.14.2.0/23"));
    }
  }
  {
    ipv6::host a(ipv6::loopback);

    BOOST_CHECK_NO_THROW(a = a1);
    BOOST_CHECK_EQUAL(a, ipv6::host("::ffff:172.14.3.1"));
  }
  {
    ipv6::network a(96);
    BOOST_CHECK_THROW(a = a1, cool::ng::exception::bad_conversion);
  }
}

COOL_AUTO_TEST_CASE(T004,
  *utf::description("operator =(uint8_t const []"))
{
  const uint8_t a1[] = {
    0xab, 0x03, 0x00, 0x00,/**/ 0x00, 0x00, 0x12, 0x34,/**/
    0x56, 0x78, 0x00, 0x00,/**/  192,  168,    3,   99
  };
  const uint8_t *a2 = a1 + 12;

  {
    ipv4::host a;
    BOOST_CHECK_NO_THROW(a = a2);
    BOOST_CHECK_EQUAL(a, ipv4::host("192.168.3.99"));
  }
  {
    ipv4::network a(23);
    BOOST_CHECK_NO_THROW(a = a2);
    BOOST_CHECK_EQUAL(a, ipv4::network("192.168.2.0/23"));
  }
  {
    ipv6::host a;
    BOOST_CHECK_NO_THROW(a = a1);
    BOOST_CHECK_EQUAL(a, ipv6::host("ab03:0:0:1234:5678:0:192.168.3.99"));
  }
  {
    ipv6::network a(96);
    BOOST_CHECK_NO_THROW(a = a1);
    BOOST_CHECK_EQUAL(a, ipv6::network("ab03:0:0:1234:5678::/96"));
  }
}

COOL_AUTO_TEST_CASE(T005,
  *utf::description("operator =(const std::string&)"))
{
  {
    ipv4::host a(ipv4::loopback);
    BOOST_CHECK_NO_THROW(a = "192.168.3.99");
    BOOST_CHECK_EQUAL(a, ipv4::host({ 192, 168, 3, 99}));
  }
  {
    {
      ipv4::network a(ipv4::rfc_test);
      BOOST_CHECK_NO_THROW(a = "192.168.3.99/23");
      BOOST_CHECK_EQUAL(a.mask(), 23);
      BOOST_CHECK_EQUAL(a, ipv4::network(23, { 192, 168, 2, 0}));
    }
    {
      ipv4::network a(ipv4::rfc_test);
      BOOST_CHECK_THROW(a = "192.168.3.99", cool::ng::exception::bad_conversion);
    }
  }
  {
    {
      ipv6::host a(ipv6::loopback);
      BOOST_CHECK_NO_THROW(a = "2605:2700:0000:0003:0000:0000:4713:93e3"); // fully expanded
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x93, 0xe3}));
      BOOST_CHECK_NO_THROW(a = "2605:2700:0:3:0:0:4713:93e3"); // expanded
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x93, 0xe3}));
      BOOST_CHECK_NO_THROW(a = "2605:2700:0:3::4713:93e3"); // canonical
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x93, 0xe3}));
      BOOST_CHECK_NO_THROW(a = "2605-2700-0-3--4713-93e3"); // microsoft
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x93, 0xe3}));
      BOOST_CHECK_NO_THROW(a = "2605:2700:0:3::71.19.147.227"); // dotted-quad
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x93, 0xe3}));

      // more corner cases
      BOOST_CHECK_NO_THROW(a = "200:9:8:1223:3456:789a:bcde:f001");
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0x02, 0, 0, 0x09, 0, 0x08, 0x12, 0x23, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x01 }));
      BOOST_CHECK_NO_THROW(a = "200::1223:3456:789a:bcde:f001");
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0x02, 0, 0, 0, 0, 0, 0x12, 0x23, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x01 }));
      BOOST_CHECK_NO_THROW(a = "::1");
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01 }));
      BOOST_CHECK_NO_THROW(a = "--1");
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01 }));
      BOOST_CHECK_NO_THROW(a = "1::");
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }));
      BOOST_CHECK_NO_THROW(a = "1--");
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }));
      BOOST_CHECK_NO_THROW(a = "::");
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }));
      BOOST_CHECK_NO_THROW(a = "--");
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }));
      BOOST_CHECK_NO_THROW(a = "1::1");
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01 }));
      BOOST_CHECK_NO_THROW(a = "1--1");
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01 }));
      BOOST_CHECK_NO_THROW(a = "::fffe:192.168.3.99");
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xfe, 192, 168, 3, 99 }));
      BOOST_CHECK_NO_THROW(a = "::192.168.3.99");
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 192, 168, 3, 99 }));

    }
  }
  {
    {
      ipv6::network a(ipv6::rfc_mcast);
      BOOST_CHECK_NO_THROW(a = "2605:2700:0000:0003:0000:0000:4713:93e3/113"); // fully expanded
      BOOST_CHECK_EQUAL(a.mask(), 113);
      BOOST_CHECK_EQUAL(a, ipv6::network(113, { 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x80, 0}));
    }
    {
      ipv6::network a(ipv6::rfc_mcast);
      BOOST_CHECK_NO_THROW(a = "2605:2700:0:3:0:0:4713:93e3/113"); // expanded
      BOOST_CHECK_EQUAL(a.mask(), 113);
      BOOST_CHECK_EQUAL(a, ipv6::network(113, { 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x80, 0}));
    }
    {
      ipv6::network a(ipv6::rfc_mcast);
      BOOST_CHECK_NO_THROW(a = "2605:2700:0:3::4713:93e3/113"); // canonical
      BOOST_CHECK_EQUAL(a.mask(), 113);
      BOOST_CHECK_EQUAL(a, ipv6::network(113, { 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x80, 0}));
    }
    {
      ipv6::network a(ipv6::rfc_mcast);
      BOOST_CHECK_NO_THROW(a = "2605-2700-0-3--4713-93e3/113"); // microsoft
      BOOST_CHECK_EQUAL(a.mask(), 113);
      BOOST_CHECK_EQUAL(a, ipv6::network(113, { 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x80, 0}));
    }
    { // no mask size
      ipv6::network a(113);
      BOOST_CHECK_THROW(a = "2605:2700:0:3::4713:93e3", cool::ng::exception::bad_conversion);
    }
  }
    // ==============================================================
    // ====
    // ==== Erroneous input
    // ====
    // ==============================================================
  {
    // -- invalid characters
    {
      ipv4::host a(ipv4::loopback);
      BOOST_CHECK_THROW(a = "192", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.1682", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.1682x", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3.", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.300", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3]", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3.a9", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3:99", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3. 99", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "sometimes something goes wrong", cool::ng::exception::bad_conversion);
      BOOST_CHECK_EQUAL(a, ipv4::loopback);
    }
    {
      ipv4::network a(ipv4::rfc_test);
      BOOST_CHECK_THROW(a = "192", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.1682", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.1682x", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3.", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.300", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3]", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3.a9", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3:99", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3. 99", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3./", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3.0/", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "192.168.3.0/33", cool::ng::exception::out_of_range);
      BOOST_CHECK_THROW(a = "192.168.3.0/ 32", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "sometimes // something goes wrong", cool::ng::exception::bad_conversion);
      BOOST_CHECK_EQUAL(a, ipv4::rfc_test);
    }
    {
      ipv6::host a(ipv6::loopback);

      BOOST_CHECK_THROW(a = ":200", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200:", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200:0", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200:0", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200:0", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200:0:1223", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200:0:1223", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200:0:1223:3456", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200:0:1223:3456", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200:0:1223:3456:789a", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200:0:1223:3456:789a", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200:0:1223:3456:789a:bcde", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200:0:1223:3456:789a:bcde", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200:0:1223:3456:789a-bcde:2000", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "200g:0:1223:3456:789a:bcde:2000", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = ":2005:0:1223:3456:789a:bcde:2000:a0", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "sometimes something goes wrong", cool::ng::exception::bad_conversion);
      // deflation sequences
      BOOST_CHECK_THROW(a = "200:::3456:789a:bcde:2000", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = ":::3456:789a:bcde:2000", cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(a = "::3456:789a::bcde", cool::ng::exception::bad_conversion);

      // microsoft style and dot-decimal
      BOOST_CHECK_THROW(a = "--fffe-192.168.3.99", cool::ng::exception::bad_conversion);
      BOOST_CHECK_EQUAL(a, ipv6::loopback);
    }
    {
      ipv6::network a;

      BOOST_CHECK_THROW(a = "2605:2700:0:3::4713:93e3/129", cool::ng::exception::out_of_range);
    }
  }
}

COOL_AUTO_TEST_CASE(T006,
  *utf::description("ctor()"))
{
  {
    ipv4::host a;
    BOOST_CHECK_EQUAL(a, ipv4::unspecified);
  }
  {
    {
      ipv4::network a;
      BOOST_CHECK_EQUAL(a.mask(), 0);
      BOOST_CHECK_EQUAL(a, ipv4::unspecified_network);
    }
    {
      ipv4::network a;
      BOOST_CHECK_THROW(ipv4::network(33), cool::ng::exception::out_of_range);
      BOOST_CHECK_NO_THROW(a = ipv4::network(17));
      BOOST_CHECK_EQUAL(a.mask(), 17);
      BOOST_CHECK_EQUAL(memcmp(static_cast<const uint8_t*>(a), static_cast<const uint8_t*>(ipv4::unspecified), 4), 0);
    }
  }
  {
    ipv6::host a;
    BOOST_CHECK_EQUAL(a, ipv6::unspecified);
  }
  {
    {
      ipv6::network a;
      BOOST_CHECK_EQUAL(a.mask(), 0);
      BOOST_CHECK_EQUAL(a, ipv6::unspecified_network);
    }
    {
      ipv6::network a;
      BOOST_CHECK_THROW(ipv6::network(129), cool::ng::exception::out_of_range);
      BOOST_CHECK_NO_THROW(a = ipv6::network(42));
      BOOST_CHECK_EQUAL(a.mask(), 42);
      BOOST_CHECK_EQUAL(memcmp(static_cast<const uint8_t*>(a), static_cast<const uint8_t*>(ipv6::unspecified), 16), 0);
    }
  }
}

COOL_AUTO_TEST_CASE(T007,
  *utf::description("ctor(const address&)"))
{
  const ipv4::host ra4("192.168.3.99");
  const ipv6::host ra6("2605:2700:0:3::4713:93e3");
  const ipv6::host ra6_mapped("::ffff:172.14.3.1");
  const ipv6::host ra6_translated("64:ff9b::172.14.5.99");
  const ipv6::network rn6("2605:2700:0:3::4713:93e3:0/16");
  const ipv4::network rn4("192.168.0.0/16");

  {
    ipv4::host a;
    BOOST_CHECK_NO_THROW(a = ipv4::host(ra4));
    BOOST_CHECK_EQUAL(a, ra4);
    BOOST_CHECK_NO_THROW(a = ipv4::host(ra6_mapped));
    BOOST_CHECK_EQUAL(a, ipv4::host("172.14.3.1"));
    BOOST_CHECK_NO_THROW(a = ipv4::host(ra6_translated));
    BOOST_CHECK_EQUAL(a, ipv4::host("172.14.5.99"));

    BOOST_CHECK_THROW(a = ipv4::host(ra6), cool::ng::exception::bad_conversion);
  }
  {
    ipv6::host a;
    BOOST_CHECK_NO_THROW(a = ipv6::host(ra6));
    BOOST_CHECK_EQUAL(a, ra6);
    BOOST_CHECK_NO_THROW(a = ipv6::host(ra4));
    BOOST_CHECK_EQUAL(a, ipv6::host("::ffff:192.168.3.99"));
  }
  {
    ipv4::network n(8);
    BOOST_CHECK_NO_THROW(n = ipv4::network(16, ra4));
    BOOST_CHECK_EQUAL(n.mask(), 16);
    BOOST_CHECK_EQUAL(n, ipv4::network("192.168.0.0/16"));
    BOOST_CHECK_NO_THROW(n = ipv4::network(23, ra6_mapped));
    BOOST_CHECK_EQUAL(n.mask(), 23);
    BOOST_CHECK_EQUAL(n, ipv4::network("172.14.2.0/23"));
    BOOST_CHECK_NO_THROW(n = ipv4::network(8, ra6_translated));
    BOOST_CHECK_EQUAL(n.mask(), 8);
    BOOST_CHECK_EQUAL(n, ipv4::network("172.0.0.0/8"));
    BOOST_CHECK_THROW(ipv4::network(33, ra6_translated), cool::ng::exception::out_of_range);
    BOOST_CHECK_THROW(ipv4::network(24, ra6), cool::ng::exception::bad_conversion);
    auto aux = "192.4.4.0/24"_ipv4_net;
    address& a = aux;
    BOOST_CHECK_NO_THROW(n = ipv4::network(16, a));
    BOOST_CHECK_EQUAL(n, "192.4.0.0/16"_ipv4_net);
    BOOST_CHECK_THROW(ipv4::network(24, "1::/8"_ipv6_net), cool::ng::exception::bad_conversion);
  }
  {
    ipv6::network n(8);
    BOOST_CHECK_NO_THROW(n = ipv6::network(88, ra6));
    BOOST_CHECK_EQUAL(n.mask(), 88);
    BOOST_CHECK_EQUAL(n, ipv6::network("2605:2700:0:3::/88"));
    BOOST_CHECK_THROW(ipv6::network(129, ra6_translated), cool::ng::exception::out_of_range);
  }
}

COOL_AUTO_TEST_CASE(T008,
  * utf::description("ctor(const uint8[])"))
{
  const uint8_t a1[] = {
    0xab, 0x03, 0x00, 0x00,/**/ 0x00, 0x00, 0x12, 0x34,/**/
    0x56, 0x78, 0x00, 0x00,/**/  192,  168,    3,   99
  };
  const uint8_t *a2 = a1 + 12;

  {
    ipv4::host a;
    BOOST_CHECK_NO_THROW(a = ipv4::host(a2));
    BOOST_CHECK_EQUAL(a, ipv4::host("192.168.3.99"));
  }
  {
    ipv4::network a(23);
    BOOST_CHECK_NO_THROW(a = ipv4::network(23, a2));
    BOOST_CHECK_EQUAL(a, ipv4::network("192.168.2.0/23"));
    BOOST_CHECK_THROW(a = ipv4::network(33, a2), cool::ng::exception::out_of_range);
  }
  {
    ipv6::host a;
    BOOST_CHECK_NO_THROW(a = ipv6::host(a1));
    BOOST_CHECK_EQUAL(a, ipv6::host("ab03:0:0:1234:5678:0:192.168.3.99"));
  }
  {
    ipv6::network a(96);
    BOOST_CHECK_NO_THROW(a = ipv6::network(96, a1));
    BOOST_CHECK_EQUAL(a, ipv6::network("ab03:0:0:1234:5678::/96"));
    BOOST_CHECK_THROW(a = ipv6::network(129, a1), cool::ng::exception::out_of_range);
  }
}

COOL_AUTO_TEST_CASE(T009,
  *utf::description("ctor(const in6_addr&)"))
{
  const std::string s1 = "2605:2700:0:3::4713:93e3";
  const std::string s2 = "64:ff9b::172.14.3.1";
  const std::string s3 = "::ffff:172.14.5.99";

  const in6_addr a1 = static_cast<in6_addr>(ipv6::host(s1));
  const in6_addr a2 = static_cast<in6_addr>(ipv6::host(s2));
  const in6_addr a3 = static_cast<in6_addr>(ipv6::host(s3));

  {
    ipv4::host a(ipv4::loopback);

    BOOST_CHECK_THROW(a = ipv4::host(a1), cool::ng::exception::bad_conversion);
    BOOST_CHECK_NO_THROW(a = ipv4::host(a2));
    BOOST_CHECK_EQUAL(a, ipv4::host("172.14.3.1"));
    BOOST_CHECK_NO_THROW(a = ipv4::host(a3));
    BOOST_CHECK_EQUAL(a, ipv4::host("172.14.5.99"));
  }
  {
    ipv6::host a(ipv6::loopback);

    BOOST_CHECK_NO_THROW(a = ipv6::host(a1));
    BOOST_CHECK_EQUAL(a, ipv6::host(s1));
  }
  {
    {
      ipv6::network a;

      BOOST_CHECK_NO_THROW(a = ipv6::network(96, a1));
      BOOST_CHECK_EQUAL(a, ipv6::network("2605:2700:0:3::0:0/96"));

      BOOST_CHECK_NO_THROW(a = ipv6::network(111, a1));
      BOOST_CHECK_EQUAL(a, ipv6::network("2605:2700:0:3::4712:0/111"));

      BOOST_CHECK_THROW(a = ipv6::network(129, a1), cool::ng::exception::out_of_range);
    }
  }
}

COOL_AUTO_TEST_CASE(T010,
  *utf::description("ctor(const in_addr&)"))
{
  const std::string s1 = "172.14.3.1";

  const in_addr a1 = static_cast<in_addr>(ipv4::host(s1));

  {
    ipv4::host a(ipv4::loopback);

    BOOST_CHECK_NO_THROW(a = ipv4::host(a1));
    BOOST_CHECK_EQUAL(a, ipv4::host("172.14.3.1"));
  }
  {
    ipv4::network a(24);

    BOOST_CHECK_NO_THROW(a = ipv4::network(23, a1));
    BOOST_CHECK_EQUAL(a, ipv4::network("172.14.2.0/23"));
    BOOST_CHECK_THROW(a = ipv4::network(33, a1), cool::ng::exception::out_of_range);
  }
  {
    ipv6::host a(ipv6::loopback);

    BOOST_CHECK_NO_THROW(a = ipv6::host(a1));
    BOOST_CHECK_EQUAL(a, ipv6::host("::ffff:172.14.3.1"));
  }
}

COOL_AUTO_TEST_CASE(T011,
  *utf::description("ctor(const std::string&)"))
{
  {
    ipv4::host a(ipv4::loopback);
    BOOST_CHECK_NO_THROW(a = ipv4::host("192.168.3.99"));
    BOOST_CHECK_EQUAL(a, ipv4::host({ 192, 168, 3, 99}));
    BOOST_CHECK_THROW(a = ipv4::host("2605:2700:0000:0003:0000:0000:4713:93e3"), cool::ng::exception::bad_conversion);
  }
  {
    {
      ipv4::network a(ipv4::rfc_test);
      BOOST_CHECK_NO_THROW(a = ipv4::network("192.168.3.99/23"));
      BOOST_CHECK_EQUAL(a.mask(), 23);
      BOOST_CHECK_EQUAL(a, ipv4::network(23, { 192, 168, 2, 0}));
      BOOST_CHECK_THROW(a = ipv4::network("192.168.3.99/33"), cool::ng::exception::out_of_range);
      BOOST_CHECK_THROW(a = ipv4::network("192.168.3.99"), cool::ng::exception::bad_conversion);
    }
  }
  {
    {
      ipv6::host a(ipv6::loopback);
      BOOST_CHECK_NO_THROW(a = "2605:2700:0000:0003:0000:0000:4713:93e3"); // fully expanded
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x93, 0xe3}));
      BOOST_CHECK_NO_THROW(a = "2605:2700:0:3:0:0:4713:93e3"); // expanded
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x93, 0xe3}));
      BOOST_CHECK_NO_THROW(a = "2605:2700:0:3::4713:93e3"); // canonical
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x93, 0xe3}));
      BOOST_CHECK_NO_THROW(a = "2605-2700-0-3--4713-93e3"); // microsoft
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x93, 0xe3}));
      BOOST_CHECK_NO_THROW(a = "2605:2700:0:3::71.19.147.227"); // dotted-quad
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x93, 0xe3}));

      // more corner cases
      BOOST_CHECK_NO_THROW(a = ipv6::host("200:9:8:1223:3456:789a:bcde:f001"));
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0x02, 0, 0, 0x09, 0, 0x08, 0x12, 0x23, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x01 }));
      BOOST_CHECK_NO_THROW(a = ipv6::host("200::1223:3456:789a:bcde:f001"));
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0x02, 0, 0, 0, 0, 0, 0x12, 0x23, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x01 }));
      BOOST_CHECK_NO_THROW(a = ipv6::host("::1"));
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01 }));
      BOOST_CHECK_NO_THROW(a = ipv6::host("--1"));
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01 }));
      BOOST_CHECK_NO_THROW(a = ipv6::host("1::"));
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }));
      BOOST_CHECK_NO_THROW(a = ipv6::host("1--"));
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }));
      BOOST_CHECK_NO_THROW(a = ipv6::host("::"));
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }));
      BOOST_CHECK_NO_THROW(a = ipv6::host("--"));
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }));
      BOOST_CHECK_NO_THROW(a = ipv6::host("1::1"));
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01 }));
      BOOST_CHECK_NO_THROW(a = ipv6::host("1--1"));
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01 }));
      BOOST_CHECK_NO_THROW(a = ipv6::host("::fffe:192.168.3.99"));
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xfe, 192, 168, 3, 99 }));
      BOOST_CHECK_NO_THROW(a = ipv6::host("::192.168.3.99"));
      BOOST_CHECK_EQUAL(a, ipv6::host({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 192, 168, 3, 99 }));
    }
  }
  {
    {
      ipv6::network a(ipv6::rfc_mcast);
      BOOST_CHECK_NO_THROW(a = ipv6::network("2605:2700:0000:0003:0000:0000:4713:93e3/113")); // fully expanded
      BOOST_CHECK_EQUAL(a.mask(), 113);
      BOOST_CHECK_EQUAL(a, ipv6::network(113, { 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x80, 0}));
      BOOST_CHECK_NO_THROW(a = ipv6::network("2605:2700:0:3:0:0:4713:93e3/113")); // expanded
      BOOST_CHECK_EQUAL(a.mask(), 113);
      BOOST_CHECK_EQUAL(a, ipv6::network(113, { 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x80, 0}));
      BOOST_CHECK_NO_THROW(a = ipv6::network("2605:2700:0:3::4713:93e3/113")); // canonical
      BOOST_CHECK_EQUAL(a.mask(), 113);
      BOOST_CHECK_EQUAL(a, ipv6::network(113, { 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x80, 0}));
      BOOST_CHECK_NO_THROW(a = ipv6::network("2605-2700-0-3--4713-93e3/113")); // microsoft
      BOOST_CHECK_EQUAL(a.mask(), 113);
      BOOST_CHECK_EQUAL(a, ipv6::network(113, { 0x26, 0x05, 0x27, 0, 0, 0, 0, 0x03, 0, 0, 0, 0, 0x47, 0x13, 0x80, 0}));

      BOOST_CHECK_THROW(a = ipv6::network("2605:2700:0:3::4713:93e3/129"), cool::ng::exception::out_of_range);
      BOOST_CHECK_THROW(a = ipv6::network("2605:2700:0:3::4713:93e3"), cool::ng::exception::bad_conversion);
    }
  }
}

COOL_AUTO_TEST_CASE(T012,
  *utf::description("ctor(std::initializer_list<uint8_t>)"))
{
  {
    ipv4::host a(ipv4::loopback);
    BOOST_CHECK_NO_THROW(a = ipv4::host({ 192, 168, 3, 99 }));
    BOOST_CHECK_EQUAL(a, ipv4::host("192.168.3.99"));
  }
  {
    ipv4::network a;
    BOOST_CHECK_NO_THROW(a = ipv4::network(23, { 192, 168, 3, 99 }));
    BOOST_CHECK_EQUAL(a, ipv4::network("192.168.2.0/23"));
    BOOST_CHECK_THROW(a = ipv4::network(33, { 192, 168, 3, 99 }), cool::ng::exception::out_of_range);
  }
  {
    ipv6::host a;
    BOOST_CHECK_NO_THROW(a = ipv6::host({ 0x02, 0x00, 0, 0, 0, 0, 0x12, 0x23, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x01 }));
    BOOST_CHECK_EQUAL(a, ipv6::host("200::1223:3456:789a:bcde:f001"));
  }
  {
    ipv6::network a;
    BOOST_CHECK_NO_THROW(a = ipv6::network(97, { 0x02, 0x00, 0, 0, 0, 0, 0x12, 0x23, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x01 }));
    BOOST_CHECK_EQUAL(a, ipv6::network("200::1223:3456:789a:8000:0/97"));
    BOOST_CHECK_THROW(a = ipv6::network(129, { 0x02, 0x00, 0, 0, 0, 0, 0x12, 0x23, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x01 }), cool::ng::exception::out_of_range);
  }
}

COOL_AUTO_TEST_CASE(T014,
  *utf::description("operator const uint8_t *() const"))
{
  {
    ipv4::host a("192.168.3.99");
    uint8_t arr[] = { 0xc0, 0xa8, 0x03, 0x63 };
    BOOST_CHECK(0 == memcmp(arr, static_cast<const uint8_t*>(a), a.size()));
  }
  {
    ipv4::network a("192.168.3.99/26");
    uint8_t arr[] = { 0xc0, 0xa8, 0x03, 0x40 };
    BOOST_CHECK(0 == memcmp(arr, static_cast<const uint8_t*>(a), a.size()));
  }
  {
    ipv6::host a("200::1223:3456:789a:bcde:f001");
    uint8_t arr[] = { 0x02, 0x00, 0, 0, 0, 0, 0x12, 0x23, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x01 };
    BOOST_CHECK(0 == memcmp(arr, static_cast<const uint8_t*>(a), a.size()));
  }
  {
    ipv6::network a("200::1223:3456:789a:bcde:f001/93");
    uint8_t arr[] = { 0x02, 0x00, 0, 0, 0, 0, 0x12, 0x23, 0x34, 0x56, 0x78, 0x98, 0, 0, 0, 0 };
    BOOST_CHECK(0 == memcmp(arr, static_cast<const uint8_t*>(a), a.size()));
  }
}

COOL_AUTO_TEST_CASE(T015,
  *utf::description("operator struct in_addr() const"))
{
  struct in_addr ip4;

  {
    ipv4::host a("192.168.3.99");
    uint8_t ref[] = { 0xc0, 0xa8, 0x03, 0x63 };
    BOOST_CHECK_NO_THROW(ip4 = static_cast<struct in_addr>(a));
    BOOST_CHECK(0 == memcmp(&ref, &ip4.s_addr, sizeof(ref)));
  }
  {
    ipv4::network a("192.168.3.99/26");
    uint8_t ref[] = { 0xc0, 0xa8, 0x03, 0x40 };
    BOOST_CHECK_NO_THROW(ip4 = static_cast<struct in_addr>(a));
    BOOST_CHECK(0 == memcmp(&ref, &ip4.s_addr, sizeof(ref)));
  }
  {
    {
      ipv6::host a("200::1223:3456:789a:bcde:f001");
      BOOST_CHECK_THROW(ip4 = static_cast<struct in_addr>(a), cool::ng::exception::bad_conversion);
    }
    {
      ipv6::host a("::ffff:172.4.12.75");
      uint8_t ref[] = { 172, 4, 12, 75 };
      BOOST_CHECK_NO_THROW(ip4 = static_cast<struct in_addr>(a));
      BOOST_CHECK(0 == memcmp(&ref, &ip4.s_addr, sizeof(ref)));
    }
    {
      ipv6::host a("64:ff9b::172.4.12.75");
      uint8_t ref[] = { 172, 4, 12, 75 };
      BOOST_CHECK_NO_THROW(ip4 = static_cast<struct in_addr>(a));
      BOOST_CHECK(0 == memcmp(&ref, &ip4.s_addr, sizeof(ref)));
    }
  }
  {
    ipv6::network a("200::1223:3456:789a:bcde:f001/93");
    BOOST_CHECK_THROW(ip4 = static_cast<struct in_addr>(a), cool::ng::exception::bad_conversion);
  }
}

COOL_AUTO_TEST_CASE(T016,
  *utf::description("operator struct in6_addr() const"))
{
  struct in6_addr ip6;

  {
    ipv4::host a("192.168.3.99");
    uint8_t ref[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 192, 168, 3, 99 };
    BOOST_CHECK_NO_THROW(ip6 = static_cast<struct in6_addr>(a));
    BOOST_CHECK(0 == memcmp(&ref, ip6.s6_addr, sizeof(ref)));
  }
  {
    ipv4::network a("192.168.3.99/26");
    BOOST_CHECK_THROW(ip6 = static_cast<struct in6_addr>(a), cool::ng::exception::bad_conversion);
  }
  {
    ipv6::host a("200::1223:3456:789a:bcde:f001");
    uint8_t ref[] = { 0x02, 0, 0, 0, 0, 0, 0x12, 0x23, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x01 };
    BOOST_CHECK_NO_THROW(ip6 = static_cast<struct in6_addr>(a));
    BOOST_CHECK(0 == memcmp(&ref, ip6.s6_addr, sizeof(ref)));
  }
  {
    ipv6::network a("200::1223:3456:789a:bcde:f001/93");
    uint8_t ref[] = { 0x02, 0, 0, 0, 0, 0, 0x12, 0x23, 0x34, 0x56, 0x78, 0x98, 0, 0, 0, 0 };
    BOOST_CHECK_NO_THROW(ip6 = static_cast<struct in6_addr>(a));
    BOOST_CHECK(0 == memcmp(&ref, ip6.s6_addr, sizeof(ref)));
  }
}

COOL_AUTO_TEST_CASE(T017,
  *utf::description("operator std::string() const"))
{
  {
    ipv4::host a("192.168.3.99");
    BOOST_CHECK_EQUAL(static_cast<std::string>(a), "192.168.3.99");
  }
  {
    ipv4::network a("192.168.3.99/26");
    BOOST_CHECK_EQUAL(static_cast<std::string>(a), "192.168.3.64/26");
  }
  {
    {
      ipv6::host a("200::1223:3456:789a:bcde:f001");
      BOOST_CHECK_EQUAL(static_cast<std::string>(a), "200::1223:3456:789a:bcde:f001");
    }
    {
      ipv6::host a("::ffff:172.4.12.75");
      BOOST_CHECK_EQUAL(static_cast<std::string>(a), "::ffff:172.4.12.75");
    }
    {
      ipv6::host a("64:ff9b::172.4.12.75");
      BOOST_CHECK_EQUAL(static_cast<std::string>(a), "64:ff9b::172.4.12.75");
    }
  }
  {
    ipv6::network a("200::1223:3456:789a:bcde:f001/93");
    BOOST_CHECK_EQUAL(static_cast<std::string>(a), "200::1223:3456:7898:0:0/93");
  }
}

COOL_AUTO_TEST_CASE(T018,
  *utf::description("address::visualize()"))
{
  {
    auto a = "172.17.5.42"_ipv4;

    BOOST_CHECK_EQUAL("172.17.5.42", visualize(a));
    BOOST_CHECK_EQUAL("172.17.5.42", visualize(a, style::customary));
    BOOST_CHECK_EQUAL("172.17.5.42", visualize(a, style::dot_decimal));
    BOOST_CHECK_THROW(visualize(a, style::canonical), cool::ng::exception::bad_conversion);
    BOOST_CHECK_THROW(visualize(a, style::strict), cool::ng::exception::bad_conversion);
    BOOST_CHECK_THROW(visualize(a, style::expanded), cool::ng::exception::bad_conversion);
    BOOST_CHECK_THROW(visualize(a, style::microsoft), cool::ng::exception::bad_conversion);
    BOOST_CHECK_THROW(visualize(a, style::dotted_quad), cool::ng::exception::bad_conversion);
  }
  {
    ipv4::network a("172.17.5.42/23");

    BOOST_CHECK_EQUAL("172.17.4.0/23", visualize(a));
    BOOST_CHECK_EQUAL("172.17.4.0/23", visualize(a, style::customary));
    BOOST_CHECK_EQUAL("172.17.4.0/23", visualize(a, style::dot_decimal));
    BOOST_CHECK_THROW(visualize(a, style::canonical), cool::ng::exception::bad_conversion);
    BOOST_CHECK_THROW(visualize(a, style::strict), cool::ng::exception::bad_conversion);
    BOOST_CHECK_THROW(visualize(a, style::expanded), cool::ng::exception::bad_conversion);
    BOOST_CHECK_THROW(visualize(a, style::microsoft), cool::ng::exception::bad_conversion);
    BOOST_CHECK_THROW(visualize(a, style::dotted_quad), cool::ng::exception::bad_conversion);
  }
  {
    ipv6::host addr;

    addr = ip6_host_examples[0];
    BOOST_CHECK_EQUAL("200::1223:3456:789a:bcde:f001", str(addr, style::canonical));
    BOOST_CHECK_EQUAL("200::1223:3456:789a:bcde:f001", str(addr, style::strict));
    BOOST_CHECK_EQUAL("200:0:0:1223:3456:789a:bcde:f001", str(addr, style::expanded));
    BOOST_CHECK_EQUAL("200--1223-3456-789a-bcde-f001", str(addr, style::microsoft));
    BOOST_CHECK_EQUAL("200::1223:3456:789a:188.222.240.1", str(addr, style::dotted_quad));
    BOOST_CHECK_THROW(str(addr, style::dot_decimal), cool::ng::exception::bad_conversion);

    addr = ip6_host_examples[1];
    BOOST_CHECK_EQUAL("203::1223:3400:0:0:f001", str(addr, style::canonical));
    BOOST_CHECK_EQUAL("203::1223:3400:0:0:f001", str(addr, style::strict));
    BOOST_CHECK_EQUAL("203:0:0:1223:3400:0:0:f001", str(addr, style::expanded));
    BOOST_CHECK_EQUAL("203--1223-3400-0-0-f001", str(addr, style::microsoft));
    BOOST_CHECK_EQUAL("203::1223:3400:0:0.0.240.1", str(addr, style::dotted_quad));

    addr = ip6_host_examples[2];
    BOOST_CHECK_EQUAL("ab03:0:0:1234:5678::", str(addr, style::canonical));
    BOOST_CHECK_EQUAL("ab03:0:0:1234:5678::", str(addr, style::strict));
    BOOST_CHECK_EQUAL("ab03:0:0:1234:5678:0:0:0", str(addr, style::expanded));
    BOOST_CHECK_EQUAL("ab03-0-0-1234-5678--", str(addr, style::microsoft));
    BOOST_CHECK_EQUAL("ab03::1234:5678:0:0.0.0.0", str(addr, style::dotted_quad));

    addr = ip6_host_examples[3];
    BOOST_CHECK_EQUAL("ab03::1234:5678:1:0:0", str(addr, style::canonical));
    BOOST_CHECK_EQUAL("ab03::1234:5678:1:0:0", str(addr, style::strict));
    BOOST_CHECK_EQUAL("ab03:0:0:1234:5678:1:0:0", str(addr, style::expanded));
    BOOST_CHECK_EQUAL("ab03--1234-5678-1-0-0", str(addr, style::microsoft));
    BOOST_CHECK_EQUAL("ab03::1234:5678:1:0.0.0.0", str(addr, style::dotted_quad));

    addr = ip6_host_examples[4];
    BOOST_CHECK_EQUAL("::ffff:0:a0b:c0d", str(addr, style::canonical));
    BOOST_CHECK_EQUAL("::ffff:0:a0b:c0d", str(addr, style::strict));
    BOOST_CHECK_EQUAL("0:0:0:0:ffff:0:a0b:c0d", str(addr, style::expanded));
    BOOST_CHECK_EQUAL("--ffff-0-a0b-c0d", str(addr, style::microsoft));
    BOOST_CHECK_EQUAL("::ffff:0:10.11.12.13", str(addr, style::dotted_quad));

    addr = ip6_host_examples[5];
    BOOST_CHECK_EQUAL("::ffff:10.11.12.13", str(addr, style::canonical));
    BOOST_CHECK_EQUAL("::ffff:a0b:c0d", str(addr, style::strict));
    BOOST_CHECK_EQUAL("0:0:0:0:0:ffff:a0b:c0d", str(addr, style::expanded));
    BOOST_CHECK_EQUAL("--ffff-a0b-c0d", str(addr, style::microsoft));
    BOOST_CHECK_EQUAL("::ffff:10.11.12.13", str(addr, style::dotted_quad));

    addr = ip6_host_examples[6];
    BOOST_CHECK_EQUAL("64:ff9b::172.4.12.75", str(addr, style::canonical));
    BOOST_CHECK_EQUAL("64:ff9b::ac04:c4b", str(addr, style::strict));
    BOOST_CHECK_EQUAL("64:ff9b:0:0:0:0:ac04:c4b", str(addr, style::expanded));
    BOOST_CHECK_EQUAL("64-ff9b--ac04-c4b", str(addr, style::microsoft));
    BOOST_CHECK_EQUAL("64:ff9b::172.4.12.75", str(addr, style::dotted_quad));
  }
  {
    {
      ipv6::network addr(96, ip6_host_examples[0]);
      BOOST_CHECK_EQUAL("200::1223:3456:789a:0:0/96", str(addr, style::canonical));
      BOOST_CHECK_EQUAL("200::1223:3456:789a:0:0/96", str(addr, style::strict));
      BOOST_CHECK_EQUAL("200:0:0:1223:3456:789a:0:0/96", str(addr, style::expanded));
      BOOST_CHECK_EQUAL("200--1223-3456-789a-0-0/96", str(addr, style::microsoft));
      BOOST_CHECK_THROW(str(addr, style::dotted_quad), cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(str(addr, style::dot_decimal), cool::ng::exception::bad_conversion);
    }
    {
      ipv6::network addr(96, ip6_host_examples[1]);
      BOOST_CHECK_EQUAL("203:0:0:1223:3400::/96", str(addr, style::canonical));
      BOOST_CHECK_EQUAL("203:0:0:1223:3400::/96", str(addr, style::customary));
      BOOST_CHECK_EQUAL("203:0:0:1223:3400::/96", str(addr, style::strict));
      BOOST_CHECK_EQUAL("203:0:0:1223:3400:0:0:0/96", str(addr, style::expanded));
      BOOST_CHECK_EQUAL("203-0-0-1223-3400--/96", str(addr, style::microsoft));
    }
    {
      ipv6::network addr(96, ip6_host_examples[2]);
      BOOST_CHECK_EQUAL("ab03:0:0:1234:5678::/96", str(addr, style::canonical));
      BOOST_CHECK_EQUAL("ab03:0:0:1234:5678::/96", str(addr, style::strict));
      BOOST_CHECK_EQUAL("ab03:0:0:1234:5678:0:0:0/96", str(addr, style::expanded));
      BOOST_CHECK_EQUAL("ab03-0-0-1234-5678--/96", str(addr, style::microsoft));
    }
    {
      ipv6::network addr(96, ip6_host_examples[3]);
      BOOST_CHECK_EQUAL("ab03::1234:5678:1:0:0/96", str(addr, style::canonical));
      BOOST_CHECK_EQUAL("ab03::1234:5678:1:0:0/96", str(addr, style::strict));
      BOOST_CHECK_EQUAL("ab03:0:0:1234:5678:1:0:0/96", str(addr, style::expanded));
      BOOST_CHECK_EQUAL("ab03--1234-5678-1-0-0/96", str(addr, style::microsoft));
    }
    {
      ipv6::network addr(96, ip6_host_examples[4]);
      BOOST_CHECK_EQUAL("::ffff:0:0:0/96", str(addr, style::canonical));
      BOOST_CHECK_EQUAL("::ffff:0:0:0/96", str(addr, style::strict));
      BOOST_CHECK_EQUAL("0:0:0:0:ffff:0:0:0/96", str(addr, style::expanded));
      BOOST_CHECK_EQUAL("--ffff-0-0-0/96", str(addr, style::microsoft));
    }
    {
      ipv6::network addr(96, ip6_host_examples[5]);
      BOOST_CHECK_EQUAL("::ffff:0:0/96", str(addr, style::canonical));
      BOOST_CHECK_EQUAL("::ffff:0:0/96", str(addr, style::customary));
      BOOST_CHECK_EQUAL("::ffff:0:0/96", str(addr, style::strict));
      BOOST_CHECK_EQUAL("0:0:0:0:0:ffff:0:0/96", str(addr, style::expanded));
      BOOST_CHECK_EQUAL("--ffff-0-0/96", str(addr, style::microsoft));
    }
    {
      ipv6::network addr(96, ip6_host_examples[6]);
      BOOST_CHECK_EQUAL("64:ff9b::/96", str(addr, style::canonical));
      BOOST_CHECK_EQUAL("64:ff9b::/96", str(addr, style::strict));
      BOOST_CHECK_EQUAL("64:ff9b:0:0:0:0:0:0/96", str(addr, style::expanded));
      BOOST_CHECK_EQUAL("64-ff9b--/96", str(addr, style::microsoft));
    }
  }
}

COOL_AUTO_TEST_CASE(T019,
  *utf::description("address::equals()"))
{
  // IPv4 Host
  {
    BOOST_CHECK("10.35.12.161"_ipv4.equals("10.35.12.161"_ipv4));
    BOOST_CHECK("10.35.12.161"_ipv4.equals("64:ff9b::10.35.12.161"_ipv6));  // equal - translated
    BOOST_CHECK("10.35.12.161"_ipv4.equals("::ffff:10.35.12.161"_ipv6));    // equal - mapped

    BOOST_CHECK(!"10.35.12.161"_ipv4.equals("192.168.206.40"_ipv4));
    BOOST_CHECK(!"10.35.12.161"_ipv4.equals("10.35.12.161/8"_ipv4_net));
    BOOST_CHECK(!"10.35.12.161"_ipv4.equals(ipv6::rfc_ipv4map));
  }
  // IPv4 network
  {
    BOOST_CHECK("192.168.206.40/24"_ipv4_net.equals("192.168.206.40/24"_ipv4_net));
    BOOST_CHECK("192.168.206.40/24"_ipv4_net.equals("192.168.206.0/24"_ipv4_net));    // equal - netmask applied
    BOOST_CHECK("192.168.206.40/24"_ipv4_net.equals("192.168.206.43/24"_ipv4_net));   // equal - netmask applied

    BOOST_CHECK(!"192.168.206.40/24"_ipv4_net.equals("192.168.206.40/24"_ipv4));
    BOOST_CHECK(!"192.168.206.40/24"_ipv4_net.equals("192.168.206.40/16"_ipv4_net));
    BOOST_CHECK(!"192.168.206.40/24"_ipv4_net.equals("192.168.205.40/24"_ipv4_net));
    BOOST_CHECK(!"192.168.206.40/24"_ipv4_net.equals("::ffff:192.168.206.40"_ipv6));
    BOOST_CHECK(!"192.168.206.40/24"_ipv4_net.equals("::ffff:192.168.206.40/96"_ipv6_net));

    // corner case - network mask not at the byte boundary
    BOOST_CHECK("192.168.205.40/23"_ipv4_net.equals("192.168.204.0/23"_ipv4_net));
  }
  // IPv6 Host
  {
    BOOST_CHECK("64:ff9b::10.35.12.161"_ipv6.equals( "64:ff9b::10.35.12.161"_ipv6));
    BOOST_CHECK("::ffff:10.35.12.161"_ipv6.equals(   "10.35.12.161"_ipv4));   // equal - mappd
    BOOST_CHECK("64:ff9b::10.35.12.161"_ipv6.equals( "10.35.12.161"_ipv4));   // equal - translated
    BOOST_CHECK(!"64:ff9b::10.35.12.162"_ipv6.equals("64:ff9b::10.35.12.161"_ipv6));
    BOOST_CHECK(!"::ffff:10.35.12.162"_ipv6.equals(  "10.35.12.161"_ipv4));
    BOOST_CHECK(!"64:ff9b::10.35.12.162"_ipv6.equals("10.35.12.161"_ipv4));
    BOOST_CHECK(!"63:ff9b::10.35.12.161"_ipv6.equals("10.35.12.161"_ipv4));
    BOOST_CHECK(!"64:ff9b::10.35.12.161"_ipv6.equals("64:ff9b::10.35.12.162"_ipv6));
    BOOST_CHECK(!"64:ff9b::10.35.12.161"_ipv6.equals("64:ff9b::10.35.12.161/112"_ipv6_net));
    BOOST_CHECK(!"64:ff9b::10.35.12.161"_ipv6.equals("192.168.206.40/24"_ipv4_net));
  }
  // IPv6 network
  {
    BOOST_CHECK("200::1223:3456:789a:abcd:ef01/94"_ipv6_net.equals("200::1223:3456:789a:abcd:ef01/94"_ipv6_net));
    BOOST_CHECK("200::1223:3456:789a:abcd:ef01/94"_ipv6_net.equals("200::1223:3456:7898:0:0/94"_ipv6_net));   // netmask applied
    BOOST_CHECK("200:1:2:1223:3456:789a:abcd:ef01/96"_ipv6_net.equals("200:1:2:1223:3456:789a::/96"_ipv6_net)); // netmasl applied

    BOOST_CHECK(!"200:1:2:1223:3456:789a:abcd:ef01/94"_ipv6_net.equals("200::1223:3456:789a:abcd:ef01/94"_ipv6_net));
    BOOST_CHECK(!"200:1:2:1223:3456:789a:abcd:ef01/96"_ipv6_net.equals("200::1223:3456:789a:abcd:ef01/96"_ipv6_net));
    BOOST_CHECK(!"200:1:2:1223:3456:789a:abcd:ef01/96"_ipv6_net.equals("200::1223:3456:789a:abcd:ef01/96"_ipv6));
    BOOST_CHECK(!"200:1:2:1223:3456:789a:abcd:ef01/96"_ipv6_net.equals("192.168.206.0/24"_ipv4_net));
    BOOST_CHECK(!"200:1:2:1223:3456:789a:abcd:ef01/96"_ipv6_net.equals("192.168.206.40/24"_ipv4));
  }
}

COOL_AUTO_TEST_CASE(T020,
  *utf::description("global operators == and !="))
{
  // IPv4 Host
  {
    BOOST_CHECK("10.35.12.161"_ipv4 == "10.35.12.161"_ipv4);
    BOOST_CHECK("10.35.12.161"_ipv4 == "64:ff9b::10.35.12.161"_ipv6);  // equal - translated
    BOOST_CHECK("10.35.12.161"_ipv4 == "::ffff:10.35.12.161"_ipv6);    // equal - mapped

    BOOST_CHECK("10.35.12.161"_ipv4 != "192.168.206.40"_ipv4);
    BOOST_CHECK("10.35.12.161"_ipv4 != "10.35.12.161/8"_ipv4_net);
    BOOST_CHECK("10.35.12.161"_ipv4 != ipv6::rfc_ipv4map);
  }
  // IPv4 network
  {
    BOOST_CHECK("192.168.206.40/24"_ipv4_net == "192.168.206.40/24"_ipv4_net);
    BOOST_CHECK("192.168.206.40/24"_ipv4_net == "192.168.206.0/24"_ipv4_net);    // equal - netmask applied
    BOOST_CHECK("192.168.206.40/24"_ipv4_net == "192.168.206.43/24"_ipv4_net);   // equal - netmask applied

    BOOST_CHECK("192.168.206.40/24"_ipv4_net != "192.168.206.40/24"_ipv4);
    BOOST_CHECK("192.168.206.40/24"_ipv4_net != "192.168.206.40/16"_ipv4_net);
    BOOST_CHECK("192.168.206.40/24"_ipv4_net != "192.168.205.40/24"_ipv4_net);
    BOOST_CHECK("192.168.206.40/24"_ipv4_net != "::ffff:192.168.206.40"_ipv6);
    BOOST_CHECK("192.168.206.40/24"_ipv4_net != "::ffff:192.168.206.40/96"_ipv6_net);

    // corner case - network mask not at the byte boundary
    BOOST_CHECK("192.168.205.40/23"_ipv4_net == "192.168.204.0/23"_ipv4_net);
  }
  // IPv6 Host
  {
    BOOST_CHECK("64:ff9b::10.35.12.161"_ipv6 == "64:ff9b::10.35.12.161"_ipv6);
    BOOST_CHECK("::ffff:10.35.12.161"_ipv6   == "10.35.12.161"_ipv4);   // equal - mappd
    BOOST_CHECK("64:ff9b::10.35.12.161"_ipv6 == "10.35.12.161"_ipv4);   // equal - translated
    BOOST_CHECK("64:ff9b::10.35.12.162"_ipv6 != "64:ff9b::10.35.12.161"_ipv6);
    BOOST_CHECK("::ffff:10.35.12.162"_ipv6   != "10.35.12.161"_ipv4);
    BOOST_CHECK("64:ff9b::10.35.12.162"_ipv6 != "10.35.12.161"_ipv4);
    BOOST_CHECK("63:ff9b::10.35.12.161"_ipv6 != "10.35.12.161"_ipv4);
    BOOST_CHECK("64:ff9b::10.35.12.161"_ipv6 != "64:ff9b::10.35.12.162"_ipv6);
    BOOST_CHECK("64:ff9b::10.35.12.161"_ipv6 != "64:ff9b::10.35.12.161/112"_ipv6_net);
    BOOST_CHECK("64:ff9b::10.35.12.161"_ipv6 != "192.168.206.40/24"_ipv4_net);
  }
  // IPv6 network
  {
    BOOST_CHECK("200::1223:3456:789a:abcd:ef01/94"_ipv6_net    == "200::1223:3456:789a:abcd:ef01/94"_ipv6_net);
    BOOST_CHECK("200::1223:3456:789a:abcd:ef01/94"_ipv6_net    == "200::1223:3456:7898:0:0/94"_ipv6_net);   // netmask applied
    BOOST_CHECK("200:1:2:1223:3456:789a:abcd:ef01/96"_ipv6_net == "200:1:2:1223:3456:789a::/96"_ipv6_net); // netmasl applied

    BOOST_CHECK("200:1:2:1223:3456:789a:abcd:ef01/94"_ipv6_net != "200::1223:3456:789a:abcd:ef01/94"_ipv6_net);
    BOOST_CHECK("200:1:2:1223:3456:789a:abcd:ef01/96"_ipv6_net != "200::1223:3456:789a:abcd:ef01/96"_ipv6_net);
    BOOST_CHECK("200:1:2:1223:3456:789a:abcd:ef01/96"_ipv6_net != "200::1223:3456:789a:abcd:ef01/96"_ipv6);
    BOOST_CHECK("200:1:2:1223:3456:789a:abcd:ef01/96"_ipv6_net != "192.168.206.0/24"_ipv4_net);
    BOOST_CHECK("200:1:2:1223:3456:789a:abcd:ef01/96"_ipv6_net != "192.168.206.40/24"_ipv4);
  }
}

COOL_AUTO_TEST_CASE(T021,
  *utf::description("address::has() and address::in()"))
{
  ipv6::network net6_1("2001:ab33::/32");
  ipv6::network net6_2("2001:ab00::/24");
  ipv6::network net6_3("2001:ab44::/32");
  ipv6::host host6_1("2001:ab33::20");
  ipv6::host host6_2("2001:ab44::21");
  ipv6::host host6_3("2001:ab13::21");
  ipv6::host host6_4("::ffff:192.168.3.40");

  ipv4::network net4_1("10.60.0.0/16");
  ipv4::network net4_2("10.0.0.0/8");
  ipv4::network net4_3("10.62.0.0/16");
  ipv4::host host4_1("10.60.1.12");
  ipv4::host host4_2("10.62.55.42");
  ipv4::host host4_3("10.42.66.99");
  ipv4::host host4_4("192.168.3.44");

  // IN(net6_1, net6_2, true);
  BOOST_CHECK_EQUAL(true, net6_1.in(net6_2));
  BOOST_CHECK_EQUAL(true, net6_2.has(net6_1));
  //IN(net6_3, net6_2, true);
  BOOST_CHECK_EQUAL(true, net6_3.in(net6_2));
  BOOST_CHECK_EQUAL(true, net6_2.has(net6_3));
  // IN(net6_2, net6_3, false);
  BOOST_CHECK_EQUAL(false, net6_2.in(net6_3));
  BOOST_CHECK_EQUAL(false, net6_3.has(net6_2));
  // IN(net6_2, net6_1, false);
  BOOST_CHECK_EQUAL(false, net6_2.in(net6_1));
  BOOST_CHECK_EQUAL(false, net6_1.has(net6_2));
  // IN(net6_2, net4_3, false);
  BOOST_CHECK_EQUAL(false, net6_2.in(net4_3));
  BOOST_CHECK_EQUAL(false, net4_3.has(net6_2));
  // IN(net4_3, net6_2, false);
  BOOST_CHECK_EQUAL(false, net4_3.in(net6_2));
  BOOST_CHECK_EQUAL(false, net6_2.has(net4_3));

  // IN(host6_1, net6_1, true);
  BOOST_CHECK_EQUAL(true, host6_1.in(net6_1));
  BOOST_CHECK_EQUAL(true, net6_1.has(host6_1));
  // IN(host6_1, net6_2, true);
  BOOST_CHECK_EQUAL(true, host6_1.in(net6_2));
  BOOST_CHECK_EQUAL(true, net6_2.has(host6_1));
  // IN(host6_1, net6_3, false);
  BOOST_CHECK_EQUAL(false, host6_1.in(net6_3));
  BOOST_CHECK_EQUAL(false, net6_3.has(host6_1));
  // IN(host6_2, net6_1, false);
  BOOST_CHECK_EQUAL(false, host6_2.in(net6_1));
  BOOST_CHECK_EQUAL(false, net6_1.has(host6_2));
  // IN(host6_2, net6_2, true);
  BOOST_CHECK_EQUAL(true, host6_2.in(net6_2));
  BOOST_CHECK_EQUAL(true, net6_2.has(host6_2));
  // IN(host6_2, net6_3, true);
  BOOST_CHECK_EQUAL(true, host6_2.in(net6_3));
  BOOST_CHECK_EQUAL(true, net6_3.has(host6_2));
  // IN(host6_3, net6_1, false);
  BOOST_CHECK_EQUAL(false, host6_3.in(net6_1));
  BOOST_CHECK_EQUAL(false, net6_1.has(host6_3));
  // IN(host6_3, net6_2, true);
  BOOST_CHECK_EQUAL(true, host6_3.in(net6_2));
  BOOST_CHECK_EQUAL(true, net6_2.has(host6_3));
  // IN(host6_3, net6_3, false);
  BOOST_CHECK_EQUAL(false, host6_3.in(net6_3));
  BOOST_CHECK_EQUAL(false, net6_3.has(host6_3));
  // IN(host6_4, net6_1, false);
  BOOST_CHECK_EQUAL(false, host6_4.in(net6_1));
  BOOST_CHECK_EQUAL(false, net6_1.has(host6_4));
  // IN(host6_4, net6_2, false);
  BOOST_CHECK_EQUAL(false, host6_4.in(net6_2));
  BOOST_CHECK_EQUAL(false, net6_2.has(host6_4));
  // IN(host6_4, net6_3, false);
  BOOST_CHECK_EQUAL(false, host6_4.in(net6_3));
  BOOST_CHECK_EQUAL(false, net6_3.has(host6_4));

  // IN(net4_1, net4_2, true);
  BOOST_CHECK_EQUAL(true, net4_1.in(net4_2));
  BOOST_CHECK_EQUAL(true, net4_2.has(net4_1));
  // IN(net4_3, net4_2, true);
  BOOST_CHECK_EQUAL(true, net4_3.in(net4_2));
  BOOST_CHECK_EQUAL(true, net4_2.has(net4_3));
  // IN(net4_2, net4_3, false);
  BOOST_CHECK_EQUAL(false, net4_2.in(net4_3));
  BOOST_CHECK_EQUAL(false, net4_3.has(net4_2));
  // IN(net4_2, net4_1, false);
  BOOST_CHECK_EQUAL(false, net4_2.in(net4_1));
  BOOST_CHECK_EQUAL(false, net4_1.has(net4_2));

  // IN(host4_1, net4_1, true);
  BOOST_CHECK_EQUAL(true, host4_1.in(net4_1));
  BOOST_CHECK_EQUAL(true, net4_1.has(host4_1));
  // IN(host4_1, net4_2, true);
  BOOST_CHECK_EQUAL(true, host4_1.in(net4_2));
  BOOST_CHECK_EQUAL(true, net4_2.has(host4_1));
  // IN(host4_1, net4_3, false);
  BOOST_CHECK_EQUAL(false, host4_1.in(net4_3));
  BOOST_CHECK_EQUAL(false, net4_3.has(host4_1));
  // IN(host4_2, net4_1, false);
  BOOST_CHECK_EQUAL(false, host4_2.in(net4_1));
  BOOST_CHECK_EQUAL(false, net4_1.has(host4_2));
  // IN(host4_2, net4_2, true);
  BOOST_CHECK_EQUAL(true, host4_2.in(net4_2));
  BOOST_CHECK_EQUAL(true, net4_2.has(host4_2));
  // IN(host4_2, net4_3, true);
  BOOST_CHECK_EQUAL(true, host4_2.in(net4_3));
  BOOST_CHECK_EQUAL(true, net4_3.has(host4_2));
  // IN(host4_3, net4_1, false);
  BOOST_CHECK_EQUAL(false, host4_3.in(net4_1));
  BOOST_CHECK_EQUAL(false, net4_1.has(host4_3));
  // IN(host4_3, net4_2, true);
  BOOST_CHECK_EQUAL(true, host4_3.in(net4_2));
  BOOST_CHECK_EQUAL(true, net4_2.has(host4_3));
  // IN(host4_3, net4_3, false);
  BOOST_CHECK_EQUAL(false, host4_3.in(net4_3));
  BOOST_CHECK_EQUAL(false, net4_3.has(host4_3));
  // IN(host4_4, net4_1, false);
  BOOST_CHECK_EQUAL(false, host4_4.in(net4_1));
  BOOST_CHECK_EQUAL(false, net4_1.has(host4_4));
  // IN(host4_4, net4_2, false);
  BOOST_CHECK_EQUAL(false, host4_4.in(net4_2));
  BOOST_CHECK_EQUAL(false, net4_2.has(host4_4));
  // IN(host4_4, net4_3, false);
  BOOST_CHECK_EQUAL(false, host4_4.in(net4_3));
  BOOST_CHECK_EQUAL(false, net4_3.has(host4_4));

  // IN(host4_1, net6_1, false);
  BOOST_CHECK_EQUAL(false, host4_1.in(net6_1));
  BOOST_CHECK_EQUAL(false, net6_1.has(host4_1));
  // IN(host6_1, net4_1, false);
  BOOST_CHECK_EQUAL(false, host6_1.in(net4_1));
  BOOST_CHECK_EQUAL(false, net4_1.has(host6_1));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(host_container_)

COOL_AUTO_TEST_CASE(T001,
  *utf::description("ctor host_container()"))
{
  BOOST_CHECK_EQUAL(host_container(), ipv6::unspecified);
}

COOL_AUTO_TEST_CASE(T002,
  *utf::description("ctor host_container(const address&)"))
{
  {
    host_container c(ipv4::loopback);
    BOOST_CHECK_EQUAL(c, ipv4::loopback);
  }
  {
    host_container c(ipv6::loopback);
    BOOST_CHECK_EQUAL(c, ipv6::loopback);
  }
  {
    host_container c(ipv6::loopback);
    BOOST_CHECK_THROW(c = host_container(ipv6::rfc_ipv4map), cool::ng::exception::bad_conversion);
    BOOST_CHECK_THROW(c = host_container(ipv4::rfc_unset), cool::ng::exception::bad_conversion);
    BOOST_CHECK_EQUAL(c, ipv6::loopback);
  }
}

COOL_AUTO_TEST_CASE(T003,
  *utf::description("ctor host_container(const in_addr&)"))
{
  {
    host_container c;
    BOOST_CHECK_NO_THROW(c = host_container(static_cast<in_addr>("192.168.200.3"_ipv4)));
    BOOST_CHECK_EQUAL(c, "192.168.200.3"_ipv4);
  }
}
COOL_AUTO_TEST_CASE(T004,
  *utf::description("ctor host_container(const in6_addr&)"))
{
  {
    host_container c;
    BOOST_CHECK_NO_THROW(c = host_container(static_cast<in6_addr>("64:ff9b::ac04:c4b"_ipv6)));
    BOOST_CHECK_EQUAL(c, "64:ff9b::ac04:c4b"_ipv6);
  }
}
COOL_AUTO_TEST_CASE(T005,
  *utf::description("ctor host_container(const sockaddr_storage&)"))
{
  {
    sockaddr_storage s;
    s.ss_family = AF_INET;
    static_cast<sockaddr_in*>(static_cast<void*>(&s))->sin_addr = static_cast<in_addr>("192.168.4.16"_ipv4);
    host_container c;
    BOOST_CHECK_NO_THROW(c = host_container(s));
    BOOST_CHECK_EQUAL(c, "192.168.4.16"_ipv4);
  }
  {
    sockaddr_storage s;
    s.ss_family = AF_INET6;
    static_cast<sockaddr_in6*>(static_cast<void*>(&s))->sin6_addr = static_cast<in6_addr>("64:ff9b::ac04:c4b"_ipv6);
    host_container c;
    BOOST_CHECK_NO_THROW(c = host_container(s));
    BOOST_CHECK_EQUAL(c, "64:ff9b::ac04:c4b"_ipv6);
  }
  {
    sockaddr_storage s;
    s.ss_family = AF_INET6 + AF_INET;
    static_cast<sockaddr_in6*>(static_cast<void*>(&s))->sin6_addr = static_cast<in6_addr>("64:ff9b::ac04:c4b"_ipv6);
    host_container c;
    BOOST_CHECK_THROW(c = host_container(s), cool::ng::exception::bad_conversion);
  }
}

COOL_AUTO_TEST_CASE(T006,
  *utf::description("operator =(const address&)"))
{
  {
    host_container c;
    BOOST_CHECK_NO_THROW(c = ipv4::loopback);
    BOOST_CHECK_EQUAL(c, ipv4::loopback);
  }
  {
    host_container c;
    BOOST_CHECK_NO_THROW(c = ipv6::loopback);
    BOOST_CHECK_EQUAL(c, ipv6::loopback);
  }
  {
    host_container c(ipv6::loopback);
    BOOST_CHECK_THROW(c = ipv6::rfc_ipv4map, cool::ng::exception::bad_conversion);
    BOOST_CHECK_THROW(c = ipv4::rfc_unset, cool::ng::exception::bad_conversion);
    BOOST_CHECK_EQUAL(c, ipv6::loopback);
  }
}

COOL_AUTO_TEST_CASE(T007,
  *utf::description("operator =(const in_addr&)"))
{
  {
    host_container c;
    BOOST_CHECK_NO_THROW(c = static_cast<in_addr>("192.168.200.3"_ipv4));
    BOOST_CHECK_EQUAL(c, "192.168.200.3"_ipv4);
  }
}
COOL_AUTO_TEST_CASE(T008,
  *utf::description("operator =(const in6_addr&)"))
{
  {
    host_container c;
    BOOST_CHECK_NO_THROW(c = static_cast<in6_addr>("64:ff9b::ac04:c4b"_ipv6));
    BOOST_CHECK_EQUAL(c, "64:ff9b::ac04:c4b"_ipv6);
  }
}
COOL_AUTO_TEST_CASE(T009,
  *utf::description("operator =(const sockaddr_storage&)"))
{
  {
    sockaddr_storage s;
    s.ss_family = AF_INET;
    static_cast<sockaddr_in*>(static_cast<void*>(&s))->sin_addr = static_cast<in_addr>("192.168.4.16"_ipv4);
    host_container c;
    BOOST_CHECK_NO_THROW(c = s);
    BOOST_CHECK_EQUAL(c, "192.168.4.16"_ipv4);
  }
  {
    sockaddr_storage s;
    s.ss_family = AF_INET6;
    static_cast<sockaddr_in6*>(static_cast<void*>(&s))->sin6_addr = static_cast<in6_addr>("64:ff9b::ac04:c4b"_ipv6);
    host_container c;
    BOOST_CHECK_NO_THROW(c = s);
    BOOST_CHECK_EQUAL(c, "64:ff9b::ac04:c4b"_ipv6);
  }
  {
    sockaddr_storage s;
    s.ss_family = AF_INET6 + AF_INET;
    static_cast<sockaddr_in6*>(static_cast<void*>(&s))->sin6_addr = static_cast<in6_addr>("64:ff9b::ac04:c4b"_ipv6);
    host_container c;
    BOOST_CHECK_THROW(c = s, cool::ng::exception::bad_conversion);
  }
}

COOL_AUTO_TEST_CASE(T010,
  *utf::description("operator sockaddr_storage() const"))
{
  {
    uint8_t ref[] = { 192, 168, 13, 42};
    host_container c = ipv4::host(ref);
    auto sas = static_cast<sockaddr_storage>(c);
    BOOST_CHECK_EQUAL(AF_INET, sas.ss_family);
    auto sa = *reinterpret_cast<sockaddr_in*>(&sas);
    BOOST_CHECK_EQUAL(AF_INET, sa.sin_family);
    BOOST_CHECK_EQUAL(0, memcmp(&sa.sin_addr, ref, sizeof(ref)));
  }
  {
    uint8_t ref[] = { 0x02, 0, 0, 0, 0, 0, 0x12, 0x23, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x01 };
    host_container c = ipv6::host(ref);
    auto sas = static_cast<sockaddr_storage>(c);
    BOOST_CHECK_EQUAL(AF_INET6, sas.ss_family);
    auto sa = *reinterpret_cast<sockaddr_in6*>(&sas);
    BOOST_CHECK_EQUAL(AF_INET6, sa.sin6_family);
    BOOST_CHECK_EQUAL(0, memcmp(&sa.sin6_addr, ref, sizeof(ref)));
  }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(service_)

COOL_AUTO_TEST_CASE(T001,
  *utf::description("ctor service()"))
{
  service s;
  std::string aux;

  BOOST_CHECK(!s);
  BOOST_CHECK_THROW(s.socket_type(), cool::ng::exception::bad_conversion);
  BOOST_CHECK_EQUAL(s.socket_domain(), AF_INET6);
  BOOST_CHECK_THROW(aux = static_cast<std::string>(s), cool::ng::exception::bad_conversion);
}

COOL_AUTO_TEST_CASE(T002,
  *utf::description("ctor service(transport)"))
{
  {
    service s;
    BOOST_CHECK_NO_THROW(s = service(transport::tcp));
    BOOST_CHECK(s);
    BOOST_CHECK_EQUAL(s.socket_type(), SOCK_STREAM);
    BOOST_CHECK_EQUAL(s.socket_domain(), AF_INET6);
    BOOST_CHECK_EQUAL(static_cast<std::string>(s), "tcp://[::]:0");
  }
  {
    service s;
    BOOST_CHECK_NO_THROW(s = service(transport::udp));
    BOOST_CHECK(s);
    BOOST_CHECK_EQUAL(s.socket_type(), SOCK_DGRAM);
    BOOST_CHECK_EQUAL(s.socket_domain(), AF_INET6);
    BOOST_CHECK_EQUAL(static_cast<std::string>(s), "udp://[::]:0");
  }
  {
    service s;
    BOOST_CHECK_THROW(s = service(transport::unknown), cool::ng::exception::illegal_argument);
    BOOST_CHECK(!s);
  }
}

COOL_AUTO_TEST_CASE(T003,
  *utf::description("ctor service(transport, const sockaddr*)"))
{
  {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr = static_cast<in_addr>("192.168.3.99"_ipv4);
    addr.sin_port = htons(7778);

    service s;
    BOOST_CHECK_NO_THROW(s = service(transport::tcp, static_cast<sockaddr*>(static_cast<void*>(&addr))));
    BOOST_CHECK(s);
    BOOST_CHECK_EQUAL(s.socket_type(), SOCK_STREAM);
    BOOST_CHECK_EQUAL(s.socket_domain(), AF_INET);
    BOOST_CHECK_EQUAL(static_cast<std::string>(s), "tcp://192.168.3.99:7778");
  }
  {
    sockaddr_in6 addr;
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = static_cast<in6_addr>("200::1223:3456:789a:bcde:f001"_ipv6);
    addr.sin6_port = htons(4778);

    service s;
    BOOST_CHECK_NO_THROW(s = service(transport::udp, static_cast<sockaddr*>(static_cast<void*>(&addr))));
    BOOST_CHECK(s);
    BOOST_CHECK_EQUAL(s.socket_type(), SOCK_DGRAM);
    BOOST_CHECK_EQUAL(s.socket_domain(), AF_INET6);
    BOOST_CHECK_EQUAL(static_cast<std::string>(s), "udp://[200::1223:3456:789a:bcde:f001]:4778");
  }
  {
    sockaddr_in6 addr;
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = static_cast<in6_addr>("200::1223:3456:789a:bcde:f001"_ipv6);
    addr.sin6_port = htons(4778);

    service s;
    BOOST_CHECK_THROW(s = service(transport::unknown, static_cast<sockaddr*>(static_cast<void*>(&addr))), cool::ng::exception::illegal_argument);
    BOOST_CHECK(!s);
  }
  {
    sockaddr_in addr;
    addr.sin_family = AF_UNIX;
    service s;
    BOOST_CHECK_THROW(s = service(transport::tcp, static_cast<sockaddr*>(static_cast<void*>(&addr))), cool::ng::exception::bad_conversion);
    BOOST_CHECK(!s);
  }
  {
    service s;
    BOOST_CHECK_THROW(s = service(transport::tcp, nullptr), cool::ng::exception::illegal_argument);
    BOOST_CHECK(!s);
  }
}

COOL_AUTO_TEST_CASE(T004,
  *utf::description("ctor service(transport, address, uint16_t)"))
{
  {
    service s;
    BOOST_CHECK_THROW(s = service(transport::unknown, "192.168.3.99"_ipv4, 4242), cool::ng::exception::illegal_argument);
  }
  {
    service s;
    BOOST_CHECK_THROW(s = service(transport::tcp, "192.168.3.0/24"_ipv4_net, 4242), cool::ng::exception::bad_conversion);
  }
  {
    service s;
    BOOST_CHECK_THROW(s = service(transport::udp, "ffee::/16"_ipv6_net, 4242), cool::ng::exception::bad_conversion);
  }
  {
    service s;
    BOOST_CHECK_NO_THROW(s = service(transport::udp, "192.168.3.99"_ipv4, 4242));
    BOOST_CHECK_EQUAL(s.socket_type(), SOCK_DGRAM);
    BOOST_CHECK_EQUAL(s.socket_domain(), AF_INET);
    BOOST_CHECK_EQUAL(static_cast<std::string>(s), "udp://192.168.3.99:4242");
  }
  {
    service s;
    BOOST_CHECK_NO_THROW(s = service(transport::tcp, "200::1223:3456:789a:bcde:f001"_ipv6, 4242));
    BOOST_CHECK_EQUAL(s.socket_type(), SOCK_STREAM);
    BOOST_CHECK_EQUAL(s.socket_domain(), AF_INET6);
    BOOST_CHECK_EQUAL(static_cast<std::string>(s), "tcp://[200::1223:3456:789a:bcde:f001]:4242");
  }
}

COOL_AUTO_TEST_CASE(T005,
  *utf::description("ctor service(const std::string&)"))
{
  {
    service s;
    BOOST_CHECK_NO_THROW(s = service("udp://192.16.32.11:8032"));
    BOOST_CHECK_EQUAL(s.socket_type(), SOCK_DGRAM);
    BOOST_CHECK_EQUAL(s.socket_domain(), AF_INET);
    BOOST_CHECK_EQUAL(static_cast<std::string>(s), "udp://192.16.32.11:8032");
  }
  {
    service s;
    BOOST_CHECK_NO_THROW(s = service("tcp://[200::1223:3456:789a:bcde:f001]:4242"));
    BOOST_CHECK_EQUAL(s.socket_type(), SOCK_STREAM);
    BOOST_CHECK_EQUAL(s.socket_domain(), AF_INET6);
    BOOST_CHECK_EQUAL(static_cast<std::string>(s), "tcp://[200::1223:3456:789a:bcde:f001]:4242");
  }
  {
    service s;
    BOOST_CHECK_THROW(s = service("tcp://[200::1223:3456:789a:bcde:f001:4242"), cool::ng::exception::bad_conversion);
  }
  {
    service s;
    BOOST_CHECK_THROW(s = service("arg://[200::1223:3456:789a:bcde:f001:4242"), cool::ng::exception::bad_conversion);
  }
  {
    service s;
    BOOST_CHECK_THROW(s = service("tcp://192.168.3:1212"), cool::ng::exception::bad_conversion);
  }
  {
    service s;
    BOOST_CHECK_THROW(s = service("tcp://192.168.3.12;1212"), cool::ng::exception::bad_conversion);
  }
}

COOL_AUTO_TEST_CASE(T006,
  *utf::description("opeartor =(const sockaddr*)"))
{
  {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr = static_cast<in_addr>("192.168.3.99"_ipv4);
    addr.sin_port = htons(7778);

    service s;
    BOOST_CHECK_THROW(s = reinterpret_cast<sockaddr*>(&addr), cool::ng::exception::invalid_state);
    BOOST_CHECK(!s);
  }
  {
    service s(transport::tcp);
    BOOST_CHECK_THROW(s = nullptr, cool::ng::exception::illegal_argument);
  }
  {
    sockaddr_in addr;
    addr.sin_family = AF_UNIX;
    service s("udp://172.16.16.16:8080");
    BOOST_CHECK_THROW(s = static_cast<sockaddr*>(static_cast<void*>(&addr)), cool::ng::exception::bad_conversion);
  }
  {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr = static_cast<in_addr>("192.168.3.99"_ipv4);
    addr.sin_port = htons(7778);

    service s("tcp://[200::1223:3456:789a:bcde:f001]:4242");
    BOOST_CHECK_NO_THROW(s = reinterpret_cast<sockaddr*>(&addr));
    BOOST_CHECK_EQUAL(s.socket_type(), SOCK_STREAM);
    BOOST_CHECK_EQUAL(s.socket_domain(), AF_INET);
    BOOST_CHECK_EQUAL(static_cast<std::string>(s), "tcp://192.168.3.99:7778");
  }
  {
    sockaddr_in6 addr;
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = static_cast<in6_addr>("200::1223:3456:789a:bcde:f001"_ipv6);
    addr.sin6_port = htons(4778);

    service s("udp://172.16.16.16:8080");
    BOOST_CHECK_NO_THROW(s = static_cast<sockaddr*>(static_cast<void*>(&addr)));
    BOOST_CHECK(s);
    BOOST_CHECK_EQUAL(s.socket_type(), SOCK_DGRAM);
    BOOST_CHECK_EQUAL(s.socket_domain(), AF_INET6);
    BOOST_CHECK_EQUAL(static_cast<std::string>(s), "udp://[200::1223:3456:789a:bcde:f001]:4778");
  }
}

COOL_AUTO_TEST_CASE(T007,
  *utf::description("opeartor =(const std::string&)"))
{
  {
    service s;
    BOOST_CHECK_NO_THROW(s = "udp://192.16.32.11:8032");
    BOOST_CHECK_EQUAL(s.socket_type(), SOCK_DGRAM);
    BOOST_CHECK_EQUAL(s.socket_domain(), AF_INET);
    BOOST_CHECK_EQUAL(static_cast<std::string>(s), "udp://192.16.32.11:8032");
  }
  {
    service s;
    BOOST_CHECK_NO_THROW(s = "tcp://[200::1223:3456:789a:bcde:f001]:4242");
    BOOST_CHECK_EQUAL(s.socket_type(), SOCK_STREAM);
    BOOST_CHECK_EQUAL(s.socket_domain(), AF_INET6);
    BOOST_CHECK_EQUAL(static_cast<std::string>(s), "tcp://[200::1223:3456:789a:bcde:f001]:4242");
  }
  {
    service s;
    BOOST_CHECK_THROW(s = "tcp://[200::1223:3456:789a:bcde:f001:4242", cool::ng::exception::bad_conversion);
  }
  {
    service s;
    BOOST_CHECK_THROW(s = "arg://[200::1223:3456:789a:bcde:f001:4242", cool::ng::exception::bad_conversion);
  }
  {
    service s;
    BOOST_CHECK_THROW(s = "tcp://192.168.3:1212", cool::ng::exception::bad_conversion);
  }
  {
    service s;
    BOOST_CHECK_THROW(s = "tcp://192.168.3.12;1212", cool::ng::exception::bad_conversion);
  }
}

COOL_AUTO_TEST_CASE(T008,
  *utf::description("service::visualize()"))
{
  {
    service s;
    std::stringstream ss;
    BOOST_CHECK_THROW(s.visualize(ss), cool::ng::exception::bad_conversion);
  }
  {
    service s(transport::udp, "192.168.3.99"_ipv4, 2222);
    {
      std::stringstream ss;
      BOOST_CHECK_NO_THROW(s.visualize(ss));
      BOOST_CHECK_EQUAL(ss.str(), "udp://192.168.3.99:2222");
    }
    {
      std::stringstream ss;
      BOOST_CHECK_NO_THROW(s.visualize(ss, style::dot_decimal));
      BOOST_CHECK_EQUAL(ss.str(), "udp://192.168.3.99:2222");
    }
    {
      std::stringstream ss;
      BOOST_CHECK_THROW(s.visualize(ss, style::canonical), cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(s.visualize(ss, style::strict), cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(s.visualize(ss, style::expanded), cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(s.visualize(ss, style::microsoft), cool::ng::exception::bad_conversion);
      BOOST_CHECK_THROW(s.visualize(ss, style::dotted_quad), cool::ng::exception::bad_conversion);
    }
  }
  {
    service s(transport::tcp, "200::1223:3456:789a:bcde:f001"_ipv6, 44444);
    {
      std::stringstream ss;
      BOOST_CHECK_NO_THROW(s.visualize(ss));
      BOOST_CHECK_EQUAL(ss.str(), "tcp://[200::1223:3456:789a:bcde:f001]:44444");
    }
    {
      std::stringstream ss;
      BOOST_CHECK_NO_THROW(s.visualize(ss, style::canonical));
      BOOST_CHECK_EQUAL(ss.str(), "tcp://[200::1223:3456:789a:bcde:f001]:44444");
    }
    {
      std::stringstream ss;
      BOOST_CHECK_NO_THROW(s.visualize(ss, style::strict));
      BOOST_CHECK_EQUAL(ss.str(), "tcp://[200::1223:3456:789a:bcde:f001]:44444");
    }
    {
      std::stringstream ss;
      BOOST_CHECK_NO_THROW(s.visualize(ss, style::expanded));
      BOOST_CHECK_EQUAL(ss.str(), "tcp://[200:0:0:1223:3456:789a:bcde:f001]:44444");
    }
    {
      std::stringstream ss;
      BOOST_CHECK_NO_THROW(s.visualize(ss, style::microsoft));
      BOOST_CHECK_EQUAL(ss.str(), "tcp://[200--1223-3456-789a-bcde-f001]:44444");
    }
    {
      std::stringstream ss;
      BOOST_CHECK_NO_THROW(s.visualize(ss, style::dotted_quad));
      BOOST_CHECK_EQUAL(ss.str(), "tcp://[200::1223:3456:789a:188.222.240.1]:44444");
    }
    {
      std::stringstream ss;
      BOOST_CHECK_THROW(s.visualize(ss, style::dot_decimal), cool::ng::exception::bad_conversion);
    }
  }
}

COOL_AUTO_TEST_CASE(T009,
*utf::description("service::sockaddr() const"))
{
  {
    service s;
    const struct sockaddr* ptr;
    socklen_t len;

    BOOST_CHECK_THROW(ptr = s.sockaddr(), cool::ng::exception::bad_conversion);
    BOOST_CHECK_THROW(len = s.sockaddr_len(), cool::ng::exception::bad_conversion);
  }
  {
    service s("tcp://[200::1223:3456:789a:bcde:f001]:4242");
    const struct sockaddr* ptr;
    socklen_t len;
    BOOST_CHECK_NO_THROW(ptr = s.sockaddr());
    BOOST_CHECK_NO_THROW(len = s.sockaddr_len());
    BOOST_CHECK_EQUAL(len, sizeof(struct sockaddr_in6));
    BOOST_REQUIRE(ptr->sa_family == AF_INET6);
    BOOST_CHECK_EQUAL(s.sockaddr_len(), sizeof(struct sockaddr_in6));
    auto q = reinterpret_cast<const struct sockaddr_in6*>(ptr);
    auto h = ipv6::host(q->sin6_addr);
    BOOST_CHECK_EQUAL(h, s.host());
  }
  {
    service s("tcp://192.168.3.100:4242");
    const struct sockaddr* ptr;
    socklen_t len;
    BOOST_CHECK_NO_THROW(ptr = s.sockaddr());
    BOOST_CHECK_NO_THROW(len = s.sockaddr_len());
    BOOST_CHECK_EQUAL(len, sizeof(struct sockaddr_in));
    BOOST_REQUIRE(ptr->sa_family == AF_INET);
    BOOST_CHECK_EQUAL(s.sockaddr_len(), sizeof(struct sockaddr_in));
    auto q = reinterpret_cast<const struct sockaddr_in*>(ptr);
    auto h = ipv6::host(q->sin_addr);
    BOOST_CHECK_EQUAL(h, s.host());
  }
}


BOOST_AUTO_TEST_SUITE_END()


