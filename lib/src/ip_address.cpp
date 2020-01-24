/* Copyright (c) 2015 Digiverse d.o.o.
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
#include <sstream>
#include <cstring>
#include <vector>

#include "cool/ng/ip_address.h"

#if defined(WINDOWS_TARGET)
# pragma comment(lib, "Ws2_32.lib")
#endif

namespace cool { namespace ng {

namespace ip {

std::string to_string(style style_) NOEXCEPT_
{
  switch (style_)
  {
    case style::customary:
      return "customary";
    case style::dot_decimal:
      return "dot_decimal";
    case style::canonical:
      return "canonical";
    case style::strict:
      return "strict";
    case style::expanded:
      return "expanded";
    case style::microsoft:
      return "microsoft";
    case style::dotted_quad:
      return "dotted_quad";
    default:
      return "-unknown-";
  }
}

// ==== ======================================================================
// ==== ======================================================================
// ====
// ====
// ==== host_container
// ====
// ====
// ==== ======================================================================
// ==== ======================================================================

host_container::~host_container()
{
  release();
}

host_container& host_container::operator =(const address& addr_)
{
  switch (addr_.version())
  {
    case version::ipv4:
      assign(ipv4::host(addr_));
      break;

    case version::ipv6:
      assign(ipv6::host(addr_));
      break;
  }
  return *this;
}

host_container& host_container::operator =(const host_container& addr_)
{
  *this = *static_cast<const address*>(addr_);
  return *this;
}

host_container& host_container::operator =(const struct in_addr& addr_)
{
  assign(ipv4::host(addr_));
  return *this;
}

host_container& host_container::operator =(const struct in6_addr& addr_)
{
  *this = ipv6::host(addr_);
  return *this;
}

host_container& host_container::operator =(const struct sockaddr_storage& addr_)
{
  switch (addr_.ss_family)
  {
    case AF_INET:
      *this = reinterpret_cast<const struct sockaddr_in*>(&addr_)->sin_addr;
      break;

    case AF_INET6:
      *this = reinterpret_cast<const struct sockaddr_in6*>(&addr_)->sin6_addr;
      break;

    default:
      throw cool::ng::exception::bad_conversion("unknown address family with numerical value " + std::to_string(addr_.ss_family));
  }
  return *this;
}

host_container::operator sockaddr_storage() const
{
  auto self = static_cast<const address*>(*this);

  sockaddr_storage res;
  switch (self->version())
  {
    case version::ipv4:
      res.ss_family =  AF_INET;
      reinterpret_cast<sockaddr_in*>(&res)->sin_addr = static_cast<in_addr>(*self);
      break;

    case version::ipv6:
      res.ss_family =  AF_INET6;
      reinterpret_cast<sockaddr_in6*>(&res)->sin6_addr = static_cast<in6_addr>(*self);
      break;
  }
  return res;
}

void host_container::release()
{
  static_cast<address*>(static_cast<void*>(&this->m_v4))->~address();
}

void host_container::assign(const ipv4::host& addr_)
{
  release();
  new (&m_v4) ipv4::host(addr_);
}

void host_container::assign(const ipv6::host& addr_)
{
  release();
  new (&m_v6) ipv6::host(addr_);
}

// ==== ======================================================================
// ==== ======================================================================
// ====
// ====
// ==== service
// ====
// ====
// ==== ======================================================================
// ==== ======================================================================

namespace
{

template <typename Buf>
std::istream& get_word(std::istream& is_, Buf* b_, std::size_t n_)
{
  for ( ; n_ > 0; --n_)
  {
    b_->sputc(is_.get());
    if (is_.eof())
      break;
  }
  return is_;
}

class parser
{
 public:
  inline parser(std::istream& is_)
    : m_stream(is_), m_proto(transport::unknown)
  { /* noop */ }
  explicit operator service();

 private:
  void parse();

 private:
  std::istream& m_stream;
  transport m_proto;
  host_container m_host;
  uint16_t m_port;
};

parser::operator service()
{
  parse();
  return service(m_proto, m_host, m_port);
}

void parser::parse()
{
  {
    std::stringstream ss;

    get_word(m_stream, ss.rdbuf(), 6);
    auto aux = ss.str();
    if (aux == "tcp://")
      m_proto = transport::tcp;
    else if (aux == "udp://")
      m_proto = transport::udp;
    else
      throw cool::ng::exception::parsing_error("tcp:// or udp:// protocol specifier expected");
  }

  {
    char c = m_stream.get();
    if (m_stream.eof() || !(c == '[' || (c >= '0' && c <='9')))
      throw cool::ng::exception::parsing_error("either an IPv6 address in square brackets or an IPv4 address expected");

    if (c == '[')  // ipv6 address assumed
    {
      ipv6::host h;
      m_stream >> h;
      m_stream >> c;
      if (c != ']')
        throw cool::ng::exception::parsing_error("closing square bracket ']' expected");
      m_host = h;
    }
    else
    {
      m_stream.unget();
      ipv4::host h;
      m_stream >> h;
      m_host = h;
    }
    m_stream >> c;
    if (c != ':')
      throw cool::ng::exception::parsing_error("port number separator (':') expected in a service address");
    m_stream >> m_port;
  }
}
} // anonymous namespace

namespace detail
{

std::istream& sin(std::istream& is, cool::ng::ip::service& val)
{
  val = static_cast<cool::ng::ip::service>(parser(is));
  return is;
}

} // namespace detail

service::operator std::string() const
{
  std::stringstream ss;
  visualize(ss, style::customary);
  return ss.str();}

std::ostream& service::visualize(std::ostream& os, style style_) const
{
  switch (m_proto)
  {
    case transport::unknown:
      throw cool::ng::exception::bad_conversion("this service object does not contain valid service data");

    case transport::tcp:
      os << "tcp://";
      break;

    case transport::udp:
      os << "udp://";
      break;
  }

  switch (static_cast<const address&>(m_host).version())
  {
    case version::ipv4:
      static_cast<const address&>(m_host).visualize(os, style_);
      break;
    case version::ipv6:
      os << "[";
      static_cast<const address&>(m_host).visualize(os, style_) << "]";
      break;
  }

  os << std::dec << ":" << m_port;
  return os;
}

const struct sockaddr* service::sockaddr() const
{
  if (m_proto == transport::unknown)
    throw cool::ng::exception::bad_conversion("this service object does not contain valid service data");

  switch (static_cast<const address&>(m_host).version())
  {
    case version::ipv4:
      return reinterpret_cast<const struct sockaddr*>(&m_in);

    case version::ipv6:
      return reinterpret_cast<const struct sockaddr*>(&m_in6);
  }

  throw cool::ng::exception::bad_conversion("this service object does not contain valid service data");
}

socklen_t service::sockaddr_len() const
{
  if (m_proto == transport::unknown)
    throw cool::ng::exception::bad_conversion("this service object does not contain valid service data");

  switch (static_cast<const address&>(m_host).version())
  {
    case version::ipv4:
      return sizeof(m_in);

    case version::ipv6:
      return sizeof(m_in6);
  }

  throw cool::ng::exception::bad_conversion("this service object does not contain valid service data");
}

void service::assign(const struct sockaddr *sa_)
{
  if (sa_ == nullptr)
    throw cool::ng::exception::illegal_argument("nullptr input pointer");
  if (m_proto == transport::unknown)
    throw cool::ng::exception::invalid_state("this service object has unknown transport");

  switch (sa_->sa_family)
  {
    case AF_INET:
      m_host = reinterpret_cast<const struct sockaddr_in *>(sa_)->sin_addr;
      m_port = ntohs(reinterpret_cast<const struct sockaddr_in *>(sa_)->sin_port);
      break;

    case AF_INET6:
      m_host = reinterpret_cast<const struct sockaddr_in6 *>(sa_)->sin6_addr;
      m_port = ntohs(reinterpret_cast<const struct sockaddr_in6 *>(sa_)->sin6_port);
      break;

    default:
      throw cool::ng::exception::bad_conversion("unknown address family with numerical value " + std::to_string(sa_->sa_family));
      break;
  }
  sync();
}

