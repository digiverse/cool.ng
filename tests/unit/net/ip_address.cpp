
#include <stdint.h>
#include <iostream>
#include <sstream>

#if !defined(WINDOWS_TARGET)

#include <arpa/inet.h>

#else

# pragma comment(lib, "Ws2_32.lib")

#endif

#define BOOST_TEST_MODULE IpAddress
#include <boost/test/unit_test.hpp>

#include "cool/ng/ip_address.h"

using namespace cool::ng::net;

ipv6::host ip6_host_examples[] = {
  { 0x02, 0x00, 0x00, 0x00,/**/ 0x00, 0x00, 0x12, 0x23,/**/ 0x34, 0x56, 0x78, 0x9a,/**/ 0xbc, 0xde, 0xf0, 0x01 },
  { 0x02, 0x03, 0x00, 0x00,/**/ 0x00, 0x00, 0x12, 0x23,/**/ 0x34, 0x00, 0x00, 0x00,/**/ 0x00, 0x00, 0xf0, 0x01 },
  { 0xab, 0x03, 0x00, 0x00,/**/ 0x00, 0x00, 0x12, 0x34,/**/ 0x56, 0x78, 0x00, 0x00,/**/ 0x00, 0x00, 0x00, 0x00 },
  { 0xab, 0x03, 0x00, 0x00,/**/ 0x00, 0x00, 0x12, 0x34,/**/ 0x56, 0x78, 0x00, 0x01,/**/ 0x00, 0x00, 0x00, 0x00 },
  { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0xff, 0xff, 0x00, 0x00,     0x0a, 0x0b, 0x0c, 0x0d },
  { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0x0a, 0x0b, 0x0c, 0x0d },
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

BOOST_AUTO_TEST_SUITE(ip)

std::string str(const ipv6::host& addr, ipv6::Style style)
{
  std::stringstream ss;
  addr.visualize(ss, style);
  return ss.str();
}

BOOST_AUTO_TEST_CASE(ip6_host_str)
{

  ipv6::host addr;

  addr = ip6_host_examples[0];
  BOOST_CHECK_EQUAL("200::1223:3456:789a:bcde:f001", str(addr, ipv6::Canonical));
  BOOST_CHECK_EQUAL("200::1223:3456:789a:bcde:f001", str(addr, ipv6::StrictCanonical));
  BOOST_CHECK_EQUAL("200:0:0:1223:3456:789a:bcde:f001", str(addr, ipv6::Expanded));
  BOOST_CHECK_EQUAL("200--1223-3456-789a-bcde-f001", str(addr, ipv6::Microsoft));
  BOOST_CHECK_EQUAL("200::1223:3456:789a:188.222.240.1", str(addr, ipv6::DottedQuad));

  addr = ip6_host_examples[1];
  BOOST_CHECK_EQUAL("203::1223:3400:0:0:f001", str(addr, ipv6::Canonical));
  BOOST_CHECK_EQUAL("203::1223:3400:0:0:f001", str(addr, ipv6::StrictCanonical));
  BOOST_CHECK_EQUAL("203:0:0:1223:3400:0:0:f001", str(addr, ipv6::Expanded));
  BOOST_CHECK_EQUAL("203--1223-3400-0-0-f001", str(addr, ipv6::Microsoft));
  BOOST_CHECK_EQUAL("203::1223:3400:0:0.0.240.1", str(addr, ipv6::DottedQuad));

  addr = ip6_host_examples[2];
  BOOST_CHECK_EQUAL("ab03:0:0:1234:5678::", str(addr, ipv6::Canonical));
  BOOST_CHECK_EQUAL("ab03:0:0:1234:5678::", str(addr, ipv6::StrictCanonical));
  BOOST_CHECK_EQUAL("ab03:0:0:1234:5678:0:0:0", str(addr, ipv6::Expanded));
  BOOST_CHECK_EQUAL("ab03-0-0-1234-5678--", str(addr, ipv6::Microsoft));
  BOOST_CHECK_EQUAL("ab03::1234:5678:0:0.0.0.0", str(addr, ipv6::DottedQuad));

  addr = ip6_host_examples[3];
  BOOST_CHECK_EQUAL("ab03::1234:5678:1:0:0", str(addr, ipv6::Canonical));
  BOOST_CHECK_EQUAL("ab03::1234:5678:1:0:0", str(addr, ipv6::StrictCanonical));
  BOOST_CHECK_EQUAL("ab03:0:0:1234:5678:1:0:0", str(addr, ipv6::Expanded));
  BOOST_CHECK_EQUAL("ab03--1234-5678-1-0-0", str(addr, ipv6::Microsoft));
  BOOST_CHECK_EQUAL("ab03::1234:5678:1:0.0.0.0", str(addr, ipv6::DottedQuad));

  addr = ip6_host_examples[4];
  BOOST_CHECK_EQUAL("::ffff:0:a0b:c0d", str(addr, ipv6::Canonical));
  BOOST_CHECK_EQUAL("::ffff:0:a0b:c0d", str(addr, ipv6::StrictCanonical));
  BOOST_CHECK_EQUAL("0:0:0:0:ffff:0:a0b:c0d", str(addr, ipv6::Expanded));
  BOOST_CHECK_EQUAL("--ffff-0-a0b-c0d", str(addr, ipv6::Microsoft));
  BOOST_CHECK_EQUAL("::ffff:0:10.11.12.13", str(addr, ipv6::DottedQuad));

  addr = ip6_host_examples[5];
  BOOST_CHECK_EQUAL("::ffff:10.11.12.13", str(addr, ipv6::Canonical));
  BOOST_CHECK_EQUAL("::ffff:a0b:c0d", str(addr, ipv6::StrictCanonical));
  BOOST_CHECK_EQUAL("0:0:0:0:0:ffff:a0b:c0d", str(addr, ipv6::Expanded));
  BOOST_CHECK_EQUAL("--ffff-a0b-c0d", str(addr, ipv6::Microsoft));
  BOOST_CHECK_EQUAL("::ffff:10.11.12.13", str(addr, ipv6::DottedQuad));
}

BOOST_AUTO_TEST_CASE(ip6_host_parse)
{
  ipv6::host addr;
  {
    std::stringstream ss(ip6_host_input[0]);
    ss >> addr;
    BOOST_CHECK_EQUAL("::1", static_cast<std::string>(addr));
  }
  {
    std::stringstream ss(ip6_host_input[1]);
    ss >> addr;
    BOOST_CHECK_EQUAL("::1", static_cast<std::string>(addr));
  }
  {
    std::stringstream ss(ip6_host_input[2]);
    ss >> addr;
    BOOST_CHECK_EQUAL("2605:2700:0:3::4713:93e3", static_cast<std::string>(addr));
  }
  {
    std::stringstream ss(ip6_host_input[3]);
    ss >> addr;
    BOOST_CHECK_EQUAL("2605:2700:0:3::4713:93e3", static_cast<std::string>(addr));
  }
  {
    std::stringstream ss(ip6_host_input[4]);
    ss >> addr;
    BOOST_CHECK_EQUAL("::ffff:192.168.173.22", static_cast<std::string>(addr));
  }
  {
    std::stringstream ss(ip6_host_input[5]);
    ss >> addr;
    BOOST_CHECK_EQUAL("::ffff:192.168.173.22", static_cast<std::string>(addr));
  }
  {
    std::stringstream ss(ip6_host_input[6]);
    ss >> addr;
    BOOST_CHECK_EQUAL("::c0a8:ad16", static_cast<std::string>(addr));
  }
  {
    std::stringstream ss(ip6_host_input[7]);
    ss >> addr;
    BOOST_CHECK_EQUAL("ffff::c0a8:ad16", static_cast<std::string>(addr));
  }
  {
    std::stringstream ss(ip6_host_input[8]);
    ss >> addr;
    BOOST_CHECK_EQUAL("1::", static_cast<std::string>(addr));
  }
  {
    std::stringstream ss(ip6_host_input[9]);
    ss >> addr;
    BOOST_CHECK_EQUAL("1::", static_cast<std::string>(addr));
  }
  {
    std::stringstream ss(ip6_host_input[10]);
    ss >> addr;
    BOOST_CHECK_EQUAL("::", static_cast<std::string>(addr));
  }
  {
    std::stringstream ss(ip6_host_input[11]);
    ss >> addr;
    BOOST_CHECK_EQUAL("::", static_cast<std::string>(addr));
  }
  {
    std::stringstream ss(ip6_host_input[12]);
    ss >> addr;
    BOOST_CHECK_EQUAL("2605:2700:0:3::4713:93e3", static_cast<std::string>(addr));
  }
  {
    std::stringstream ss(ip6_host_input[13]);
    ss >> addr;
    BOOST_CHECK_EQUAL("2605:2700:0:3::4713:93e3", static_cast<std::string>(addr));
  }
  {
    std::stringstream ss(ip6_host_input[14]);
    ss >> addr;
    BOOST_CHECK_EQUAL("2605:2700:abcd::", static_cast<std::string>(addr));
  }
}

#if defined(WINDOWS_TARGET)
# define INET_PTON_(af_, src_, dst_) InetPton(af_, src_, dst_)
#else
# define INET_PTON_(af_, src_, dst_) inet_pton(af_, src_, dst_)
#endif

// ------ struct in_addr/in6_addr assignment and type conversion

BOOST_AUTO_TEST_CASE(in_addr_conversions)
{
  { // ==== host
     ipv6::host ip6_r1    = { 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0xff,0xff, 0xc0,0xa8, 0x03,0x14 };
    ipv6::host ip6_r2    = { 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0xef,0xff, 0xc0,0xa8, 0x03,0x14 };
    ipv4::host ip4_r1    = { 192, 168, 3, 20 };
    ipv6::network ip6_r3 = { 96, { 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0xff,0xff} };
    ipv4::network ip4_r2 = { 24, { 192, 168, 3, 0 } };

    struct in6_addr ip6_ref;
    struct in6_addr ip6_ref_2;
    struct in_addr  ip4_ref;

    INET_PTON_(AF_INET6, "::ffff:c0a8:314", &ip6_ref);
    INET_PTON_(AF_INET6, "::efff:c0a8:314", &ip6_ref_2);
    INET_PTON_(AF_INET, "192.168.3.20", &ip4_ref);

   // ctors
    {
      in6_addr ip6_ref_3;
      INET_PTON_(AF_INET6, "2001::", &ip6_ref_3);

      { // first make sure that ctors won't throw
        BOOST_CHECK_NO_THROW(ipv6::host h1(ip6_ref_2));
        BOOST_CHECK_NO_THROW(ipv6::host h2(ip4_ref));
        BOOST_CHECK_NO_THROW(ipv6::network n1(96, ip6_ref));
        BOOST_CHECK_NO_THROW(ipv4::host h3(ip4_ref));
        BOOST_CHECK_NO_THROW(ipv4::host h4(ip6_ref));
        BOOST_CHECK_NO_THROW(ipv4::network n2(24, ip4_ref));
        BOOST_CHECK_THROW(ipv4::host h(ip6_ref_2), cool::ng::exception::bad_conversion);
      }
      // next check that values are correct - CHECK_NO_THROW is scoped, so
      // construct variables again
      ipv6::host h1(ip6_ref_2);
      ipv6::host h2(ip4_ref);
      ipv6::network n1(96, ip6_ref);
      ipv4::host h3(ip4_ref);
      ipv4::host h4(ip6_ref);
      ipv4::network n2(24, ip4_ref);

      BOOST_CHECK_EQUAL(h1, ip6_r2);
      BOOST_CHECK_EQUAL(h2, ip6_r1);
      BOOST_CHECK_EQUAL(n1, ip6_r3);
      BOOST_CHECK_EQUAL(h3, ip4_r1);
      BOOST_CHECK_EQUAL(h4, ip4_r1);
      BOOST_CHECK_EQUAL(n2, ip4_r2);
    }

    // ipv6::host in6_addr from/to conversions
    {
      in6_addr ip6_aux;
      in_addr ip4_aux;
      ::memset(&ip6_aux, 0, sizeof(ip6_aux));
      ::memset(&ip4_aux, 0, sizeof(ip4_aux));

      ipv6::host ip6_1;

      BOOST_CHECK_NO_THROW(ip6_1 = ip6_ref);
      BOOST_CHECK_EQUAL(ip6_r1, ip6_1);
      BOOST_CHECK_NO_THROW(ip6_1 = ipv6::loopback);  // reset content
      BOOST_CHECK_NO_THROW(ip6_1 = ip4_ref);         // to produce IPv4 mapped address
      BOOST_CHECK_EQUAL(ip6_r1, ip6_1);

      BOOST_CHECK_NO_THROW(ip6_aux = static_cast<in6_addr>(ip6_1));
      BOOST_CHECK_EQUAL(0, ::memcmp(&ip6_aux, &ip6_ref, sizeof(in6_addr)));
      BOOST_CHECK_NO_THROW(ip4_aux = static_cast<in_addr>(ip6_1));
      BOOST_CHECK_EQUAL(0, ::memcmp(&ip4_aux, &ip4_ref, sizeof(in_addr)));
    }
    // ipv4::host in6_addr from/to conversions
    {
      in6_addr ip6_aux;
      in_addr ip4_aux;
      ::memset(&ip6_aux, 0, sizeof(ip6_aux));
      ::memset(&ip4_aux, 0, sizeof(ip4_aux));

      ipv4::host ip4_1;

      BOOST_CHECK_NO_THROW(ip4_1 = ip4_ref);
      BOOST_CHECK_EQUAL(ip4_1, ip4_r1);

      BOOST_CHECK_NO_THROW(ip4_1 = ipv4::loopback); // reset content
      BOOST_CHECK_NO_THROW(ip4_1 = ip6_ref);   // legal to assign in6_addr if mapped network
      BOOST_CHECK_EQUAL(ip4_1, ip4_r1);
      BOOST_CHECK_THROW(ip4_1 = ip6_ref_2, cool::ng::exception::bad_conversion); // not legal to assign other in6_addr

      BOOST_CHECK_NO_THROW(ip4_aux = static_cast<in_addr>(ip4_1));
      BOOST_CHECK_EQUAL(0, ::memcmp(&ip4_aux, &ip4_ref, sizeof(in_addr)));
      BOOST_CHECK_NO_THROW(ip6_aux = static_cast<in6_addr>(ip4_1));
      BOOST_CHECK_EQUAL(0, ::memcmp(&ip6_aux, &ip6_ref, sizeof(in6_addr)));
    }
  }
  { // ==== network
    ipv6::network ip6_r1 = { 112, { 0x20, 0x01 } };
    ipv4::network ip4_r1 = { 24, { 192, 168, 3, 0 } };

    struct in6_addr ip6_ref;
    struct in_addr  ip4_ref;
    INET_PTON_(AF_INET6, "2001::", &ip6_ref);
    INET_PTON_(AF_INET, "192.168.3.0", &ip4_ref);

    in6_addr ip6_aux;
    in_addr ip4_aux;

    // ---- ipv6::network
    {
      ipv6::network ip6_1 = { 112, { 0 } };
      cool::ng::net::ip::address& ref = ip6_1;

      ::memset(&ip6_aux, 0, sizeof(ip6_aux));
      ::memset(&ip4_aux, 0, sizeof(ip4_aux));

      BOOST_CHECK_NO_THROW(ip6_1 = ip6_ref);
      BOOST_CHECK_EQUAL(ip6_1, ip6_r1);
      BOOST_CHECK_THROW(ref = ip4_ref, cool::ng::exception::bad_conversion);

      BOOST_CHECK_NO_THROW(ip6_aux = static_cast<in6_addr>(ip6_r1));
      BOOST_CHECK_EQUAL(0, ::memcmp(&ip6_aux, &ip6_ref, sizeof(in6_addr)));
      BOOST_CHECK_THROW(ip4_aux = static_cast<in_addr>(ref), cool::ng::exception::bad_conversion);
    }
    // ---- ipv4::network
    {
      ipv4::network ip4_1 = { 24, { 0 } };
      cool::ng::net::ip::address& ref = ip4_1;

      ::memset(&ip6_aux, 0, sizeof(ip6_aux));
      ::memset(&ip4_aux, 0, sizeof(ip4_aux));

      BOOST_CHECK_NO_THROW(ip4_1 = ip4_ref);
      BOOST_CHECK_EQUAL(ip4_1, ip4_r1);
      BOOST_CHECK_THROW(ref = ip6_ref, cool::ng::exception::bad_conversion);

      BOOST_CHECK_NO_THROW(ip4_aux = static_cast<in_addr>(ip4_r1));
      BOOST_CHECK_EQUAL(0, ::memcmp(&ip4_aux, &ip4_ref, sizeof(in_addr)));
      BOOST_CHECK_THROW(ip6_aux = static_cast<in6_addr>(ip4_r1), cool::ng::exception::bad_conversion);
    }
  }
}

BOOST_AUTO_TEST_CASE(byte_ptr_conversion)
{
  uint8_t ip_ref[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10 };
  ipv6::host r6h = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10 };
  ipv6::network r6n = { 128, { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10 } };
  ipv4::host r4h = { 0x01, 0x02, 0x03, 0x04 };
  ipv4::network r4n= { 32, { 0x01, 0x02, 0x03, 0x04 } };

  { // ctor
    ipv6::host h6(ip_ref);
    ipv6::network n6(128, ip_ref);
    ipv4::host h4(ip_ref);
    ipv4::network n4(32, ip_ref);

    BOOST_CHECK_EQUAL(r6h, h6);
    BOOST_CHECK_EQUAL(r6n, n6);
    BOOST_CHECK_EQUAL(r4h, h4);
    BOOST_CHECK_EQUAL(r4n, n4);
  }
  { // assignment
    ipv6::host h6;
    ipv6::network n6(128);
    ipv4::host h4;
    ipv4::network n4(32);

    h6 = ip_ref;
    n6 = ip_ref;
    h4 = ip_ref;
    n4 = ip_ref;

    BOOST_CHECK_EQUAL(r6h, h6);
    BOOST_CHECK_EQUAL(r6n, n6);
    BOOST_CHECK_EQUAL(r4h, h4);
    BOOST_CHECK_EQUAL(r4n, n4);
  }
  { // type conversion
    ipv6::host h6;
    ipv6::network n6(128);
    ipv4::host h4;
    ipv4::network n4(32);

    BOOST_CHECK_EQUAL(0, ::memcmp(ip_ref, static_cast<uint8_t*>(r6h), sizeof(in6_addr)));
    BOOST_CHECK_EQUAL(0, ::memcmp(ip_ref, static_cast<uint8_t*>(r6n), sizeof(in6_addr)));
    BOOST_CHECK_EQUAL(0, ::memcmp(ip_ref, static_cast<uint8_t*>(r4h), sizeof(in_addr)));
    BOOST_CHECK_EQUAL(0, ::memcmp(ip_ref, static_cast<uint8_t*>(r4n), sizeof(in_addr)));

    ::memcpy(static_cast<uint8_t*>(h6), ip_ref, 16);
    ::memcpy(static_cast<uint8_t*>(n6), ip_ref, 16);
    ::memcpy(static_cast<uint8_t*>(h4), ip_ref, 4);
    ::memcpy(static_cast<uint8_t*>(n4), ip_ref, 4);

    BOOST_CHECK_EQUAL(h6, r6h);
    BOOST_CHECK_EQUAL(n6, r6n);
    BOOST_CHECK_EQUAL(h4, r4h);
    BOOST_CHECK_EQUAL(n4, r4n);
    BOOST_CHECK_EQUAL(0, ::memcmp(ip_ref, static_cast<uint8_t*>(r6h), sizeof(in6_addr)));
    BOOST_CHECK_EQUAL(0, ::memcmp(ip_ref, static_cast<uint8_t*>(r6n), sizeof(in6_addr)));
    BOOST_CHECK_EQUAL(0, ::memcmp(ip_ref, static_cast<uint8_t*>(r4h), sizeof(in_addr)));
    BOOST_CHECK_EQUAL(0, ::memcmp(ip_ref, static_cast<uint8_t*>(r4n), sizeof(in_addr)));

  }
}

