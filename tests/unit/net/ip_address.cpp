
#include <stdint.h>
#include <iostream>
#include <sstream>

#define BOOST_TEST_MODULE IpAddress
#include <boost/test/unit_test.hpp>

#include "cool/ng/ip_address.h"

using namespace cool::ng::net;

cool::ng::net::ipv6::host ip6_host_examples[] = {
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
//  BOOST_CHECK_EQUAL("200::1223:3456:789a:bcde:f001", str(addr, ipv6::StrictCanonical));
  BOOST_CHECK_EQUAL("200:0:0:1223:3456:789a:bcde:f001", str(addr, ipv6::Expanded));
  BOOST_CHECK_EQUAL("200--1223-3456-789a-bcde-f001", str(addr, ipv6::Microsoft));
  BOOST_CHECK_EQUAL("200::1223:3456:789a:188.222.240.1", str(addr, ipv6::DottedQuad));

  addr = ip6_host_examples[1];
  BOOST_CHECK_EQUAL("203::1223:3400:0:0:f001", str(addr, ipv6::Canonical));
//  BOOST_CHECK_EQUAL("203::1223:3400:0:0:f001", str(addr, ipv6::StrictCanonical));
  BOOST_CHECK_EQUAL("203:0:0:1223:3400:0:0:f001", str(addr, ipv6::Expanded));
  BOOST_CHECK_EQUAL("203--1223-3400-0-0-f001", str(addr, ipv6::Microsoft));
  BOOST_CHECK_EQUAL("203::1223:3400:0:0.0.240.1", str(addr, ipv6::DottedQuad));

  addr = ip6_host_examples[2];
  BOOST_CHECK_EQUAL("ab03:0:0:1234:5678::", str(addr, ipv6::Canonical));
//  BOOST_CHECK_EQUAL("ab03:0:0:1234:5678::", str(addr, ipv6::StrictCanonical));
  BOOST_CHECK_EQUAL("ab03:0:0:1234:5678:0:0:0", str(addr, ipv6::Expanded));
  BOOST_CHECK_EQUAL("ab03-0-0-1234-5678--", str(addr, ipv6::Microsoft));
  BOOST_CHECK_EQUAL("ab03::1234:5678:0:0.0.0.0", str(addr, ipv6::DottedQuad));

  addr = ip6_host_examples[3];
  BOOST_CHECK_EQUAL("ab03::1234:5678:1:0:0", str(addr, ipv6::Canonical));
//  BOOST_CHECK_EQUAL("ab03::1234:5678:1:0:0", str(addr, ipv6::StrictCanonical));
  BOOST_CHECK_EQUAL("ab03:0:0:1234:5678:1:0:0", str(addr, ipv6::Expanded));
  BOOST_CHECK_EQUAL("ab03--1234-5678-1-0-0", str(addr, ipv6::Microsoft));
  BOOST_CHECK_EQUAL("ab03::1234:5678:1:0.0.0.0", str(addr, ipv6::DottedQuad));

  addr = ip6_host_examples[4];
  BOOST_CHECK_EQUAL("::ffff:0:a0b:c0d", str(addr, ipv6::Canonical));
//  BOOST_CHECK_EQUAL("::ffff:0:a0b:c0d", str(addr, ipv6::StrictCanonical));
  BOOST_CHECK_EQUAL("0:0:0:0:ffff:0:a0b:c0d", str(addr, ipv6::Expanded));
  BOOST_CHECK_EQUAL("--ffff-0-a0b-c0d", str(addr, ipv6::Microsoft));
  BOOST_CHECK_EQUAL("::ffff:0:10.11.12.13", str(addr, ipv6::DottedQuad));

  addr = ip6_host_examples[5];
  BOOST_CHECK_EQUAL("::ffff:10.11.12.13", str(addr, ipv6::Canonical));
//  BOOST_CHECK_EQUAL("::ffff:a0b:c0d", str(addr, ipv6::StrictCanonical));
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
//    std::stringstream ss(ip6_host_input[8]);
//    ss >> addr;
//    BOOST_CHECK_EQUAL("1::", static_cast<std::string>(addr));
  }
  {
//    std::stringstream ss(ip6_host_input[9]);
//    ss >> addr;
//    BOOST_CHECK_EQUAL("1::", static_cast<std::string>(addr));
  }
  {
//    std::stringstream ss(ip6_host_input[10]);
//    ss >> addr;
//    BOOST_CHECK_EQUAL("::", static_cast<std::string>(addr));
  }
  {
//    std::stringstream ss(ip6_host_input[11]);
//    ss >> addr;
//    BOOST_CHECK_EQUAL("::", static_cast<std::string>(addr));
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
//    std::stringstream ss(ip6_host_input[14]);
//    ss >> addr;
//    BOOST_CHECK_EQUAL("22605:2700:abcd::", static_cast<std::string>(addr));
  }
}
BOOST_AUTO_TEST_SUITE_END()