void service::assign(const std::string& uri_)
{
  try
  {
    std::stringstream ss(uri_);
    *this = static_cast<service>(parser(ss));
    sync();
  }
  catch (const std::exception& e)
  {
    throw cool::ng::exception::bad_conversion(e.what());
  }
}

void service::sync()
{
  switch (static_cast<const address&>(m_host).version())
  {
    case version::ipv4:
      m_in.sin_addr = static_cast<struct in_addr>(static_cast<const address&>(m_host));
      m_in.sin_family = AF_INET;
      m_in.sin_port = htons(m_port);
      break;

    case version::ipv6:
        m_in6.sin6_addr = static_cast<struct in6_addr>(static_cast<const address&>(m_host));
        m_in6.sin6_family = AF_INET6;
        m_in6.sin6_port = htons(m_port);
        break;
  }
}

namespace detail {

ip::ipv4::host literal_ipv4(const char* lit_)
{
  return ip::ipv4::host(lit_);
}
ip::ipv6::host literal_ipv6(const char* lit_)
{
  return ip::ipv6::host(lit_);
}
ip::ipv4::network literal_ipv4_net(const char* lit_)
{
  return ip::ipv4::network(lit_);
}
ip::ipv6::network literal_ipv6_net(const char* lit_)
{
  return ip::ipv6::network(lit_);
}
std::shared_ptr<ip::address> literal_ip(const char* lit_)
{
  return std::make_shared<ip::ipv4::host>(); // TODO:
}

// ==========================================================================
// =======
// ======= Parsing helpers
// =======
// ==========================================================================
namespace {

// Parses sequence of digits in input stream as decimal integer number.
// Returns the character terminating the sequence.
char parse_num(std::istream& is, int& res)
{
  char c;

  c = is.get();
  if (!is.eof() && c >= '0' && c <= '9')
  {
    res = (c - '0');
  }

  for (c = is.get(); !is.eof() && (c >= '0' && c <= '9'); c = is.get())
  {
    res = 10 * res + (c - '0');
  }
  return c;
}

int parse_hexval(char c)
{
  if (c >= '0' && c <='9')
    return c - '0';

  int res = 10;
  if (c >= 'a' && c <= 'f')
    res += (c - 'a');
  else if (c >= 'A' && c <= 'F')
    res += c - 'A';
  else
    throw exception::parsing_error("hexadecimal digit excpected, got '" + std::to_string(c) + "' instead");

  return res;
}

} // anonymous namespace

template <typename T> class refwrap
{
 public:
  typedef const T& value_t;

  refwrap(const T& arg) : m_ref(arg) { /* noop */ }
  operator value_t() const { return m_ref; }

private:
  value_t m_ref;
};

typedef refwrap<ip::address> list_t;


template <typename T, typename eT>
class special_numbers_base
{
 public:
  using value_type = T;
  using vector_type = std::vector<value_type>;
  using enum_type = eT;

 protected:
  special_numbers_base() = delete;
  special_numbers_base(std::size_t n_) : m_assigned_numbers(n_)
  { /* noop */ }

 public:
  inline typename vector_type::const_iterator cbegin() const
  {
    return m_assigned_numbers.cbegin();
  }
  inline typename vector_type::const_iterator cend() const
  {
   return m_assigned_numbers.cend();
  }
  inline const value_type& at(std::size_t ndx) const
  {
    if (ndx >= m_assigned_numbers.size())
      throw cool::ng::exception::out_of_range("index " + std::to_string(ndx) + " is out of range 0-" + std::to_string(m_assigned_numbers.size() - 1));
    return m_assigned_numbers[ndx];
  }

  std::size_t size() const
  {
    return m_assigned_numbers.size();
  }

  void add(const value_type& v_)
  {
    m_assigned_numbers.push_back(v_);
  }