BOOST_AUTO_TEST_CASE(string_conversions)
{
  ipv6::host ip6_r1 = { 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0xff,0xff, 0xc0,0xa8, 0x03,0x14 };
  ipv6::host ip6_r1_1 = { 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0xef,0xff, 0xc0,0xa8, 0x03,0x14 };
  ipv6::network ip6_r2 = { 16, { 0x20, 0x01 } };
  ipv4::host ip4_r1 = { 192, 168, 3, 20 };
  ipv4::network ip4_r2 = { 24, { 192, 168, 3 } };

  const char* c_i1 = "::ffff:192.168.3.20";
  const char* c_i1_1 = "::efff:c0a8:314";
  const char* c_i2 = "2001::/16";
  const char* c_i3 = "192.168.3.20";
  const char* c_i4 = "192.168.3.0/24";
  const char* c_i2_1 = "2001::";
  const char* c_i4_1 = "192.168.3.0";

  std::string s_i1 = c_i1;
  std::string s_i1_1 = c_i1_1;
  std::string s_i2 = c_i2;
  std::string s_i3 = c_i3;
  std::string s_i4 = c_i4;
  std::string s_i2_1 = c_i2_1;
  std::string s_i4_1 = c_i4_1;

  {  // -- std::string ctor
    {
      BOOST_CHECK_NO_THROW(ipv6::host h1(s_i1));
      BOOST_CHECK_NO_THROW(ipv6::host h1_1(s_i1_1));
      BOOST_CHECK_NO_THROW(ipv6::network n1(s_i2_1));
      BOOST_CHECK_NO_THROW(ipv6::network n2(s_i2));
      BOOST_CHECK_NO_THROW(ipv4::host h2(s_i3));
      BOOST_CHECK_NO_THROW(ipv4::network n3(s_i4_1));
      BOOST_CHECK_NO_THROW(ipv4::network n4(s_i4));
      BOOST_CHECK_THROW(ipv6::network n(std::string("2001::/130")), cool::ng::exception::parsing_error);
      BOOST_CHECK_THROW(ipv4::network n(std::string("192.168.3.0/35")), cool::ng::exception::parsing_error);
    }

    ipv6::host h1(s_i1);
    ipv6::host h1_1(s_i1_1);
    ipv6::network n1(s_i2_1);
    ipv6::network n2(s_i2);
    ipv4::host h2(s_i3);
    ipv4::network n3(s_i4_1);
    ipv4::network n4(s_i4);

    BOOST_CHECK_EQUAL(h1, ip6_r1);
    BOOST_CHECK_EQUAL(h1_1, ip6_r1_1);
    BOOST_CHECK_NE(n1, ip6_r2);
    BOOST_CHECK_EQUAL(n2, ip6_r2);
    BOOST_CHECK_EQUAL(h2, ip4_r1);
    BOOST_CHECK_NE(n3, ip4_r2);
    BOOST_CHECK_EQUAL("0.0.0.0/0", static_cast<std::string>(n3)); // TODO: is this correct?
    BOOST_CHECK_EQUAL(n4, ip4_r2);
  }
  {  // -- const char * ctor
    {
      BOOST_CHECK_NO_THROW(ipv6::host h1(c_i1));
      BOOST_CHECK_NO_THROW(ipv6::host h1_1(c_i1_1));
      BOOST_CHECK_NO_THROW(ipv6::network n1(c_i2_1));
      BOOST_CHECK_NO_THROW(ipv6::network n2(c_i2));
      BOOST_CHECK_NO_THROW(ipv4::host h2(c_i3));
      BOOST_CHECK_NO_THROW(ipv4::network n3(c_i4_1));
      BOOST_CHECK_NO_THROW(ipv4::network n4(c_i4));
      BOOST_CHECK_THROW(ipv6::network n("2001::/130"), cool::ng::exception::parsing_error);
      BOOST_CHECK_THROW(ipv4::network n("192.168.3.0/35"), cool::ng::exception::parsing_error);
    }

    ipv6::host h1(c_i1);
    ipv6::host h1_1(c_i1_1);
    ipv6::network n1(c_i2_1);
    ipv6::network n2(c_i2);
    ipv4::host h2(c_i3);
    ipv4::network n3(c_i4_1);
    ipv4::network n4(c_i4);

    BOOST_CHECK_EQUAL(h1, ip6_r1);
    BOOST_CHECK_EQUAL(h1_1, ip6_r1_1);
    BOOST_CHECK_NE(n1, ip6_r2);
    BOOST_CHECK_EQUAL(n2, ip6_r2);
    BOOST_CHECK_EQUAL(h2, ip4_r1);
    BOOST_CHECK_NE(n3, ip4_r2);
    BOOST_CHECK_EQUAL("0.0.0.0/0", static_cast<std::string>(n3)); // TODO: is this correct?
    BOOST_CHECK_EQUAL(n4, ip4_r2);
  }

  // -- std::string assignment && std::string conversion
  { std::string s;
    {
      ipv6::host a;
      BOOST_CHECK_NO_THROW(a = s_i1);
      s = static_cast<std::string>(a);
      BOOST_CHECK_EQUAL(a, ip6_r1);
      BOOST_CHECK_EQUAL(s, s_i1);
    }
    {
      ipv6::host a;
      BOOST_CHECK_NO_THROW(a = s_i1_1);
      s = static_cast<std::string>(a);
      BOOST_CHECK_EQUAL(a, ip6_r1_1);
      BOOST_CHECK_EQUAL(s, s_i1_1);
    }
    {
      ipv6::network a;
      BOOST_CHECK_NO_THROW(a = s_i2);
      s = static_cast<std::string>(a);
      BOOST_CHECK_EQUAL(a, ip6_r2);
      BOOST_CHECK_EQUAL(s, s_i2);
    }
    {
      ipv4::host a;
      BOOST_CHECK_NO_THROW(a = s_i3);
      s = static_cast<std::string>(a);
      BOOST_CHECK_EQUAL(a, ip4_r1);
      BOOST_CHECK_EQUAL(s, s_i3);
    }
    {
      ipv4::network a;
      BOOST_CHECK_NO_THROW(a = s_i4);
      s = static_cast<std::string>(a);
      BOOST_CHECK_EQUAL(a, ip4_r2);
      BOOST_CHECK_EQUAL(s, s_i4);
    }
    // check that too large mask is detected
    {
      ipv6::network a;
      BOOST_CHECK_THROW(a = std::string("2017::/129"), cool::ng::exception::parsing_error);
    }
    {
      ipv4::network a;
      BOOST_CHECK_THROW(a = std::string("192.168.3.0/33"), cool::ng::exception::parsing_error);
    }
    // check that mask remains unaffected if none specified in the string
     {
      ipv6::network a(16);
      BOOST_CHECK_NO_THROW(a = s_i2_1);
      s = static_cast<std::string>(a);
      BOOST_CHECK_EQUAL(a, ip6_r2);
      BOOST_CHECK_EQUAL(s, s_i2);
    }
    {
      ipv4::network a(24);
      BOOST_CHECK_NO_THROW(a = s_i4_1);
      s = static_cast<std::string>(a);
      BOOST_CHECK_EQUAL(a, ip4_r2);
      BOOST_CHECK_EQUAL(s, s_i4);
    }
  }
  // -- const char* assignment && std::string conversion
  { std::string s;
    {
      ipv6::host a;
      BOOST_CHECK_NO_THROW(a = c_i1);
      s = static_cast<std::string>(a);
      BOOST_CHECK_EQUAL(a, ip6_r1);
      BOOST_CHECK_EQUAL(s, s_i1);
    }
    {
      ipv6::host a;
      BOOST_CHECK_NO_THROW(a = c_i1_1);
      s = static_cast<std::string>(a);
      BOOST_CHECK_EQUAL(a, ip6_r1_1);
      BOOST_CHECK_EQUAL(s, s_i1_1);
    }
    {
      ipv6::network a;
      BOOST_CHECK_NO_THROW(a = c_i2);
      s = static_cast<std::string>(a);
      BOOST_CHECK_EQUAL(a, ip6_r2);
      BOOST_CHECK_EQUAL(s, s_i2);
    }
    {
      ipv4::host a;
      BOOST_CHECK_NO_THROW(a = c_i3);
      s = static_cast<std::string>(a);
      BOOST_CHECK_EQUAL(a, ip4_r1);
      BOOST_CHECK_EQUAL(s, s_i3);
    }
    {
      ipv4::network a;
      BOOST_CHECK_NO_THROW(a = c_i4);
      s = static_cast<std::string>(a);
      BOOST_CHECK_EQUAL(a, ip4_r2);
      BOOST_CHECK_EQUAL(s, s_i4);
    }
    // check that too large mask is detected
    {
      ipv6::network a;
      BOOST_CHECK_THROW(a = "2017::/129", cool::ng::exception::parsing_error);
    }
    {
      ipv4::network a;
      BOOST_CHECK_THROW(a = "192.168.3.0/33", cool::ng::exception::parsing_error);
    }
    // check that mask remains unaffected if none specified in the string
     {
      ipv6::network a(16);
      BOOST_CHECK_NO_THROW(a = c_i2_1);
      s = static_cast<std::string>(a);
      BOOST_CHECK_EQUAL(a, ip6_r2);
      BOOST_CHECK_EQUAL(s, s_i2);
    }
    {
      ipv4::network a(24);
      BOOST_CHECK_NO_THROW(a = c_i4_1);
      s = static_cast<std::string>(a);
      BOOST_CHECK_EQUAL(a, ip4_r2);
      BOOST_CHECK_EQUAL(s, s_i4);
    }
  }
}