#if 0
using cool::ng::net::ip::host;
using cool::ng::net::ip::IPv6;

cool::ng::net::ipv6::host ref =   { 0x02, 0x00, 0x00, 0x00,/**/ 0x00, 0x00, 0x12, 0x23,/**/ 0x34, 0x56, 0x78, 0x9a,/**/ 0xbc, 0xde, 0xf0, 0x01 };

const char* ip6expect[] = {
  "200::1223:3456:789a:bcde:f001",
  "203::1223:3400:0:0:f001",
  "ab03:0:0:1234:5678::",
  "ab03::1234:5678:1:0:0",
  "::ffff:0:a0b:c0d",
  "::ffff:10.11.12.13",
  ""
};

cool::ng::net::ipv6::host results[] = {
  cool::ng::net::ipv6::loopback,
  cool::ng::net::ipv6::loopback,
  { 0x26, 0x05, 0x27, 0x00,/**/ 0x00, 0x00, 0x00, 0x03,/**/ 0x00, 0x00, 0x00, 0x00,/**/ 0x47, 0x13, 0x93, 0xe3 },
  { 0x26, 0x05, 0x27, 0x00,/**/ 0x00, 0x00, 0x00, 0x03,/**/ 0x00, 0x00, 0x00, 0x00,/**/ 0x47, 0x13, 0x93, 0xe3 },
  { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0xc0, 0xa8, 0xad, 0x16 },
  { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0xc0, 0xa8, 0xad, 0x16 },
  { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0xc0, 0xa8, 0xad, 0x16 },
  { 0xff, 0xff, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0xc0, 0xa8, 0xad, 0x16 },
  { 0x00, 0x01, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00 },
  { 0x00, 0x01, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00 },
  cool::ng::net::ipv6::unspecified,
  cool::ng::net::ipv6::unspecified,
  { 0x26, 0x05, 0x27, 0x00,/**/ 0x00, 0x00, 0x00, 0x03,/**/ 0x00, 0x00, 0x00, 0x00,/**/ 0x47, 0x13, 0x93, 0xe3 },
  { 0x26, 0x05, 0x27, 0x00,/**/ 0x00, 0x00, 0x00, 0x03,/**/ 0x00, 0x00, 0x00, 0x00,/**/ 0x47, 0x13, 0x93, 0xe3 },

};

const char* ip6input[] = {
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
  nullptr
};

void first_simple_test_ref()
{
  std::cout << "============================ " << __FUNCTION__ << std::endl;

  for (int i = 0; i < sizeof(ip6examples) / sizeof(ip6examples[0]); ++i)
  {
    cool::ng::net::ip::host& addr = ip6examples[i];
    std::cout << std::endl << "EXPECT: " << ip6expect[i] << std::endl;
    std::cout << "        " << addr << std::endl;
  }

  std::cout << "===========================================\n";
  cool::ng::net::ipv6::host res;
  for (int i = 0; ip6input[i] != nullptr; ++i)
  {
    std::stringstream ss(ip6input[i]);
    ss >> res;
    std::cout << "Input: " << ip6input[i] << std::endl;

    cool::ng::net::ip::address& ref = res;
    if (ref == results[i])
      std::cout << "OK   : " << ref << std::endl;
    else
      std::cout << "Fail : got " << ref << " expected " << results[i] << std::endl;
  }

  std::cout << "===========================================\n";

  std::cout << "Q    " << ip6examples[4] << " in " << cool::ng::net::ipv6::rfc_ipv4map << " ?" << std::endl;
  std::cout << "A    " << (cool::ng::net::ipv6::rfc_ipv4map.has(ip6examples[4]) ? "Yes" : "No") << std::endl;
  std::cout << "Q    " << ip6examples[5] << " in " << cool::ng::net::ipv6::rfc_ipv4map << " ?" << std::endl;
  std::cout << "A    " << (cool::ng::net::ipv6::rfc_ipv4map.has(ip6examples[5]) ? "Yes" : "No") << std::endl;
}