 protected:
  vector_type m_assigned_numbers;
};

template <typename T>
bool is_special_number(const T& table_, const ip::address& what_)
{
  for (auto it = table_.cbegin(); it != table_.cend(); ++it)
    if (what_.in(*it->m_address))
      return true;
  return false;
}

const uint32_t default_attributes =
    static_cast<int>(ip::attribute::source)
  | static_cast<int>(ip::attribute::destination)
  | static_cast<int>(ip::attribute::forwardable)
  | static_cast<int>(ip::attribute::global)
;

template <typename T>
bool has_attribute(const T& table_, const ip::address& what_, ip::attribute which_)
{
  auto selected = table_.cend();

  // find the narrower address range it belongs to
  for (auto it = table_.cbegin(); it != table_.cend(); ++it)
  {
    if (what_.in(*it->m_address))
    {
      if (selected != table_.cend())
      {
        if (selected->m_address->mask() < it->m_address->mask())
          selected = it;
      }
      else
      {
        selected = it;
      }
    }
  }

  return (((selected == table_.cend() ? default_attributes : selected->m_attributes) ) & static_cast<int>(which_)) != 0;
}

namespace ipv4
{

const ip::ipv4::host loopback = { 127, 0, 0, 1 };
const ip::ipv4::host unspecified = { 0, 0, 0, 0 };
const ip::ipv4::host any;


const ip::ipv4::network unspecified_network =  {  8, {   0, 0, 0, 0 } };
const ip::ipv4::network rfc_private_1 =        {  8, {  10, 0, 0, 0 } };
const ip::ipv4::network rfc_shared_space =     { 10, { 100, 64, 0, 0 } };
const ip::ipv4::network rfc_loopback =         {  8, { 127, 0, 0, 0 } };
const ip::ipv4::network rfc_link_local =       { 16, { 169, 254, 0, 0 } };
const ip::ipv4::network rfc_private_2 =        { 12, { 172, 16, 0, 0 } };
const ip::ipv4::network ietf =                 { 24, { 192, 0, 0, 0 } };
const ip::ipv4::network ietf_continuity =      { 29, { 192, 0, 0, 0 } };
const ip::ipv4::network ietf_dummy =           { 32, { 192, 0, 0, 8 } };
const ip::ipv4::network ietf_pcp_anycast =     { 32, { 192, 0, 0, 9 } };
const ip::ipv4::network ietf_nat_anycast =     { 32, { 192, 0, 0, 10 } };
const ip::ipv4::network ietf_discovery_1 =     { 32, { 192, 0, 0, 170 } };
const ip::ipv4::network ietf_discovery_2 =     { 32, { 192, 0, 0, 171 } };
const ip::ipv4::network rfc_test_net_1 =       { 24, { 192, 0, 2, 0 } };
const ip::ipv4::network rfc_as112 =            { 24, { 192, 31, 196, 0 } };
const ip::ipv4::network rfc_amt =              { 24, { 192, 52, 193, 0 } };
const ip::ipv4::network rfc_private_3 =        { 16, { 192, 168, 0, 0 } };
const ip::ipv4::network rfc_as112_delegation = { 24, { 192, 175, 48, 0 } };
const ip::ipv4::network rfc_benchmark =        { 15, { 198, 18, 0, 0 } };
const ip::ipv4::network rfc_test_net_2 =       { 24, { 198, 51, 100, 0 } };
const ip::ipv4::network rfc_test_net_3 =       { 24, { 203, 0, 113, 0 } };
const ip::ipv4::network rfc_multicast =        {  4, { 224, 0, 0, 0 } };
const ip::ipv4::network rfc_future_use =       {  4, { 240, 0, 0, 0 } };
const ip::ipv4::network rfc_broadcast =        { 32, { 255, 255, 255, 255 } };

class special_number : public detail::special_number<ip::ipv4::network>
                     , public ip::assigned_number
{
 public:
  special_number() : ip::detail::special_number<ip::ipv4::network>()
  { /* noop */}
  special_number(const ip::ipv4::network* n_, const char* a_, const char* b_, bool c_, bool d_, bool e_, bool f_, bool  g_, bool h_ = false)
    : detail::special_number<ip::ipv4::network>(
          n_
        , a_
        , b_
        ,   (c_ ? static_cast<int>(ip::attribute::source) : 0)
          | (d_ ? static_cast<int>(ip::attribute::destination) : 0)
          | (e_ ? static_cast<int>(ip::attribute::forwardable) : 0)
          | (f_ ? static_cast<int>(ip::attribute::global) : 0)
          | (g_ ? static_cast<int>(ATTR_IPv4_ORIGINATED) : 0)
          | (h_ ? static_cast<int>(ip::attribute::multicast) : 0)
      )
  { /* noop */ }

  const network& address()  const override   { return *m_address; }
  const char* description() const override   { return m_name; }
  const char* reference() const override     { return m_ref; }
  bool valid_as_source() const override      { return (m_attributes & static_cast<int>(ip::attribute::source)) != 0; }
  bool valid_as_destination() const override { return (m_attributes & static_cast<int>(ip::attribute::destination)) != 0; }
  bool is_forwardable() const override       { return (m_attributes & static_cast<int>(ip::attribute::forwardable)) != 0; }
  bool is_global() const override            { return (m_attributes & static_cast<int>(ip::attribute::global)) != 0; }
  bool is_mcast() const override             { return (m_attributes & static_cast<int>(ip::attribute::multicast)) != 0; }

};

class special_numbers : public special_numbers_base<special_number, ip::ipv4::assigned>
{
 public:
  special_numbers();
};

special_numbers::special_numbers() : special_numbers_base(enum_type::count)
{

  m_assigned_numbers[enum_type::unspecified]      = value_type(&unspecified_network, "This host on this network", "RFC1122", true, false, false, false, false);
  m_assigned_numbers[enum_type::private_1]        = value_type(&rfc_private_1, "Private use", "RFC1918", true, true, true, false, false);
  m_assigned_numbers[enum_type::shared_space]     = value_type(&rfc_shared_space, "Shared address space", "RFC6598", true, true, true, false, false);
  m_assigned_numbers[enum_type::loopback]         = value_type(&rfc_loopback, "Loopback", "RFC1122", false, false, false, false, false);
  m_assigned_numbers[enum_type::private_2]        = value_type(&rfc_private_2, "Private use", "RFC1918", true, true, true, false, false);
  m_assigned_numbers[enum_type::ietf]             = value_type(&ietf, "IETF protocol assignments", "RFC6980", false, false, false, false, false);
  m_assigned_numbers[enum_type::ietf_continuity]  = value_type(&ietf_continuity, "IPv4 Service Continuity Prefix", "RFC7335", true, true, true, false, false);
  m_assigned_numbers[enum_type::ietf_dummy]       = value_type(&ietf_dummy, "IPv4 dummy address", "RFC7600", true, false, false, false, false);
  m_assigned_numbers[enum_type::ietf_pcp_anycast] = value_type(&ietf_pcp_anycast, "Port control protocol anycast", "RFC7723", true, true, true, true, false);
  m_assigned_numbers[enum_type::ietf_nat_anycast] = value_type(&ietf_nat_anycast, "Traversal using relays around NAT snycast", "RFC8155", true, true, true, true, false);
  m_assigned_numbers[enum_type::ietf_discovery_1] = value_type(&ietf_discovery_1, "NAT64/DNS64 discovery", "RFC7050", false, false, false, false, false);
  m_assigned_numbers[enum_type::ietf_discovery_2] = value_type(&ietf_discovery_2, "NAT64/DNS64 discovery", "RFC7050", false, false, false, false, false);
  m_assigned_numbers[enum_type::test_net_1]       = value_type(&rfc_test_net_1, "Documentation (TEST-NET-1)", "RFC5737", false, false, false, false, false);
  m_assigned_numbers[enum_type::as112]            = value_type(&rfc_as112, "AS112-v4", "RFC7535", true, true, true, true, false);
  m_assigned_numbers[enum_type::amt]              = value_type(&rfc_amt, "Automatic multicast tunneling", "RFC7450", true, true, true, true, false);
  m_assigned_numbers[enum_type::private_3]        = value_type(&rfc_private_3, "Private use", "RFC1918", true, true, true, false, false);
  m_assigned_numbers[enum_type::as112_delegation] = value_type(&rfc_as112_delegation, "Direct Delegation AS112 Service", "RFC7534", true, true, true, true, false);
  m_assigned_numbers[enum_type::benchmark]        = value_type(&rfc_benchmark, "Benchmarking", "RFC2544", true, true, true, false, false);
  m_assigned_numbers[enum_type::test_net_2]       = value_type(&rfc_test_net_2, "Documentation (TEST-NET-2)", "RFC5737", false, false, false, false, false);
  m_assigned_numbers[enum_type::test_net_3]       = value_type(&rfc_test_net_3, "Documentation (TEST-NET-3)", "RFC5737", false, false, false, false, false);
  m_assigned_numbers[enum_type::multicast]        = value_type(&rfc_multicast, "Multicast", "RFC1112", false, true, true, true, false, true);
  m_assigned_numbers[enum_type::future_use]       = value_type(&rfc_future_use, "Reserved", "RFC5771", false, false, false, false, false);
  m_assigned_numbers[enum_type::broadcast]        = value_type(&rfc_broadcast,"Limited broadcast", "RFC8190", false, true, false, false, false);
  m_assigned_numbers[enum_type::link_local]       = value_type(&rfc_link_local, "Link local", "RFC3927", true, true, false, false, false);
}

special_numbers& get_assigned_numbers()
{
  static special_numbers assigned;

  return assigned;
}

} // namespace ipv4

namespace ipv6 {

const ip::ipv6::host loopback = { 1 };
const ip::ipv6::host unspecified;
const ip::ipv6::host any;
const ip::ipv6::network rfc_unspecified(128);
const ip::ipv6::network rfc_loopback =            { 128, {                                                                                             0x01 } };
const ip::ipv6::network rfc_ipv4map =             {  96, {                                                               0xff, 0xff, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network rfc_ipv4translate =       {  96, { 0x00, 0x64, 0xff, 0x9b, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network rfc_ipv4translate_local = {  48, { 0x00, 0x64, 0xff, 0x9b, 0x00, 0x01, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network rfc_discard =             {  64, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network rfc_ietf =                {  23, { 0x20, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network ietf_teredo =             {  32, { 0x20, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network ietf_pcp_anycast =        { 128, { 0x20, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } };
const ip::ipv6::network ietf_nat_anycast =        { 128, { 0x20, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 } };
const ip::ipv6::network rfc_benchmark =           {  48, { 0x20, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network rfc_amt =                 {  32, { 0x20, 0x01, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network rfc_as112 =               {  48, { 0x20, 0x01, 0x00, 0x04, 0x01, 0x12, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network rfc_orchid_v2 =           {  28, { 0x20, 0x01, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network rfc_documentation =       {  32, { 0x20, 0x01, 0x0d, 0xb8, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network rfc_6to4 =                {  16, { 0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network rfc_as112_delegation =    {  48, { 0x26, 0x20, 0x00, 0x4f, 0x80, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network rfc_multicast =           {   8, { 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network rfc_local =               {   7, { 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const ip::ipv6::network rfc_link_local =          {  10, { 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

class special_number : public detail::special_number<ip::ipv6::network>
                     , public ip::assigned_number
{
 public:
  special_number() : detail::special_number<ip::ipv6::network>()
  { /* noop */}
  special_number(const ip::ipv6::network* n_, const char* a_, const char* b_, bool c_, bool d_, bool e_, bool f_, bool  g_, bool h_ =  false)
    : detail::special_number<ip::ipv6::network>(
          n_
        , a_
        , b_
        ,   (c_ ? static_cast<int>(ip::attribute::source) : 0)
          | (d_ ? static_cast<int>(ip::attribute::destination) : 0)
          | (e_ ? static_cast<int>(ip::attribute::forwardable) : 0)
          | (f_ ? static_cast<int>(ip::attribute::global) : 0)
          | (g_ ? static_cast<int>(ATTR_IPv4_ORIGINATED) : 0)
          | (h_ ? static_cast<int>(ip::attribute::multicast) : 0)
      )
  { /* noop */ }

  const network& address()  const override   { return *m_address; }
  const char* description() const override   { return m_name; }
  const char* reference() const override     { return m_ref; }
  bool valid_as_source() const override      { return (m_attributes & static_cast<int>(ip::attribute::source)) != 0; }
  bool valid_as_destination() const override { return (m_attributes & static_cast<int>(ip::attribute::destination)) != 0; }
  bool is_forwardable() const override       { return (m_attributes & static_cast<int>(ip::attribute::forwardable)) != 0; }
  bool is_global() const override            { return (m_attributes & static_cast<int>(ip::attribute::global)) != 0; }
  bool is_mcast() const override             { return (m_attributes & static_cast<int>(ip::attribute::multicast)) != 0; }
};

class special_numbers : public special_numbers_base<special_number, ip::ipv6::assigned>
{
 public:
  special_numbers();
};

special_numbers::special_numbers() : special_numbers_base(enum_type::count)
{
  m_assigned_numbers[enum_type::unspecified] = value_type(&rfc_unspecified, "Unspecified address", "RFC4291", true, false, false, false, false);
  m_assigned_numbers[enum_type::loopback] = value_type(&rfc_loopback, "Loopback address", "RFC4291", false, false, false, false, false);
  m_assigned_numbers[enum_type::ipv4map] = value_type(&rfc_ipv4map, "IPv4-mapped address", "RFC4291", false, false, false, false, true);
  m_assigned_numbers[enum_type::ipv4xlat] = value_type(&rfc_ipv4translate, "IPv4-IPv6 translation", "RFC6052", true, true, true, true, true);
  m_assigned_numbers[enum_type::ipv4xlat_local] = value_type(&rfc_ipv4translate_local, "IPv4-IPv6 translation for local use", "RFC8215", true, true, true, false, true);
  m_assigned_numbers[enum_type::discard] = value_type(&rfc_discard, "Discard-only address block", "RFC6666", true, true, true, false, false);
  m_assigned_numbers[enum_type::ietf] = value_type(&rfc_ietf, "IETF protocol assignments", "RFC2928", false, false, false, false, false);
  m_assigned_numbers[enum_type::ietf_teredo] = value_type(&ietf_teredo, "TEREDO", "RFC4380", true, true, true, false, false);
  m_assigned_numbers[enum_type::ietf_pcp_anycast] = value_type(&ietf_pcp_anycast, "Port control protocol anycast", "RFC7723", true, true, true, true, false);
  m_assigned_numbers[enum_type::ietf_nat_anycast] = value_type(&ietf_nat_anycast, "Traversal using relays around NAT anycast", "RFC8155", true, true, true, true, false);
  m_assigned_numbers[enum_type::ietf_benchmark] = value_type(&rfc_benchmark, "Benchmarking", "RFC5180", true, true, true, false, false);
  m_assigned_numbers[enum_type::ietf_amt] = value_type(&rfc_amt, "Automatic multicast tunneling", "RFC7450", true, true, true, true, false);
  m_assigned_numbers[enum_type::ietf_as112] = value_type(&rfc_as112, "AS112-v6", "RFC7535", true, true, true, true, false);
  m_assigned_numbers[enum_type::ietf_orchid_v2] = value_type(&rfc_orchid_v2, "ORCHIDv2", "RFC7343", true, true, true, true, false);
  m_assigned_numbers[enum_type::documentation] = value_type(&rfc_documentation, "Documentation", "RFC3849", false, false, false, false, false);
  m_assigned_numbers[enum_type::rfc_6to4] = value_type(&rfc_6to4, "Connection of IPv6 domains via IPv4 clouds", "RFC3056", true, true, true, false, false);
  m_assigned_numbers[enum_type::as112_delegation] = value_type(&rfc_as112_delegation, "Direct delegation AS112 service", "RFC7534", true, true, true, true, false);
  m_assigned_numbers[enum_type::multicast] = value_type(&rfc_multicast, "multicast", "RFC2373", false, true, true, true, false, true);
  m_assigned_numbers[enum_type::local] = value_type(&rfc_local, "Unique-local", "RFC4193", true, true, true, false, false);
  m_assigned_numbers[enum_type::link_local] = value_type(&rfc_link_local, "Link-local unicast", "RFC4291", true, true, false, false, false);
}

special_numbers& get_assigned_numbers()
{
  static special_numbers assigned;

  return assigned;
}

} // namespace ipv6

bool is_ipv4_host(const address& addr_)
{
  if (addr_.kind() != ip::kind::host)
    return false;
  if (addr_.version() == ip::version::ipv4)
    return true;

  for (auto it = detail::ipv6::get_assigned_numbers().cbegin(); it != detail::ipv6::get_assigned_numbers().cend(); ++it)
  {
    if ((it->m_attributes & ATTR_IPv4_ORIGINATED) != 0)
    {
      if (addr_.in(*it->m_address))
        return true;
    }
  }
  return false;
}

// ===========================================================================
// ======
// ======
// ======  IPv4 utilities
// ======
// ======
// ===========================================================================

namespace ipv4 {

using cool::ng::ip::ipv4::binary_t;

bool is_valid(char c, const std::string& whitelist)
{
  return whitelist.find_first_of(c) != whitelist.npos;
}

// ====== Parser for dot-decimal IPv4 addresses

class parser
{
 public:
  inline parser(std::istream& is_)
    : m_stream(is_)
  { /* noop */ }
  explicit operator const uint8_t*();

 private:
  void parse();

 private:
  std::istream& m_stream;
  binary_t      m_address;
};

class visualizer
{
 public:
  visualizer(std::ostream& os_, const binary_t& b_, ip::style s_);
  std::ostream& operator()();

 private:
  std::ostream&   m_stream;
  const binary_t& m_buffer;
};

// Parses IPv4 IP address in dotted format.
void parser::parse()
{
  int count;
  char c;

  try
  {
    for (count = 0; count < 4; ++count)
    {
      int aux = -1;
      c = parse_num(m_stream, aux);
      if (aux < 0 || aux > 255)  // number required but not found or number too large
        throw exception::parsing_error("number " + std::to_string(aux) + " is out of valid value range 0-255");

      m_address[count] = static_cast<uint8_t>(aux);

      if (c != '.' && count < 3)
        throw exception::parsing_error("invalid character '" + std::to_string(c) + "' encountered in an IPv4 IP address");
    }

    if (count < 4)
      throw exception::parsing_error("the IPv4 IP address has only " + std::to_string(count) + " fields, 4 were expected");

    if (!m_stream.eof())   // push back the last character if not EOF
      m_stream.unget();
  }
  catch (const cool::ng::exception::parsing_error&)
  {
    if (!m_stream.eof())   // push back the last character if not EOF
      m_stream.unget();
    throw;
  }
}

parser::operator const uint8_t*()
{
  parse();
  return m_address.data();
}

void parse(std::istream& is, binary_t& addr, std::size_t& mask, bool require_mask)
{
  try
  {
    binary_t aux_addr = static_cast<const uint8_t*>(parser(is));

    if (require_mask)
    {
      int aux = -1;
      char c;

      is >> c;

      if (is.eof() || c != '/')
      {
        if (!is.eof())
          is.unget();

        throw cool::ng::exception::bad_conversion("CIDR seperator '/' expected, found '" + std::to_string(c) + "' instead");
      }
      else
      {
        c = parse_num(is, aux);
        if (!is.eof())
          is.unget();
        if (aux < 0)
          throw cool::ng::exception::bad_conversion("bitmask length " + std::to_string(aux) + " is out of valid range 0-32");
      }

      if (aux > 32)
        throw exception::out_of_range("bitmask length " + std::to_string(aux) + " is out of valid range 0-32");
      mask = aux;
    }

    addr = aux_addr;
  }
  catch (const cool::ng::exception::parsing_error& e)
  {
    throw cool::ng::exception::bad_conversion(e.what());
  }
}

visualizer::visualizer(std::ostream& os_, const binary_t& b_, ip::style s_)
  : m_stream(os_), m_buffer(b_)
{
  switch (s_)
  {
    case style::customary:
    case style::dot_decimal:
      break;

    default:
      throw cool::ng::exception::bad_conversion("invalid visual style " + to_string(s_) + " requested");
  }
}

std::ostream& visualizer::operator ()()
{
  std::stringstream ss;
  for (int i = 0; i < 4; ++i)
  {
    if (i > 0)
      m_stream << '.';
    m_stream << static_cast<int>(m_buffer[i]);
  }
  return m_stream;
}

} // namespace ipv4

// ===========================================================================
// ======
// ======
// ======  IPv6 utilities
// ======
// ======
// ===========================================================================
namespace ipv6
{

using cool::ng::ip::ipv6::binary_t;

// IPv6 address quad (two bytes) as used by IPv6 address parser
class quad
{
 public:
  quad& operator =(const std::string&);
  explicit operator uint16_t() const;

 private:
  quad& append_(char c_);

 private:
  std::string m_str;
};

quad::operator uint16_t() const
{
  int res = 0;

  for (auto it = m_str.cbegin(); it != m_str.cend(); ++it)
  {
    res <<= 4;
    res |= parse_hexval(*it);
  }
  return res;
}

quad& quad::operator =(const std::string& str_)
{
  if (str_.length() > 4)
    throw cool::ng::exception::parsing_error("IPv6 address quad string longer than 4 characters: '" + str_ + "'");
  m_str = str_;
  return *this;
}

// IPv6 address delimiter as used by IPv6 address parser. It will throw if
// delimiter is changed mid-address; the only permitted chagne is that of
// ':' into '.' for dotted-decimal presentation

const char delim_ms  = '-';
const char delim_rfc = ':';
const char delim_dot = '.';

class delimiter
{
 public:
  inline delimiter() : m_char('\0')       { }
  explicit inline operator bool() const   { return !(m_char == '\0'); }
  explicit inline operator char() const   { return m_char; }
  inline bool is_rfc() const              { return m_char == delim_rfc; }
  inline bool is_ms() const               { return m_char == delim_ms; }
  inline bool is_dot() const              { return m_char == delim_dot; }

  char operator =(char c_);
  static inline bool is(const char c_) { return c_ == delim_rfc || c_ == delim_ms || c_ == delim_dot; }

 private:
  char set_(char c_)             { return m_char = c_; }
  char m_char;
};

char delimiter::operator =(char c_)
{
  if (!*this)
  {
    if (c_ == delim_rfc || c_ == delim_ms)
      return set_(c_);
    throw cool::ng::exception::parsing_error("IPv6 address quad delimiter (':' or '-') expected, got '" + std::to_string(c_) + "' instead");
  }
  if (c_ != m_char)
  {
    if (m_char == delim_rfc && c_ == delim_dot)  // only permitted change
      return set_(c_);
    throw cool::ng::exception::parsing_error("cannot change IPv6 quad delimiter from ':' to '-' or vice versa");
  }
  return m_char;
}


// ====== Parser for RFC 5952 IPv6 addresses

class parser
{
  enum class token { delimiter, compression, quad, decimal, empty, end_of_stream };
  enum class base { decimal = 10, hex = 16 };

 public:
  inline parser(std::istream& is_)
    : m_stream(is_)
  { /* noop */ }
  explicit operator const uint8_t*();

 private:
  void parse();
  token fetch(std::string& str_, base b_);
  void inflate();
  static bool is_digit(char c_, base b_);
  inline int number_of_quads() const { return m_decimal_count == 0 ? 8 : 6; }

 private:
  // input and interim data
  std::istream& m_stream;
  delimiter     m_delimiter;
  quad          m_quads[8];
  cool::ng::ip::ipv6::binary_t m_address; // result

  // state variables
  bool m_start = true;           // true until the first character is read
  bool m_is_deflated = false;    // true if it has :: compression sequence
  int  m_first_deflated = -1;    // position of first deflated quad
  int  m_last_quad = -1;
  int  m_decimal_count = 0;      // number of dot-decimal fields, if present

};

bool parser::is_digit(char c_, base b_)
{
  if (c_ >= '0' && c_ <= '9')
    return true;

  return c_ >= '0' && c_ <= '9' ? true
      : b_ == base::hex ? (c_ >='a' && c_ <= 'f') || (c_ >= 'A' && c_ <= 'F') : false;
}

parser::operator const uint8_t*()
{
  parse();
  return m_address.data();
}

parser::token parser::fetch(std::string& str_, base b_)
{
  char c;
  m_stream >> c;

  if (m_stream.eof())
    return token::end_of_stream;

  switch (c)
  {
    case delim_ms:
    case delim_rfc:
      m_delimiter = c;
      if (m_start)
      {
        m_start = false;
        return token::delimiter;
      }

      return token::compression;

    default:
      m_start = false;
      str_.clear();
      do
      {
        if (!is_digit(c, b_))
          break;
        str_ += c;
        if (str_.length() > 4)
          throw cool::ng::exception::parsing_error("IPv6 quad string is longer than 4 characters: '" + str_ + "'");
        m_stream >> c;
      } while (!m_stream.eof());

      if (m_stream.eof())
        return b_ == base::hex ? token::quad : token::decimal;

      if (c == delim_rfc || c == delim_ms)
      {
        m_delimiter = c;
        return token::quad;
      }
      else if (c == delim_dot)
      {
        if (b_ == base::hex)   // the first decimal group, check if only decimal digits
        {
          for (int i = 0; i < str_.length(); ++i)
            if (!is_digit(str_[i], base::decimal))
              throw cool::ng::exception::parsing_error("only decimal digits are permitted when using dot_decimal visual style");
        }
        return token::decimal;
      }
      m_stream.unget();
      break;
  }
  return str_.empty() ? token::empty : b_ == base::hex ? token::quad : token::decimal;
}

void parser::parse()
{
  std::string text;
  base numeric_base = base::hex;
  bool loop = true;
  bool need_compression = false;

  while (loop)
  {
    switch (fetch(text, numeric_base))
    {
      case token::end_of_stream:
      case token::empty:
        loop = false;
        break;

      case token::delimiter:
        need_compression = true;
        break;

      case token::quad:
        if (need_compression)
          throw cool::ng::exception::parsing_error("zero-compressed field ('::' or '--') was expected");

        ++m_last_quad;
        if (m_last_quad >= 8)
          throw cool::ng::exception::parsing_error("more than 8 address quads are present in an IPv6 address");

        m_quads[m_last_quad] = text;
        break;

      case token::compression:
        need_compression = false;
        if (m_is_deflated)     // can't have two :: sequences
          throw cool::ng::exception::parsing_error("only a single zero-compressed field ('::') is allowed in an IPv6 address");
        m_is_deflated = true;
        m_first_deflated = m_last_quad + 1;
        break;

      case token::decimal:
      {
        m_delimiter = '.';  // switch delimiter to check the change is legal
        int aux;

        if (need_compression)
          throw cool::ng::exception::parsing_error();

        if (m_decimal_count >= 4)
          throw cool::ng::exception::parsing_error("zero-compressed field ('::' or '--') was expected");
        std::istringstream is(text);
        parse_num(is, aux);
        if (aux < 0 || aux > 255)
          throw cool::ng::exception::parsing_error("dot_decimal style permits numeric field range 0-255, value " + std::to_string(aux) + " found");
        m_address[12 + m_decimal_count] = static_cast<uint8_t>(aux);
        ++m_decimal_count;
        numeric_base = base::decimal;
      }
    }
  }

  if (m_is_deflated)
    inflate();

  if (m_last_quad != number_of_quads() - 1)
    throw cool::ng::exception::parsing_error("IPv6 address does not contain 8 quads (zero compressed field included)");

  if (m_decimal_count > 0)
  {         // must have exactly 4 decimal fields with two spare quads to use
    if (m_decimal_count != 4)
      throw cool::ng::exception::parsing_error("dot_decimal style must have 4 decimal fields");
  }

  for (int i = 0; i < number_of_quads(); ++i)
  {
    auto aux = static_cast<uint16_t>(m_quads[i]);
    m_address[2 * i] = aux >> 8;
    m_address[2 * i + 1] = aux & 0xff;
  }
}

void parser::inflate()
{
  int inflate_count = number_of_quads() - (m_last_quad + 1);
  if (inflate_count < number_of_quads())
  {
    int move_count = m_last_quad - m_first_deflated + 1;
    int destination = number_of_quads() - 1;
    int source = m_last_quad;

    for (int i = 0; i < move_count; ++i)
        m_quads[destination--] = m_quads[source--];
    for (int i = 0; i < inflate_count; ++i)
      m_quads[m_first_deflated++] = quad();
  }
  m_last_quad += inflate_count;
}

// NOTE: Term "quad" in below code refers to two-byte quantity which value
//       can be visualized with four hex digits.

//
//    byte        0         1
//           +----+----+----+----+
//           |         |         |
//           +----+----+----+----+
//   nibble    0    1    2    3
class q_wrap
{
 public:
  q_wrap(const uint8_t* data) : m_data(data)
  { /* noop */ }

  uint_fast16_t operator[](std::size_t index) const;
  constexpr static std::size_t size() { return 8; }
  std::ostream& quad(std::ostream& os, std::size_t index) const;
  std::ostream& dot_decimal(std::ostream& os, std::size_t index) const;

 private:
  const uint8_t* m_data;
};

std::ostream& q_wrap::quad(std::ostream& os, std::size_t index) const
{
  return os << std::hex << (*this)[index];
}

std::ostream& q_wrap::dot_decimal(std::ostream& os, std::size_t index) const
{
  std::size_t r_index = index << 1;
  return os << std::dec << static_cast<int>(m_data[r_index]) << "." << static_cast<int>(m_data[r_index + 1]);
}

uint_fast16_t q_wrap::operator[](std::size_t index) const
{
  if (index >= size() )
    throw cool::ng::exception::out_of_range("index out of range");

  std::size_t r_index = index << 1;
  return (m_data[r_index] << 8) | m_data[r_index + 1];
}

class visualizer
{
 public:
  visualizer(std::ostream& os_, const binary_t& b_)
    : m_stream(os_)
    , m_buffer(b_.data())
  { /* noop */ }
  std::ostream& operator()(ip::style s_);
  std::ostream& operator()()
  {
    return this->operator ()(ip::style::customary);
  }

 private:
  std::ostream& expanded(std::size_t limit);
  std::ostream& deflated(std::size_t limit, char delimiter, bool& ends_with_deflated);
  std::ostream& dotted_quad();
  bool find_zero_sequence(std::size_t limit, std::size_t& start, std::size_t& length);

 private:
  std::ostream&   m_stream;
  q_wrap          m_buffer;
};

std::ostream& visualizer::operator()(ip::style s_)
{
  bool aux;

  switch (s_)
  {
    case ip::style::expanded:
      return expanded(m_buffer.size());

    case ip::style::canonical:
    case ip::style::strict:
      return deflated(m_buffer.size(), ':', aux);

    case ip::style::microsoft:
      return deflated(m_buffer.size(), '-', aux);

    case ip::style::dotted_quad:
      return dotted_quad();

    case ip::style::customary:
    case ip::style::dot_decimal:
    default:
      throw cool::ng::exception::bad_conversion("invalid visual style " + to_string(s_) + " requested");
  }
  return m_stream;
}

std::ostream& visualizer::deflated(std::size_t limit, char delimiter, bool& ends_with_deflated)
{
  std::size_t first, length;  // this is for deflated zero sequence

  if (!find_zero_sequence(limit, first, length))
    return expanded(limit);

  for (std::size_t i = 0; i < first; ++i)
    m_buffer.quad(m_stream, i) << delimiter;

  m_stream << delimiter;
  if (first == 0)
    m_stream << delimiter;

  ends_with_deflated = first + length >= limit;
  if (!ends_with_deflated)
  {
    for (std::size_t i = first + length; i < limit - 1; ++i)
      m_buffer.quad(m_stream, i) << delimiter;

    m_buffer.quad(m_stream, limit - 1);
  }
  return m_stream;
}

std::ostream& visualizer::dotted_quad()
{
  bool ends_with_deflated;

  deflated(m_buffer.size() - 2, ':', ends_with_deflated);
  if (!ends_with_deflated)
    m_stream << ":";

  m_buffer.dot_decimal(m_stream, m_buffer.size() - 2) << ".";
  return m_buffer.dot_decimal(m_stream, m_buffer.size() - 1);
}

std::ostream& visualizer::expanded(std::size_t limit)
{
  for (std::size_t i = 0; i < limit - 1; ++i)
    m_buffer.quad(m_stream, i) << ":";
  return m_buffer.quad(m_stream, limit - 1);
}

bool visualizer::find_zero_sequence(std::size_t limit, std::size_t& start, std::size_t& length)
{
  length = 0;
  std::size_t cur_start = 0, cur_length = 0;

  for (std::size_t i = 0; i < limit; ++i)
  {
    if (m_buffer[i] != 0)     // possible end of  current zero sequence
    {
      cur_length = 0;
    }
    else
    {
      if (cur_length == 0)   // start of new zero sequence
      {
        cur_start = i;
        cur_length = 1;
      }
      else
      {
        ++cur_length;        // current 0 sequence continues
        if (cur_length > length)
        {
          start = cur_start;
          length = cur_length;
        }
      }
    }
  }

  return length > 1;
}

void parse(std::istream& is, binary_t& val, std::size_t& mask, bool require_mask)
{
  try
  {
    binary_t addr = static_cast<const uint8_t*>(parser(is));

    if (require_mask)
    {
      int aux = -1;
      char c = is.get();

      if (is.eof() || c != '/')
      {
        if (!is.eof())
          is.unget();
        throw cool::ng::exception::bad_conversion("CIDR seperator '/' expected, found '" + std::to_string(c) + "' instead");
      }
      else
      {
        parse_num(is, aux);
        if (!is.eof())
          is.unget();
        if (aux < 0)
          throw cool::ng::exception::out_of_range("netmask length " + std::to_string(aux) + " is out of valid range 0-128");

        if (aux > 128)
          throw exception::out_of_range("netmask length " + std::to_string(aux) + " is out of valid range 0-128");
        mask = aux;
      }
    }

    val = addr;
  }
  catch (const cool::ng::exception::parsing_error& e)
  {
    throw cool::ng::exception::bad_conversion(e.what());
  }
}

} // namespace ipv6


std::istream& sin(std::istream& is, cool::ng::ip::address& val)
{
  try
  {
    switch (val.version())
    {
      case version::ipv4:
      {
        detail::ipv4::binary_t addr;
        std::size_t mask;
        if (val.kind() == ip::kind::host)
        {
          ipv4::parse(is, addr, mask, false);
          val = ip::ipv4::host(addr.data());
        }
        else
        {
          ipv4::parse(is, addr, mask, true);
          val = ip::ipv4::network(mask, addr.data());
        }
        break;
      }
      case version::ipv6:
      {
        detail::ipv6::binary_t addr;
        std::size_t mask;
        if (val.kind() == ip::kind::host)
        {
          ipv6::parse(is, addr, mask, false);
            val = ip::ipv6::host(addr.data());
          }
          else
          {
            ipv6::parse(is, addr, mask, true);
            val = ip::ipv6::network(mask, addr.data());
          }
        }
        break;
    }
  }
  catch (const cool::ng::exception::parsing_error& e)
  {
    throw cool::ng::exception::bad_conversion(e.what());
  }
  return is;
}

} // namespace detail

// ==== ======================================================================
// ==== ======================================================================
// ====
// ====
// ==== IPv6 API implementation
// ====
// ====
// ==== ======================================================================
// ==== ======================================================================

namespace ipv6
{

host const host::loopback = { 1 };
host const host::unspecified;
host const host::any;

// ============= assigned class
const std::size_t assigned::count = assigned::link_local + 1;

const network& assigned::get(std::size_t what_)
{
  return *detail::ipv6::get_assigned_numbers().at(what_).m_address;

}

const assigned_number& assigned::get_info(std::size_t what_)
{
  return detail::ipv6::get_assigned_numbers().at(what_);
}

std::size_t assigned::add(
    const network &addr_
  , const char *desc_
  , const char *ref_
  , bool is_src_
  , bool is_dst_
  , bool is_fwd_
  , bool is_glob_
  , bool is_mcast_)
{
  static const char* empty = "";
  std::size_t index;

  index = detail::ipv6::get_assigned_numbers().size();
  detail::ipv6::get_assigned_numbers().add(
    detail::ipv6::special_numbers::value_type(
        &addr_
      , desc_ == nullptr ? empty : desc_
      , ref_ == nullptr ? empty : ref_
      , is_src_
      , is_dst_
      , is_fwd_
      , is_glob_
      , false
      , is_mcast_
  ));

  return index;
}

// ============= host class

bool host::equals(const ip::address &other) const
{
  if (kind() != other.kind())
    return false;

  if (other.version() == version())
    return m_data == static_cast<const uint8_t*>(other);

  // special treatment if this host is mapped IPv4 host
  if (detail::is_ipv4_host(*this))
    return ::memcmp(&m_data[12], static_cast<const uint8_t*>(other), 4) == 0;

  return false;
}

host::operator struct in_addr() const
{
  struct in_addr result;

  if (detail::is_ipv4_host(*this))
    ::memcpy(static_cast<void*>(&result), m_data.data() + 12, 4);
  else
    throw exception::bad_conversion("this address is not convertible to IPv4 host address");

  return result;
}

host::operator struct in6_addr() const
{
  struct in6_addr result;

  ::memcpy(static_cast<void*>(&result), m_data.data(), m_data.size());
  return result;
}

host::operator std::string() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

std::ostream& host::visualize(std::ostream& os, ip::style style_) const
{
  switch (style_)
  {
    case ip::style::customary:
      return visualize(os, ip::style::canonical);  // call again with the real style

    case ip::style::canonical:
      if (detail::is_ipv4_host(*this))
        return visualize(os, ip::style::dotted_quad);
      break;

    case ip::style::strict:
    case ip::style::expanded:
    case ip::style::microsoft:
    case ip::style::dotted_quad:
      break;

    case ip::style::dot_decimal:
      throw cool::ng::exception::bad_conversion("IPv6 adddress cannot be visualized using " + to_string(style_) + "visual style");
  }

  return detail::ipv6::visualizer(os, m_data)(style_);
}

void host::assign(const ip::address& val)
{
  if (val.kind() != kind())
    throw exception::bad_conversion("cannot assign a network address to the host address object");

  switch (val.version())
  {
    case ip::version::ipv6:
      m_data = static_cast<const uint8_t*>(val);
      break;

    case ip::version::ipv4:
      m_data = static_cast<const uint8_t*>(detail::ipv6::rfc_ipv4map);
      ::memcpy(&m_data[size() - val.size()], static_cast<const uint8_t*>(val), val.size());
      break;
  }
}

void host::assign(const struct in_addr& rhs)
{
  m_data = static_cast<const uint8_t*>(detail::ipv6::rfc_ipv4map);
  ::memcpy(&m_data[12], &rhs, 4);
}

void host::assign(const struct in6_addr& rhs)
{
  m_data = static_cast<const uint8_t*>(static_cast<const void*>(&rhs));
}

void host::assign(uint8_t const rhs[])
{
  m_data = rhs;

}

void host::assign(const std::string& rhs)
{
  std::size_t mask;
  std::stringstream ss(rhs);

  detail::ipv6::parse(ss, m_data, mask, false);
}

bool host::is(ip::attribute a) const
{
  switch (a)
  {
    case ip::attribute::unspecified:
      return in(ip::ipv6::assigned::get(ip::ipv6::assigned::unspecified));

    case ip::attribute::loopback:
      return in(ip::ipv6::assigned::get(ip::ipv6::assigned::loopback));

    case ip::attribute::ipv4originated:
      return detail::has_attribute(detail::ipv6::get_assigned_numbers(), *this, static_cast<ip::attribute>(ATTR_IPv4_ORIGINATED));

    case ip::attribute::assigned:
      return detail::is_special_number(detail::ipv6::get_assigned_numbers(), *this);

    case ip::attribute::source:
    case ip::attribute::destination:
    case ip::attribute::forwardable:
    case ip::attribute::global:
    case ip::attribute::multicast:
      return detail::has_attribute(detail::ipv6::get_assigned_numbers(), *this, a);
  }

  return false;
}

bool host::in(const ip::network& net) const
{
  if (version() != net.version())
    return false;
  return net.has(*this);
}

// ============= network class

network::network(std::size_t mask_size, std::initializer_list<uint8_t> args)
  : m_data(args)
  , m_length(mask_size)
{
  if (mask_size > size() * 8)
    throw exception::out_of_range("network mask is out of the valid value range 0..128");
  // zero host part of the address
  m_data &= detail::calculate_mask<16>(m_length);
}

bool network::equals(const ip::address &other) const
{
  if (kind() != other.kind() || version() != other.version())
    return false;
  return m_data == static_cast<const uint8_t*>(other)
      && m_length == dynamic_cast<const network&>(other).m_length;
}

std::ostream& network::visualize(std::ostream& os, ip::style style_) const
{
  switch (style_)
  {
    case ip::style::customary:
      return visualize(os, ip::style::canonical);  // call again with the real style

    case ip::style::canonical:
      break;

    case ip::style::strict:
    case ip::style::expanded:
    case ip::style::microsoft:
      break;

    case ip::style::dotted_quad:  // not useful for networks
    case ip::style::dot_decimal:
      throw cool::ng::exception::bad_conversion("visual style " + to_string(style_) + " cannot be used to visualize network address");
  }

  return detail::ipv6::visualizer(os, m_data)(style_) << "/" << std::dec << m_length;
}

network::operator std::string() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

network::operator struct in_addr() const
{
  throw exception::bad_conversion("IPv6 network address cannot be converted to struct in_addr");
}

network::operator struct in6_addr() const
{
  struct in6_addr result;

  ::memcpy(static_cast<void*>(&result), m_data.data(), m_data.size());
  return result;
}

void network::assign(const ip::address& rhs)
{
  // special case
  if (rhs.kind() == ip::kind::host && rhs.version() == ip::version::ipv6)
  {
    m_data = static_cast<const uint8_t*>(rhs);
    m_data = m_data & detail::calculate_mask<16>(m_length);
    return;
  }

  if (kind() != rhs.kind() || version() != rhs.version())
    throw exception::bad_conversion("only and IPv6 network address can be assigned to an IPv6 network address object");
  m_data = static_cast<const uint8_t*>(rhs);
  m_length = dynamic_cast<const ip::network&>(rhs).mask();

  // zero host part of the address
  m_data = m_data & detail::calculate_mask<16>(m_length);
}

void network::assign(const struct in_addr& rhs)
{
  throw exception::bad_conversion("cannot assign struct in_addr to an IPv6 network address object");
}

void network::assign(const struct in6_addr& rhs)
{
  m_data = static_cast<const uint8_t*>(static_cast<const void*>(&rhs));
  // zero host part of the address
  m_data = m_data & detail::calculate_mask<16>(m_length);
}

void network::assign(uint8_t const rhs[])
{
  m_data = rhs;
  m_data = m_data & detail::calculate_mask<16>(m_length);
}

void network::assign(const std::string& rhs)
{
  std::stringstream ss(rhs);
  detail::ipv6::parse(ss, m_data, m_length, true);
  m_data = m_data & detail::calculate_mask<16>(m_length);
}

bool network::is(ip::attribute a) const
{
  switch (a)
  {
    case ip::attribute::unspecified:
      return in(ip::ipv6::assigned::get(ip::ipv6::assigned::unspecified));

    case ip::attribute::loopback:
      return in(ip::ipv6::assigned::get(ip::ipv6::assigned::loopback));

    case ip::attribute::ipv4originated:
      return detail::has_attribute(detail::ipv6::get_assigned_numbers(), *this, static_cast<ip::attribute>(ATTR_IPv4_ORIGINATED));

    case ip::attribute::assigned:
      return detail::is_special_number(detail::ipv6::get_assigned_numbers(), *this);

    case ip::attribute::source:
    case ip::attribute::destination:
    case ip::attribute::forwardable:
    case ip::attribute::global:
    case ip::attribute::multicast:
      return detail::has_attribute(detail::ipv6::get_assigned_numbers(), *this, a);
  }

  return false;
}

bool network::in(const ip::network& net) const
{
  if (version() != net.version())
    return false;
  return net.has(*this);
}

bool network::has(const ip::address& other) const
{
  if (version() != other.version())
    return false;

  return m_data == (detail::calculate_mask<16>(m_length) & static_cast<const uint8_t*>(other));
}

} // namespace

// ==== ======================================================================
// ==== ======================================================================
// ====
// ====
// ==== IPv4 API implementation
// ====
// ====
// ==== ======================================================================
// ==== ======================================================================

namespace ipv4 {

const host host::loopback = { 127, 0, 0, 1 };
const host host::unspecified;
const host host::any;

// ============= reserved class
const std::size_t assigned::count = assigned::link_local + 1;

const network& assigned::get(std::size_t what_)
{
  return *detail::ipv4::get_assigned_numbers().at(what_).m_address;
}

const ip::assigned_number& assigned::get_info(std::size_t what_)
{
  return detail::ipv4::get_assigned_numbers().at(what_);
}

std::size_t assigned::add(
    const network &addr_
  , const char *desc_
  , const char *ref_
  , bool is_src_
  , bool is_dst_
  , bool is_fwd_
  , bool is_glob_
  , bool is_mcast_)
{
  static const char* empty = "";
  std::size_t index;

  index = detail::ipv4::get_assigned_numbers().size();
  detail::ipv4::get_assigned_numbers().add(
    detail::ipv4::special_numbers::value_type(
        &addr_
      , desc_ == nullptr ? empty : desc_
      , ref_ == nullptr ? empty : ref_
      , is_src_
      , is_dst_
      , is_fwd_
      , is_glob_
      , false
      , is_mcast_
  ));

  return index;
}

// ============= host class
bool host::equals(const ip::address &other) const
{
  if (kind() != other.kind())
    return false;

  if (other.version() == version())
    return m_data == static_cast<const uint8_t*>(other);

  // special treatment for IPv4 mapped addresses
  if (detail::is_ipv4_host(other))
    return ::memcmp(m_data.data(), static_cast<const uint8_t*>(other) + 12, 4) == 0;

  return false;
}

std::ostream& host::visualize(std::ostream& os, ip::style style_) const
{
  return detail::ipv4::visualizer(os, m_data, style_)();
}

host::operator struct in_addr() const
{
  struct in_addr result;

  ::memcpy(static_cast<void*>(&result), m_data.data(), m_data.size());
  return result;
}

host::operator struct in6_addr() const
{
  struct in6_addr result;

  ::memcpy(static_cast<void*>(&result), static_cast<const uint8_t*>(*detail::ipv6::get_assigned_numbers().at(ip::ipv6::assigned::ipv4map).m_address), 12);
  ::memcpy(static_cast<uint8_t*>(static_cast<void*>(&result)) + 12, m_data.data(), 4);
  return result;
}

host::operator std::string() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

void host::assign(const ip::address& rhs)
{
  if (kind() != rhs.kind())
    throw exception::bad_conversion("cannot assign network address to host addresss object");

  switch (rhs.version())
  {
    case ip::version::ipv4:
      m_data = static_cast<const uint8_t*>(rhs);
      break;

    case ip::version::ipv6:
      if (!detail::is_ipv4_host(rhs))
        throw exception::bad_conversion("cannot assign IPv6 host address not originated in IPv4 space to an IPv4 host address object");
      m_data = static_cast<const uint8_t*>(rhs) + (rhs.size() - size());
      break;
  }
}

void host::assign(const struct in_addr& rhs)
{
  m_data = static_cast<const uint8_t*>(static_cast<const void*>(&rhs));
}

void host::assign(const struct in6_addr& rhs)
{
  ipv6::host aux(rhs);

  if (!detail::is_ipv4_host(aux))
    throw exception::bad_conversion("cannot assign IPv6 host address not originated in IPv4 space to an IPv4 host address object");
  m_data = static_cast<const uint8_t*>(static_cast<const void*>(&rhs)) + 12;
}

void host::assign(uint8_t const rhs[])
{
  m_data = rhs;
}

void host::assign(const std::string& rhs)
{
  std::size_t mask;
  std::stringstream ss(rhs);

  detail::ipv4::parse(ss, m_data, mask, false);
}


bool host::is(ip::attribute a) const
{
  switch (a)
  {
    case ip::attribute::unspecified:
      return in(ip::ipv4::assigned::get(ip::ipv4::assigned::unspecified));

    case ip::attribute::loopback:
      return in(ip::ipv4::assigned::get(ip::ipv4::assigned::loopback));

    case ip::attribute::ipv4originated:
      return true;

    case ip::attribute::assigned:
      return detail::is_special_number(detail::ipv4::get_assigned_numbers(), *this);

    case ip::attribute::source:
    case ip::attribute::destination:
    case ip::attribute::forwardable:
    case ip::attribute::global:
    case ip::attribute::multicast:
      return detail::has_attribute(detail::ipv4::get_assigned_numbers(), *this, a);
  }

  return false;
}

bool host::in(const ip::network& net) const
{
  if (version() != net.version())
    return false;

  return net.has(*this);
}

// ============= network class


network::network(std::size_t mask_size, std::initializer_list<uint8_t> args)
  : m_data(args)
  , m_length(mask_size)
{
  if (mask_size > size() * 8)
    throw exception::out_of_range("netmask size " + std::to_string(mask_size) + " is out of valid value range 0-32");
  m_data &= detail::calculate_mask<4>(m_length);
}

bool network::equals(const ip::address &other) const
{
  if (kind() != other.kind() || version() != other.version())
    return false;
  return m_data == static_cast<const uint8_t*>(other)
      && m_length == dynamic_cast<const network&>(other).m_length;
}

std::ostream& network::visualize(std::ostream &os, ip::style style_) const
{
  return detail::ipv4::visualizer(os, m_data, style_)() << "/" << m_length;
}

network::operator std::string() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

network::operator struct in_addr() const
{
  struct in_addr result;

  ::memcpy(static_cast<void*>(&result), m_data.data(), m_data.size());
  return result;
}

bool network::is(ip::attribute a) const
{
  switch (a)
  {
    case ip::attribute::unspecified:
      return in(ip::ipv4::assigned::get(ip::ipv4::assigned::unspecified));

    case ip::attribute::loopback:
      return in(ip::ipv4::assigned::get(ip::ipv4::assigned::loopback));

    case ip::attribute::ipv4originated:
      return true;

    case ip::attribute::assigned:
      return detail::is_special_number(detail::ipv4::get_assigned_numbers(), *this);

    case ip::attribute::source:
    case ip::attribute::destination:
    case ip::attribute::forwardable:
    case ip::attribute::global:
    case ip::attribute::multicast:
      return detail::has_attribute(detail::ipv4::get_assigned_numbers(), *this, a);
  }

  return false;
}

void network::assign(const ip::address& rhs)
{
  // special case
  if (rhs.kind() == ip::kind::host)
  {
    switch (rhs.version())
    {
      case ip::version::ipv4:
        m_data = static_cast<const uint8_t*>(rhs);
        break;

      case ip::version::ipv6:
        if (!detail::is_ipv4_host(rhs))
          throw exception::bad_conversion("cannot assign IPv6 host address not originated in IPv4 space to an IPv4 network address object");
        m_data = static_cast<const uint8_t*>(rhs) + 12;
        break;
    }

    m_data = m_data & detail::calculate_mask<4>(m_length);
    return;
  }

  if (rhs.version() != ip::version::ipv4)
    throw exception::bad_conversion("cannot assign IPv6 host address to IPv4 network address object");
  m_data = static_cast<const uint8_t*>(rhs);
  m_data = m_data & detail::calculate_mask<4>(m_length);
}

void network::assign(const struct in_addr& rhs)
{
  m_data = static_cast<const uint8_t*>(static_cast<const void*>(&rhs));
  m_data = m_data & detail::calculate_mask<4>(m_length);
}

void network::assign(const struct in6_addr& rhs)
{
  throw exception::bad_conversion("cannot assign struct inn6_addr to an IPv4 network address object");
}

void network::assign(uint8_t const rhs[])
{
  m_data = rhs;
  m_data = m_data & detail::calculate_mask<4>(m_length);
}

void network::assign(const std::string& rhs)
{
  std::stringstream ss(rhs);

  detail::ipv4::parse(ss, m_data, m_length, true);
  m_data = m_data & detail::calculate_mask<4>(m_length);
}

bool network::in(const ip::network& net) const
{
  if (version() != net.version())
    return false;

  return net.has(*this);
}

bool network::has(const ip::address& other) const
{
  if (version() != other.version())
    return false;

  return m_data == (detail::calculate_mask<4>(m_length) & static_cast<const uint8_t*>(other));
}


} // namespace ipv4

} } } // namespace