BOOST_AUTO_TEST_CASE(ipv6_host)
{
  {
    ipv6::host h1 = { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0xc0, 0xa8, 0xad, 0x16 };
    ipv6::host h2 = { 0x00, 0x00, 0x00, 0x00,     0x08, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0xc0, 0xa8, 0xad, 0x16 };
    ipv6::host h3;
    ipv6::host h4;
    ipv6::network nv6 = { 120, { 0x73, 0x00} };
    ipv4::host hv4 = { 192, 168, 3, 12};
    ipv4::network nv4 = { 8, { 10 }};
    {
      std::stringstream ss; ss << h1; ss >> h3;
    }
    {
      std::stringstream ss; ss << h2; ss >> h4;
    }
    BOOST_CHECK_NE(h1, h2);
    BOOST_CHECK_EQUAL(h1, h3);
    BOOST_CHECK_NE(h1, h4);
    BOOST_CHECK_NE(h2, h3);
    BOOST_CHECK_EQUAL(h2, h4);
    BOOST_CHECK_NE(h3, h4);
    BOOST_CHECK_NE(h1, h2);
    BOOST_CHECK_EQUAL(h1, h3);
    BOOST_CHECK_NE(h1, h4);
    BOOST_CHECK_NE(h2, h3);
    BOOST_CHECK_EQUAL(h2, h4);
    BOOST_CHECK_NE(h3, h4);
    BOOST_CHECK_NE(h1, nv6);
    BOOST_CHECK_NE(h1, hv4);
    BOOST_CHECK_NE(h1, nv4);
  }
  {
    ipv6::host hv6_1 = { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0xc0, 0xa8, 0xad, 0x16 };
    ipv6::host hv6_2 = { 0x00, 0x00, 0x00, 0x03,     0x08, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0xc0, 0xa8, 0xad, 0x16 };
    ipv6::host hv6_3 = { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     192, 168, 3, 12 };
    ipv6::host hv6;
    ipv6::network nv6_1 = { 120, { 0x73, 0x00} };
    ipv4::host hv4_1 = { 192, 168, 3, 12};
    ipv4::network nv4_1 = { 8, { 10 }};
    ipv6::network nv6;
    ipv4::host hv4;
    ipv4::network nv4;

    BOOST_CHECK_NO_THROW(hv6 = hv4_1);
    BOOST_CHECK_EQUAL(hv6, hv6_3);
    BOOST_CHECK_NO_THROW(hv4 = hv6_3);
    BOOST_CHECK_EQUAL(hv4, hv6_3);
    BOOST_CHECK_EQUAL(hv6_3, hv4);
    BOOST_CHECK_THROW(hv4 = hv6_2, cool::ng::exception::illegal_argument);
   }
}

BOOST_AUTO_TEST_CASE(ip_ownership)
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