void first_simple_test()
{
  std::cout << "============================ " << __FUNCTION__ << std::endl;
  std::cout << "IPv6 loopback:      " << cool::ng::net::ipv6::loopback << std::endl;
  std::cout << "IPv6 unspecified:   " << cool::ng::net::ipv6::unspecified << std::endl;
  std::cout << "IPv6 mapped prefix: " << cool::ng::net::ipv6::rfc_ipv4map << std::endl;
  std::cout << "IPv4 loopback:      " << cool::ng::net::ipv4::loopback << std::endl;
  std::cout << "IPv4 unspecified:   " << cool::ng::net::ipv4::any << std::endl;

  std::cout << "===========================================\n";

  for (int i = 0; i < sizeof(ip6examples) / sizeof(ip6examples[0]); ++i)
  {
    cool::ng::net::ipv6::host addr(ip6examples[i]);
    std::cout << "FULL  : ";
    addr.visualize(std::cout, cool::ng::net::ipv6::Expanded);
    std::cout << std::endl << "EXPECT: " << ip6expect[i] << std::endl;
    std::cout << "        " << addr << std::endl;
    std::cout << "        ";
    addr.visualize(std::cout, cool::ng::net::ipv6::Microsoft);
    std::cout << std::endl;
  }

  std::cout << "===========================================\n";
  cool::ng::net::ipv6::host res;
  for (int i = 0; ip6input[i] != nullptr; ++i)
  {
    std::stringstream ss(ip6input[i]);
    ss >> res;
    std::cout << "Input: " << ip6input[i] << std::endl;
    if (res == results[i])
      std::cout << "OK   : " << res << std::endl;
    else
      std::cout << "Fail : got " << res << " expected " << results[i] << std::endl;
  }

  std::cout << "===========================================\n";

  std::cout << "Q    " << ip6examples[4] << " in " << cool::ng::net::ipv6::rfc_ipv4map << " ?" << std::endl;
  std::cout << "A    " << (cool::ng::net::ipv6::rfc_ipv4map.has(ip6examples[4]) ? "Yes" : "No") << std::endl;
  std::cout << "Q    " << ip6examples[5] << " in " << cool::ng::net::ipv6::rfc_ipv4map << " ?" << std::endl;
  std::cout << "A    " << (cool::ng::net::ipv6::rfc_ipv4map.has(ip6examples[5]) ? "Yes" : "No") << std::endl;
}

#define YESNO(a) ((a) ? "Yes " : "No  ")
#define SEQ(a,b,res) std::cout << "EQ:     " << (res == (a == b) ? "OK  " : "Fail") \
           << " - " << YESNO(a == b) << (a) << " == " << (b) << "?" << std::endl
#define SNEQ(a,b,res) std::cout << "NEQ:    " << (res == (a != b) ? "OK  " : "Fail") \
           << " - " << YESNO(a != b) << (a) << " != " << (b) << "?" << std::endl
#define EXCEPT(x, a,b) try { (a); std::cout << x << "Fail - exception expected: " << #b << std::endl; } \
                       catch (const b&) { std::cout << x << "OK   - got expected exception " << #b << std::endl; } \
                       catch (...) { std::cout << x << "Fail - got unexpected exception\n"; }
#define NEXCEPT(x, a) try { (a); std::cout << x << "OK   - no exception expected\n"; } \
                      catch (...) { std::cout << x << "Fail - got unexpected exception\n"; }

#define CMP( a, b, c, res) std::cout << "CMP:    " \
     << ((::memcmp(a, b, c) == 0) == res ? "OK  " : "Fail") << " - memory compare\n"

void ip_conversions()
{
  std::cout << "============================ " << __FUNCTION__ << std::endl;

  // ------------ struct in_addr/in6_addr ctor, assignment and type conversions
  {
    std::cout << "------ struct in_addr/in6_addr assignment and type conversion\n";

    cool::ng::net::ipv6::host ip6_r1 = { 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0xff,0xff, 0xc0,0xa8, 0x03,0x14 };
    cool::ng::net::ipv6::host ip6_r2 = { 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0xef,0xff, 0xc0,0xa8, 0x03,0x14 };
    cool::ng::net::ipv4::host ip4_r1 = { 192, 168, 3, 20 };
    cool::ng::net::ipv6::network ip6_r3 = { 96, { 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0xff,0xff} };
    cool::ng::net::ipv4::network ip4_r2 = { 24, { 192, 168, 3, 0 } };
    struct sockaddr_in6 ip6_ref;
    struct sockaddr_in6 ip6_ref_2;
    struct sockaddr_in  ip4_ref;
    inet_pton(AF_INET6, "::ffff:c0a8:314", &(ip6_ref.sin6_addr));
    inet_pton(AF_INET6, "::efff:c0a8:314", &(ip6_ref_2.sin6_addr));
    inet_pton(AF_INET, "192.168.3.20", &(ip4_ref.sin_addr));

    { // ctors
      struct sockaddr_in6 ip6_ref_3;
      inet_pton(AF_INET6, "2001::", &(ip6_ref_3.sin6_addr));

      cool::ng::net::ipv6::host h1(ip6_ref_2.sin6_addr);
      cool::ng::net::ipv6::host h2(ip4_ref.sin_addr);
      cool::ng::net::ipv6::network n1(96, ip6_ref.sin6_addr);
      cool::ng::net::ipv4::host h3(ip4_ref.sin_addr);
      cool::ng::net::ipv4::host h4(ip6_ref.sin6_addr);
      cool::ng::net::ipv4::network n2(24, ip4_ref.sin_addr);
      SEQ(h1, ip6_r2, true);
      SEQ(h2, ip6_r1, true);
      SEQ(n1, ip6_r3, true);
      SEQ(h3, ip4_r1, true);
      SEQ(h4, ip4_r1, true);
      SEQ(n2, ip4_r2, true);
      try { cool::ng::net::ipv4::host h(ip6_ref_2.sin6_addr);
            std::cout << "        Fail   - did not get expected exception\n"; }
      catch (const cool::ng::exception::illegal_argument&)
      { std::cout << "        OK   - got expected exception\n"; }
      catch (...)
      { std::cout << "        Fail   - got unexpected exception\n"; }
    }

    struct sockaddr_in6 ip6_aux;
    struct sockaddr_in ip4_aux;

    {   // ipv6::host
      cool::ng::net::ipv6::host ip6_1;
      ::memset(&ip6_aux, 0, sizeof(ip6_aux));
      ::memset(&ip4_aux, 0, sizeof(ip4_aux));

      NEXCEPT("1INADDR ", ip6_1 = ip6_ref.sin6_addr);
      SEQ(ip6_1, ip6_r1, true);
      ip6_1 = cool::ng::net::ipv6::loopback;
      NEXCEPT("1INADDR ", ip6_1 = ip4_ref.sin_addr);
      SEQ(ip6_1, ip6_r1, true);

      NEXCEPT("1INADDR ", ip6_aux.sin6_addr = ip6_r1);
      CMP(&ip6_aux.sin6_addr, &ip6_ref.sin6_addr, 16, true);
      NEXCEPT("1INADDR ", ip4_aux.sin_addr = ip6_r1);
      CMP(&ip4_aux.sin_addr, &ip4_ref.sin_addr, 4, true);
    }
    { // ipv4::host
      cool::ng::net::ipv4::host ip4_1;
      ::memset(&ip6_aux, 0, sizeof(ip6_aux));
      ::memset(&ip4_aux, 0, sizeof(ip4_aux));

      NEXCEPT("2INADDR ", ip4_1 = ip4_ref.sin_addr);
      SEQ(ip4_1, ip4_r1, true);
      ip4_1 = cool::ng::net::ipv4::loopback;
      NEXCEPT("2INADDR ", ip4_1 = ip6_ref.sin6_addr);
      SEQ(ip4_1, ip4_r1, true);
      EXCEPT("2INADDR ", ip4_1 = ip6_ref_2.sin6_addr, cool::ng::exception::illegal_argument);

      NEXCEPT("2INADDR ", ip4_aux.sin_addr = ip4_r1);
      CMP(&ip4_aux.sin_addr, &ip4_ref.sin_addr, 4, true);
      NEXCEPT("2INADDR ", ip6_aux.sin6_addr = ip4_r1);
      CMP(&ip6_aux.sin6_addr, &ip6_ref.sin6_addr, 16, true);
    }
  }

  {
    cool::ng::net::ipv6::network ip6_r1 = { 112, { 0x20, 0x01 } };
    cool::ng::net::ipv4::network ip4_r1 = { 24, { 192, 168, 3, 0 } };

    struct sockaddr_in6 ip6_ref;
    struct sockaddr_in  ip4_ref;
    inet_pton(AF_INET6, "2001::", &(ip6_ref.sin6_addr));
    inet_pton(AF_INET, "192.168.3.0", &(ip4_ref.sin_addr));

    struct sockaddr_in6 ip6_aux;
    struct sockaddr_in ip4_aux;

    {  // IPv6 network
      cool::ng::net::ipv6::network ip6_1 = { 112, { 0 } };
      cool::ng::net::ip::address& ref = ip6_1;

      ::memset(&ip6_aux, 0, sizeof(ip6_aux));
      ::memset(&ip4_aux, 0, sizeof(ip4_aux));

      NEXCEPT("3INADDR ", ip6_1 = ip6_ref.sin6_addr);
      SEQ(ip6_1, ip6_r1, true);
      EXCEPT("3INADDR ", ref = ip4_ref.sin_addr, cool::ng::exception::unsupported_operation);

      NEXCEPT("3INADDR ", ip6_aux.sin6_addr = ip6_r1);
      CMP(&ip6_aux.sin6_addr, &ip6_ref.sin6_addr, 16, true);
      EXCEPT("3INADDR ", ip4_aux.sin_addr = static_cast<in_addr>(ref), cool::ng::exception::unsupported_operation);
    }
    {  // IPv4 network
      cool::ng::net::ipv4::network ip4_1 = { 24, { 0 } };
      cool::ng::net::ip::address& ref = ip4_1;

      ::memset(&ip6_aux, 0, sizeof(ip6_aux));
      ::memset(&ip4_aux, 0, sizeof(ip4_aux));

      NEXCEPT("4INADDR ", ip4_1 = ip4_ref.sin_addr);
      SEQ(ip4_1, ip4_r1, true);
      EXCEPT("4INADDR ", ref = ip6_ref.sin6_addr, cool::ng::exception::unsupported_operation);

      NEXCEPT("4INADDR ", ip4_aux.sin_addr = ip4_r1);
      CMP(&ip4_aux.sin_addr, &ip4_ref.sin_addr, 4, true);
      EXCEPT("4INADDR ", ip6_aux.sin6_addr = static_cast<in6_addr>(ref), cool::ng::exception::unsupported_operation);
    }
  }
  {
    std::cout << "------ uint8_t* ctor, assignment and type conversion\n";

    uint8_t ip_ref[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10 };
    cool::ng::net::ipv6::host r6h = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10 };
    cool::ng::net::ipv6::network r6n = { 128, { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10 } };
    cool::ng::net::ipv4::host r4h = { 0x01, 0x02, 0x03, 0x04 };
    cool::ng::net::ipv4::network r4n= { 32, { 0x01, 0x02, 0x03, 0x04 } };

    { // ctor
      cool::ng::net::ipv6::host h6(ip_ref);
      cool::ng::net::ipv6::network n6(128, ip_ref);
      cool::ng::net::ipv4::host h4(ip_ref);
      cool::ng::net::ipv4::network n4(32, ip_ref);

      SEQ(h6, r6h, true);
      SEQ(n6, r6n, true);
      SEQ(h4, r4h, true);
      SEQ(n4, r4n, true);
    }
    { // assignment
      cool::ng::net::ipv6::host h6;
      cool::ng::net::ipv6::network n6(128);
      cool::ng::net::ipv4::host h4;
      cool::ng::net::ipv4::network n4(32);

      h6 = ip_ref;
      n6 = ip_ref;
      h4 = ip_ref;
      n4 = ip_ref;

      SEQ(h6, r6h, true);
      SEQ(n6, r6n, true);
      SEQ(h4, r4h, true);
      SEQ(n4, r4n, true);
    }
    { // type conversion
      cool::ng::net::ipv6::host h6;
      cool::ng::net::ipv6::network n6(128);
      cool::ng::net::ipv4::host h4;
      cool::ng::net::ipv4::network n4(32);

      CMP(ip_ref, r6h, 16, true);
      CMP(ip_ref, r6n, 16, true);
      CMP(ip_ref, r4h, 4, true);
      CMP(ip_ref, r4n, 4, true);

      ::memcpy(h6, ip_ref, 16);
      ::memcpy(n6, ip_ref, 16);
      ::memcpy(h4, ip_ref, 4);
      ::memcpy(n4, ip_ref, 4);

      SEQ(h6, r6h, true);
      SEQ(n6, r6n, true);
      SEQ(h4, r4h, true);
      SEQ(n4, r4n, true);
      CMP(ip_ref, h6, 16, true);
      CMP(ip_ref, n6, 16, true);
      CMP(ip_ref, h4, 4, true);
      CMP(ip_ref, n4, 4, true);
    }
  }
  {  std::cout << "------ std::string ctor, assignment and type conversion\n";
    cool::ng::net::ipv6::host ip6_r1 = { 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0xff,0xff, 0xc0,0xa8, 0x03,0x14 };
    cool::ng::net::ipv6::host ip6_r1_1 = { 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0xef,0xff, 0xc0,0xa8, 0x03,0x14 };
    cool::ng::net::ipv6::network ip6_r2 = { 16, { 0x20, 0x01 } };
    cool::ng::net::ipv4::host ip4_r1 = { 192, 168, 3, 20 };
    cool::ng::net::ipv4::network ip4_r2 = { 24, { 192, 168, 3 } };

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

    { // ctor using std::string
      std::cout << "-------- ctor std::string\n";
      cool::ng::net::ipv6::host h1(s_i1);
      SEQ(h1, ip6_r1, true);
      cool::ng::net::ipv6::host h1_1(s_i1_1);
      SEQ(h1_1, ip6_r1_1, true);
      cool::ng::net::ipv6::network n1(s_i2_1);
      SNEQ(n1, ip6_r2, true);
      cool::ng::net::ipv6::network n2(s_i2);
      SEQ(n2, ip6_r2, true);
      cool::ng::net::ipv4::host h2(s_i3);
      SEQ(h2, ip4_r1, true);
      cool::ng::net::ipv4::network n3(s_i4_1);
      SNEQ(n3, ip4_r2, true);
      cool::ng::net::ipv4::network n4(s_i4);
      SEQ(n4, ip4_r2, true);
      try { cool::ng::net::ipv6::network("2001::/130");
            std::cout << "        Fail   - did not get expected exception\n"; }
      catch (const cool::ng::exception::illegal_argument&)
         { std::cout << "        OK   - got expected exception\n"; }
      catch (...)
         { std::cout << "        Fail   - got unexpected exception\n"; }
      try { cool::ng::net::ipv4::network("192.168.3.0/35");
            std::cout << "        Fail   - did not get expected exception\n"; }
      catch (const cool::ng::exception::illegal_argument&)
        { std::cout << "        OK   - got expected exception\n"; }
      catch (...)
        { std::cout << "        Fail   - got unexpected exception\n"; }
    }
    { // ctor using const char *
      std::cout << "-------- ctor const char*\n";
      cool::ng::net::ipv6::host h1(c_i1);
      SEQ(h1, ip6_r1, true);
      cool::ng::net::ipv6::host h1_1(c_i1_1);
      SEQ(h1_1, ip6_r1_1, true);
      cool::ng::net::ipv6::network n1(c_i2_1);
      SNEQ(n1, ip6_r2, true);
      cool::ng::net::ipv6::network n2(c_i2);
      SEQ(n2, ip6_r2, true);
      cool::ng::net::ipv4::host h2(c_i3);
      SEQ(h2, ip4_r1, true);
      cool::ng::net::ipv4::network n3(c_i4_1);
      SNEQ(n3, ip4_r2, true);
      cool::ng::net::ipv4::network n4(c_i4);
      SEQ(n4, ip4_r2, true);
    }

    std::cout << "-------- assign std::string\n";
    { // std::string assignment and std::string conversion
      std::stringstream ss;
      {
        cool::ng::net::ipv6::host a;
        a = s_i1;
        std::string s;
        s = a;
        SEQ(a, ip6_r1, true);
        SEQ(s, s_i1, true);
      }
      {
        cool::ng::net::ipv6::host a;
        a = s_i1_1;
        std::string s;
        s = a;
        SEQ(a, ip6_r1_1, true);
        SEQ(s, s_i1_1, true);
      }
      {
        cool::ng::net::ipv6::network a;
        a = s_i2;
        std::string s; s = a;
        SEQ(a, ip6_r2, true);
        SEQ(s, s_i2, true);
      }
      {
        cool::ng::net::ipv4::host a ;
        a = s_i3;
        std::string s; s = a;
        SEQ(a, ip4_r1, true);
        SEQ(s, s_i3, true);
      }
      {
        cool::ng::net::ipv4::network a;
        a = s_i4;
        std::string s; s = a;
        SEQ(a, ip4_r2, true);
        SEQ(s, s_i4, true);
      }
      // check that network mask remains untouched if not present in string
      {
        cool::ng::net::ipv6::network a(16);
        a = s_i2_1;
        std::string s; s = a;
        SEQ(a, ip6_r2, true);
        SEQ(s, s_i2, true);
      }
      {
        cool::ng::net::ipv4::network a(24);
        a = s_i4_1;
        std::string s; s = a;
        SEQ(a, ip4_r2, true);
        SEQ(s, s_i4, true);
      }
    }
    std::cout << "-------- assign const char *\n";
    { // const char ( assignment
      {
        cool::ng::net::ipv6::host a; a = c_i1;
        std::string s; s = a;
        SEQ(a, ip6_r1, true);
        SEQ(s, s_i1, true);
      }
      {
        cool::ng::net::ipv6::host a; a = c_i1_1;
        std::string s; s = a;
        SEQ(a, ip6_r1_1, true);
        SEQ(s, s_i1_1, true);
      }
      {
        cool::ng::net::ipv6::network a; a = c_i2;
        std::string s; s = a;
        SEQ(a, ip6_r2, true);
        SEQ(s, s_i2, true);
      }
      {
        cool::ng::net::ipv4::host a; a = c_i3;
        std::string s; s = a;
        SEQ(a, ip4_r1, true);
        SEQ(s, s_i3, true);
      }
      {
        cool::ng::net::ipv4::network a;
        a = c_i4;
        std::string s; s = a;
        SEQ(a, ip4_r2, true);
        SEQ(s, s_i4, true);
      }
    }
  }
}

void ipv6_host()
{
  std::cout << "============================ " << __FUNCTION__ << std::endl;

  {
    cool::ng::net::ipv6::host h1 = { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0xc0, 0xa8, 0xad, 0x16 };
    cool::ng::net::ipv6::host h2 = { 0x00, 0x00, 0x00, 0x00,     0x08, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0xc0, 0xa8, 0xad, 0x16 };
    cool::ng::net::ipv6::host h3;
    cool::ng::net::ipv6::host h4;
    cool::ng::net::ipv6::network nv6 = { 120, { 0x73, 0x00} };
    cool::ng::net::ipv4::host hv4 = { 192, 168, 3, 12};
    cool::ng::net::ipv4::network nv4 = { 8, { 10 }};
    {
      std::stringstream ss; ss << h1; ss >> h3;
    }
    {
      std::stringstream ss; ss << h2; ss >> h4;
    }
    SEQ(h1, h2, false);
    SEQ(h1, h3, true);
    SEQ(h1, h4, false);
    SEQ(h2, h3, false);
    SEQ(h2, h4, true);
    SEQ(h3, h4, false);
    SNEQ(h1, h2, true);
    SNEQ(h1, h3, false);
    SNEQ(h1, h4, true);
    SNEQ(h2, h3, true);
    SNEQ(h2, h4, false);
    SNEQ(h3, h4, true);
    SEQ(h1, nv6, false);
    SEQ(h1, hv4, false);
    SEQ(h1, nv4, false);
  }
  {
    cool::ng::net::ipv6::host hv6_1 = { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0xc0, 0xa8, 0xad, 0x16 };
    cool::ng::net::ipv6::host hv6_2 = { 0x00, 0x00, 0x00, 0x03,     0x08, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0xc0, 0xa8, 0xad, 0x16 };
    cool::ng::net::ipv6::host hv6_3 = { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     192, 168, 3, 12 };
    cool::ng::net::ipv6::host hv6;
    cool::ng::net::ipv6::network nv6_1 = { 120, { 0x73, 0x00} };
    cool::ng::net::ipv4::host hv4_1 = { 192, 168, 3, 12};
    cool::ng::net::ipv4::network nv4_1 = { 8, { 10 }};
    cool::ng::net::ipv6::network nv6;
    cool::ng::net::ipv4::host hv4;
    cool::ng::net::ipv4::network nv4;

    NEXCEPT("=       ", hv6 = hv4_1)
    SEQ(hv6, hv6_3, true);
    NEXCEPT("=       ", hv4 = hv6_3)
    SEQ(hv4, hv6_3, true);
    SEQ(hv6_3, hv4, true);
     EXCEPT("=       ", hv4 = hv6_2, cool::ng::exception::illegal_argument)
   }
}
void ipv6_host_ref()
{
  std::cout << "============================ " << __FUNCTION__ << std::endl;

  {
    cool::ng::net::ipv6::host h1 = { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0xc0, 0xa8, 0xad, 0x16 };
    cool::ng::net::ipv6::host h2 = { 0x00, 0x00, 0x00, 0x00,     0x08, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0xc0, 0xa8, 0xad, 0x16 };
    cool::ng::net::ipv6::host h3;
    cool::ng::net::ipv6::host h4;
    cool::ng::net::ipv6::network nv6 = { 120, { 0x73, 0x00} };
    cool::ng::net::ipv4::host hv4 = { 192, 168, 3, 12};
    cool::ng::net::ipv4::network nv4 = { 8, { 10 }};

    cool::ng::net::ip::address& rh1 = h1;
    cool::ng::net::ip::address& rh2 = h2;
    cool::ng::net::ip::address& rh3 = h3;
    cool::ng::net::ip::address& rh4 = h4;
    cool::ng::net::ip::address& rnv6 = nv6;
    cool::ng::net::ip::address& rnv4 = nv4;
    cool::ng::net::ip::address& rhv4 = hv4;;
    {
      std::stringstream ss; ss << rh1; ss >> h3;
    }
    {
      std::stringstream ss; ss << rh2; ss >> h4;
    }
    SEQ(rh1, rh2, false);
    SEQ(rh1, rh3, true);
    SEQ(rh1, rh4, false);
    SEQ(rh2, rh3, false);
    SEQ(rh2, rh4, true);
    SEQ(rh3, rh4, false);
    SNEQ(rh1, rh2, true);
    SNEQ(rh1, rh3, false);
    SNEQ(rh1, rh4, true);
    SNEQ(rh2, rh3, true);
    SNEQ(rh2, rh4, false);
    SNEQ(rh3, rh4, true);
    SEQ(rh1, rnv6, false);
    SEQ(rh1, rhv4, false);
    SEQ(rh1, rnv4, false);
  }
  {
    cool::ng::net::ipv6::host hv6_1 = { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0xc0, 0xa8, 0xad, 0x16 };
    cool::ng::net::ipv6::host hv6_2 = { 0x00, 0x00, 0x00, 0x03,     0x08, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     0xc0, 0xa8, 0xad, 0x16 };
    cool::ng::net::ipv6::host hv6_3 = { 0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0x00, 0x00,     0x00, 0x00, 0xff, 0xff,     192, 168, 3, 12 };
    cool::ng::net::ipv6::host hv6;
    cool::ng::net::ipv6::network nv6_1 = { 120, { 0x73, 0x00} };
    cool::ng::net::ipv4::host hv4_1 = { 192, 168, 3, 12};
    cool::ng::net::ipv4::network nv4_1 = { 8, { 10 }};
    cool::ng::net::ipv6::network nv6;
    cool::ng::net::ipv4::host hv4;
    cool::ng::net::ipv4::network nv4;

    cool::ng::net::ip::address& rhv6 = hv6;
    cool::ng::net::ip::address& rhv4_1 = hv4_1;

    NEXCEPT("=       ", rhv6 = rhv4_1)
    SEQ(hv6, hv6_3, true);
    NEXCEPT("=       ", hv4 = hv6_3)
    SEQ(hv4, hv6_3, true);
    SEQ(hv6_3, hv4, true);
     EXCEPT("=       ", hv4 = hv6_2, cool::ng::exception::illegal_argument)
   }
}

#define IN(a, b, res) \
  std::cout << "IN      " << (a.in(b) == res ? "OK  " : "Fail") \
            << " - Claim: " << a << (res ? " IS IN " : " IS NOT IN ") << b << "\n" \
            << "HAS     " << (b.has(a) == res ? "OK  " : "Fail") \
            << " - Claim: " << b << (res ? " HAS " : " DOES NOT HAVE ") << a << "\n"

void ip_ownership()
{
  std::cout << "============================ " << __FUNCTION__ << std::endl;

  cool::ng::net::ipv6::network net6_1("2001:ab33::/32");
  cool::ng::net::ipv6::network net6_2("2001:ab00::/24");
  cool::ng::net::ipv6::network net6_3("2001:ab44::/32");
  cool::ng::net::ipv6::host host6_1("2001:ab33::20");
  cool::ng::net::ipv6::host host6_2("2001:ab44::21");
  cool::ng::net::ipv6::host host6_3("2001:ab13::21");
  cool::ng::net::ipv6::host host6_4("::ffff:192.168.3.40");

  cool::ng::net::ipv4::network net4_1("10.60.0.0/16");
  cool::ng::net::ipv4::network net4_2("10.0.0.0/8");
  cool::ng::net::ipv4::network net4_3("10.62.0.0/16");
  cool::ng::net::ipv4::host host4_1("10.60.1.12");
  cool::ng::net::ipv4::host host4_2("10.62.55.42");
  cool::ng::net::ipv4::host host4_3("10.42.66.99");
  cool::ng::net::ipv4::host host4_4("192.168.3.44");

  IN(net6_1, net6_2, true);
  IN(net6_3, net6_2, true);
  IN(net6_2, net6_3, false);
  IN(net6_2, net6_1, false);
  IN(net6_2, net4_3, false);
  IN(net4_3, net6_2, false);

  IN(host6_1, net6_1, true);
  IN(host6_1, net6_2, true);
  IN(host6_1, net6_3, false);
  IN(host6_2, net6_1, false);
  IN(host6_2, net6_2, true);
  IN(host6_2, net6_3, true);
  IN(host6_3, net6_1, false);
  IN(host6_3, net6_2, true);
  IN(host6_3, net6_3, false);
  IN(host6_4, net6_1, false);
  IN(host6_4, net6_2, false);
  IN(host6_4, net6_3, false);
  IN(net4_1, net4_2, true);
  IN(net4_3, net4_2, true);
  IN(net4_2, net4_3, false);
  IN(net4_2, net4_1, false);

  IN(host4_1, net4_1, true);
  IN(host4_1, net4_2, true);
  IN(host4_1, net4_3, false);
  IN(host4_2, net4_1, false);
  IN(host4_2, net4_2, true);
  IN(host4_2, net4_3, true);
  IN(host4_3, net4_1, false);
  IN(host4_3, net4_2, true);
  IN(host4_3, net4_3, false);
  IN(host4_4, net4_1, false);
  IN(host4_4, net4_2, false);
  IN(host4_4, net4_3, false);

  IN(host4_1, net6_1, false);
  IN(host6_1, net4_1, false);

}

int main(int argc, char* argv[])
{
  first_simple_test();
  first_simple_test_ref();
  ipv6_host();
  ipv6_host_ref();
  ip_conversions();
  ip_ownership();
}
#endif
