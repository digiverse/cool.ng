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

#include "cool/ng/ip_address.h"

#if defined(WINDOWS_TARGET)
# pragma comment(lib, "Ws2_32.lib")
#endif

namespace cool { namespace ng { namespace net {

namespace ip {

std::istream& operator >>(std::istream& is, cool::ng::net::ip::address& val)
{
  switch (val.version())
  {
    case cool::ng::net::ip::IPv4:
      cool::ng::net::detail::ipv4::parse(is, val);
      break;

    case cool::ng::net::ip::IPv6:
      cool::ng::net::detail::ipv6::parse(is, val);
      break;
  }
  return is;
}

} // namespace ip

namespace detail {

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

namespace ipv4 {

  const list_t assigned_list[] = {
    cool::ng::net::ipv4::loopback,
    cool::ng::net::ipv4::any,
    cool::ng::net::ipv4::broadcast,
    cool::ng::net::ipv4::rfc_broadcast,
    cool::ng::net::ipv4::rfc_private_24,
    cool::ng::net::ipv4::rfc_private_20,
    cool::ng::net::ipv4::rfc_private_16,
    cool::ng::net::ipv4::rfc_carrier_nat,
    cool::ng::net::ipv4::rfc_loopback,
    cool::ng::net::ipv4::rfc_unset,
    cool::ng::net::ipv4::rfc_iana_private,
    cool::ng::net::ipv4::rfc_test,
    cool::ng::net::ipv4::rfc_test_2,
    cool::ng::net::ipv4::rfc_test_3,
    cool::ng::net::ipv4::rfc_6to4_anycast,
    cool::ng::net::ipv4::rfc_test_comm,
    cool::ng::net::ipv4::rfc_mcast,
    cool::ng::net::ipv4::rfc_test_mcast,
    cool::ng::net::ipv4::rfc_future
  };

    const std::size_t assigned_list_size = sizeof(assigned_list)/sizeof(refwrap<ip::address>);

} // namespace ipv4

namespace ipv6 {
  const list_t assigned_list[] = {
    cool::ng::net::ipv6::loopback,
    cool::ng::net::ipv6::unspecified,
    cool::ng::net::ipv6::rfc_ipv4map,
    cool::ng::net::ipv6::rfc_teredo,
    cool::ng::net::ipv6::rfc_ipv4translate,
    cool::ng::net::ipv6::rfc_discard,
    cool::ng::net::ipv6::rfc_doc,
    cool::ng::net::ipv6::rfc_local,
    cool::ng::net::ipv6::rfc_link,
    cool::ng::net::ipv6::rfc_mcast
  };

  const std::size_t assigned_list_size = sizeof(assigned_list)/sizeof(refwrap<ip::address>);

} // namespace ipv6

bool is_assigned(list_t const list[], std::size_t size, const ip::address& addr)
{
  for (int i = 0; i < size; ++i)
  {
    switch (static_cast<list_t::value_t>(list[i]).kind())
    {
      case ip::HostAddress:
        if (list[i] == addr)
          return true;
        break;

      case ip::NetworkAddress:
        if (addr.in(dynamic_cast<const ip::network&>(static_cast<list_t::value_t>(list[i]))))
          return true;
        break;
    }
  }
  return false;
}

const char* hex_digits = "0123456789abcdef";

// ===========================================================================
// ======
// ======
// ======  IPv4 utilities
// ======
// ======
// ===========================================================================

namespace ipv4 {

using cool::ng::net::ipv4::binary_t;

// Parses sequence of digits in input stream as decimal integer number.
// Returns the character terminating the sequence.
char parse_num(std::istream& is, unsigned int& res)
{
  char c;
  res = 0;

  for (c = is.get(); !is.eof() && (c >= '0' && c <= '9'); c = is.get())
  {
    res = 10 * res + (c - '0');
  }
  return c;
}

// Parses IPv4 IP address in dotted format.
cool::ng::net::ipv4::binary_t _parse(std::istream& is)
{
  binary_t result;
  int count;

  for (count = 0; count < 4; ++count)
  {
    unsigned int aux;
    char c = parse_num(is, aux);
    if (aux > 255)
      throw exception::illegal_argument("value of IPv4 address element cannot exceed 255");
    result[count] = aux;

    if (c != '.' && count < 3)
      throw exception::illegal_argument("unparsable IPv4 address in input stream");
  }

  if (count < 4)
    throw exception::illegal_argument("unparsable IPv4 address in input stream");

  if (!is.eof())   // push back the last character if not EOF
    is.unget();

  return result;
}

void parse(std::istream& is, cool::ng::net::ipv4::host& val)
{
  val = static_cast<uint8_t*>(_parse(is));
}

void parse(std::istream& is, cool::ng::net::ipv4::network& val)
{
  unsigned int mask = 0;
  auto addr = _parse(is);
  char c = is.get();
  if (c == '/')   // network mask width is supposed to follow
  {
    ipv4::parse_num(is, mask);
    if (!is.eof())
      is.unget();
    if (mask > 32)
      throw exception::illegal_argument("Network mask width cannot exceed 32 bits");
  }
  else
  {
    mask = val.mask();
    if (!is.eof())
      is.unget();
  }
  val = cool::ng::net::ipv4::network(mask, addr);
}

void parse(std::istream& is, ip::address& val)
{
  if (val.kind() == ip::HostAddress)
    parse(is, dynamic_cast<cool::ng::net::ipv4::host&>(val));
  else
    parse(is, dynamic_cast<cool::ng::net::ipv4::network&>(val));
}

void str(uint8_t const val[], std::ostream& os)
{
  os << static_cast<int>(val[0]) << "."
     << static_cast<int>(val[1]) << "."
     << static_cast<int>(val[2]) << "."
     << static_cast<int>(val[3]);
}

} // namespace

// ===========================================================================
// ======
// ======
// ======  IPv6 utilities
// ======
// ======
// ===========================================================================
namespace ipv6
{

// NOTE: Term "quad" in below code refers to two-byte quantity which value
//       can be visualized with four hex digits.

//
//    byte        0         1
//           +----+----+----+----+
//           |         |         |
//           +----+----+----+----+
//   nibble    0    1    2    3
class quad_wrap
{
 public:
  // NOTE const_cast!!! It's dirty but it works.
  quad_wrap(const uint8_t* data) : m_data(const_cast<uint8_t*>(data)) { /* noop */ }
  operator uint16_t () const { return (m_data[0] << 8) | m_data[1]; }

  // Return a nibble (four bits) from the quad.
  int operator [] (int index) const
  {
    return (index & 0x01) == 0 ? m_data[index >> 1] >> 4 : m_data[index >> 1] & 0xF;
  }

  // Assign value to nibble 3 (rightmost nibble)
  void operator =(int value)
  {
    m_data[1] |= (value & 0x0F);
  }

  void operator <<=(int n)
  {
    n &= 0x03;   // Doesn't make sense to shift more than four times
    for (int i = 0; i < n; ++i)
      shl();
  }

 private:
  void shl()
  {
    m_data[0] <<= 4;
    m_data[0] |= ((m_data[1] >> 4) & 0x0F);
    m_data[1] <<= 4;
  }

 private:
  uint8_t* m_data;
};

std::ostream& operator <<(std::ostream& os, const quad_wrap& q)
{
  if (q == 0)
    os << '0';
  else
  {
    int ndx = 0;
    for (ndx = 0; q[ndx] == 0; ++ndx)    // skip leading zeros
    ;
    for ( ; ndx < 4; ++ndx)
      os << hex_digits[q[ndx]];
  }
  return os;
}

void find_longest_0_quad_sequence(uint8_t const val[], int& start, int& length)
{
  int index = 0;
  int limit = length;
  int cur_start = -1;

  length = 0;
  start = -1;

  for (index = 0; index < limit; )
  {

    // skip non-zero quads
    for ( ; index < limit && quad_wrap(&val[index]) != 0; index += 2)
    ;

    cur_start = index;
    for (index += 2; index < limit && quad_wrap(&val[index]) == 0; index += 2)
    ;

    int cur_len = (index - cur_start) / 2;
    if (cur_len > length)
    {
      start = cur_start;
      length = cur_len;
    }
  }

  if (length < 2)
  {
    length = 0;
    start = -1;
  }
}

// Translate binary IPv6 address into canonical textual presentation as
// specified in RFC5952
//
// Parameters
//   - os    Output stream
//   - size  Number of bytes to translate; this is necessary to reuse this
//           function for dotted-quad presentation where the first part is
//           visualized as canonical IPv6 address and last 8 bytes in
//           dotted IPv4 format
void str_canonical(uint8_t const val[], char delim, std::ostream& os, std::size_t size)
{
  int zero_start, zero_length = static_cast<int>(size);

  find_longest_0_quad_sequence(val, zero_start, zero_length);

  if (zero_start == -1)
  {
    for (int i = 0; i < size; i += 2)
    {
      if (i > 0)
        os << delim;
      os << quad_wrap(&val[i]);
    }
  }
  else
  {
    int i;

    for (i = 0; i < zero_start; i += 2)
    {
      if (i > 0)
        os << delim;
      os << quad_wrap(&val[i]);
    }
    os << delim;
    i = zero_start + zero_length * 2;
    if (i >= size)
    {
      os << delim;
    }
    else
    {
      for ( ; i < size; i += 2)
      {
        if (i > 0)
          os << delim;
        os << quad_wrap(&val[i]);
      }
    }
  }
}

void str_expanded(uint8_t const val[], std::ostream& os, std::size_t size)
{

  os << quad_wrap(&val[0]);

  for (int i = 2; i < size; i += 2)
  {
    os << ':';
    os << quad_wrap(&val[i]);
  }
}

void str_microsoft(uint8_t const val[], std::ostream& os, std::size_t size)
{
  str_canonical(val, '-', os, size);
}

bool parse_ishex(char c)
{
  // don't like compilers that enforce style through default enabled warnings!!!
  return (c >= '0' && c <= '9') ||  (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int parse_hexval(char c)
{
  if (c >= '0' && c <='9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  throw exception::illegal_argument("character is not hex digit");
}

void parse_quad(std::istream& is, uint8_t* q_ptr)
{
  bool first = true;
  quad_wrap q(q_ptr);

  for (char c = is.get(); !is.eof(); c = is.get())
  {
    if (!parse_ishex(c))
    {
      is.unget();
      break;
    }
    if (first)
      first = false;
    else
      q <<= 1;
    q = parse_hexval(c);
  }
}

cool::ng::net::ipv6::binary_t _parse(std::istream& is)
{
  cool::ng::net::ipv6::binary_t result, post_compress_value;
  bool compress_begin = false;
  bool post_compress = false;
  bool brackets = false;
  bool first = true;
  char c;
  char delim = '?';
  int index = 0;
  int post_compress_index = 0;
  bool loop = true;

  while (loop)
  {
    c = is.get();
    if (is.eof())
      break;

    switch (c)
    {
      case '[':
        if (first)
          brackets = true;
        else
          throw exception::illegal_state("Unexpected '[' in input stream");
        break;

      case ':':
      case '-':
        if (delim == '?')
          delim = c;
        else if (c != delim)
          throw exception::illegal_state("Cannot mix ':' and '-' as quad delimiters");
        if (compress_begin)
        {
          compress_begin = false;
          if (post_compress)
            throw exception::illegal_state("Cannot compress zero quads twice");
          post_compress = true;
        }
        else
        {
          compress_begin = true;
        }
        break;

      case '.': // seems to be dotted-quad ... some damage was already done, undo it
        if (delim == '-')  // ... but can't have it with Microsoft notation
        {
          is.unget();
          loop = false;
          break;
        }

        {
          int quad_val;
          // 1. one quad too many is already stored in one of binary buffers,
          //    get its value and clear this quad
          if (post_compress)
          {
            post_compress_index -= 2;
            quad_wrap q(&post_compress_value[post_compress_index]);
            quad_val = q;
            q = 0;
          }
          else
          {
            index -= 2;
            quad_wrap q(&result[index]);
            quad_val = q;
            q = 0;
          }
          // construct stringstream with this value and copy over the
          std::stringstream ss;
          ss << std::hex << quad_val << '.';
          for (c = is.get(); !is.eof() && (c == '.' || (c >= '0' && c <= '9')); c = is.get())
            ss << c;

          cool::ng::net::ipv4::host addr;
          ipv4::parse(ss, addr);
          if (post_compress)
          {
            post_compress_value.set(post_compress_index, static_cast<uint8_t*>(addr), 4);
            post_compress_index+= 4;
          }
          else
            result.set(12, static_cast<uint8_t*>(addr), 4);
          loop = false;
          break;
        }
      case ']':
        if (brackets)
          loop = false;
        else
          throw exception::illegal_state("unexpected ']' in input stream");
        break;

      default:
        compress_begin = false;
        is.unget();
        if (!parse_ishex(c))
        {
          loop = false;
          break;
        }

        if (post_compress)
        {
          parse_quad(is, &post_compress_value[post_compress_index]);
          post_compress_index += 2;
        }
        else
        {
          parse_quad(is, &result[index]);
          index += 2;
        }
        break;
    }
  }
  if (post_compress_index > 0)
    result.set(static_cast<int>(result.size()) - post_compress_index, post_compress_value);
  return result;
}

void parse(std::istream& is, cool::ng::net::ipv6::host& val)
{
  val = static_cast<uint8_t*>(_parse(is));
}

void parse(std::istream& is, cool::ng::net::ipv6::network& val)
{
  unsigned int mask = 0;
  auto addr = _parse(is);
  char c = is.get();
  if (c == '/')   // network mask width is supposed to follow
  {
    ipv4::parse_num(is, mask);
    if (!is.eof())
      is.unget();
    if (mask > 128)
      throw exception::illegal_argument("Network mask width cannot exceed 128 bits");
  }
  else
  {
    mask = val.mask();
    if (!is.eof())
      is.unget();
  }

  val =  cool::ng::net::ipv6::network(mask, addr);
}

void parse(std::istream& is, ip::address& val)
{
  if (val.kind() == ip::HostAddress)
    parse(is, dynamic_cast<cool::ng::net::ipv6::host&>(val));
  else
    parse(is, dynamic_cast<cool::ng::net::ipv6::network&>(val));
}

} // namespace

} // namespace entrails

namespace ip {

address::operator struct in_addr() const
{
  throw exception::bad_conversion("Can't convert IPv6 address to struct in_addr");
}

address::operator struct in6_addr() const
{
  throw exception::bad_conversion("Can't convert IPv4 address to struct in6_addr");
}

address& address::operator =(const in_addr&)
{
  throw exception::bad_conversion("Can't assign struct in_addr to IPv6 address");
}

address& address::operator =(const in6_addr&)
{
  throw exception::bad_conversion("Can't assign struct in6_addr to IPv4 address");
}

} // namespace

namespace ipv6
{

// ============= host class

host::host(const in_addr& data)
{
  m_data = static_cast<const uint8_t*>(rfc_ipv4map);
  ::memcpy(&m_data[12], &data, 4);
}

host::host(const std::string& str)
{
  std::stringstream ss(str);
  ss >> *this;
}

host::host(const ip::address& other)
{
  if (kind() != other.kind() || version() != other.version())
    throw exception::illegal_argument("input address is not an IPv6 host address");
  m_data = static_cast<const uint8_t*>(other);
}

bool host::equals(const ip::address &other) const
{
  if (kind() != other.kind())
    return false;

  if (other.version() == version())
    return m_data == static_cast<const uint8_t*>(other);

  // special treatment for IPv4 mapped addresses
  if (kind() == ip::HostAddress)
  {
    if (in(rfc_ipv4map) || in(rfc_ipv4translate))
      return ::memcmp(&m_data[12], static_cast<const uint8_t*>(other), 4) == 0;
  }
  return false;
}

void host::visualize(std::ostream& os) const
{
  // Stateless IP/ICMP Translation prefixed addresses are shown in
  // quad-dotted format
  if (in(rfc_ipv4map) || in(rfc_ipv4translate))
    visualize(os, DottedQuad);
  else
    detail::ipv6::str_canonical(m_data, ':', os, 16);
}

host::operator struct in_addr() const
{
  struct in_addr result;

  if (in(rfc_ipv4map) || in(rfc_ipv4translate))
    ::memcpy(static_cast<void*>(&result), m_data+12, 4);
  else
    throw exception::illegal_argument("Not an IPv4 mapped address");

  ::memcpy(static_cast<void*>(&result), m_data+12, 4);
  return result;
}

host::operator struct in6_addr() const
{
  struct in6_addr result;

  ::memcpy(static_cast<void*>(&result), m_data, m_data.size());
  return result;
}

host::operator std::string() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

void host::visualize(std::ostream& os, Style style) const
{
  switch (style)
  {
    case Canonical:
      visualize(os);
      break;

    case StrictCanonical:
      detail::ipv6::str_canonical(m_data, ':',  os, 16);
      break;

    case Expanded:
      detail::ipv6::str_expanded(m_data, os, 16);
      break;

    case Microsoft:
      detail::ipv6::str_microsoft(m_data, os, 16);
      break;

    case DottedQuad:
      detail::ipv6::str_canonical(m_data, ':', os, 12);
      os << ':';
      detail::ipv4::str(&m_data[12], os);
      break;

    default:
      break;
  }
}

ip::address& host::operator =(const in_addr& data)
{
  m_data = static_cast<const uint8_t*>(rfc_ipv4map);
  ::memcpy(&m_data[12], &data, 4);
  return *this;
}

ip::address& host::operator =(const in6_addr& data)
{
  m_data = static_cast<const uint8_t*>(static_cast<const void*>(&data));
  return *this;
}


void host::assign(const ip::address& val)
{
  if (val.kind() != kind())
    throw exception::illegal_argument("Can't assign network address to host");
  switch (val.version())
  {
    case ip::IPv6:
      m_data = static_cast<const uint8_t*>(val);
      break;

    case ip::IPv4:
      m_data = static_cast<const uint8_t*>(rfc_ipv4map);
      ::memcpy(&m_data[size() - val.size()], static_cast<const uint8_t*>(val), val.size());
      break;
  }
}

ip::address& host::operator =(const std::string& arg)
{
  std::stringstream ss(arg);
  auto aux = *this;

  try { ss >> *this; }
  catch (...) { *this = aux; throw; }

  return *this;
}

bool host::is(ip::Attribute a) const
{
  switch (a)
  {
    case ip::Loopback:
      return *this == loopback;
    case ip::Unspecified:
      return *this == unspecified;
    case ip::Ipv4Mapped:
      return in(rfc_ipv4map) || in(rfc_ipv4translate);
    case ip::Assigned:
      return detail::is_assigned(detail::ipv6::assigned_list,
                                   detail::ipv6::assigned_list_size,
                                   *this);
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

network::network(std::size_t mask_size, uint8_t const data[])
    : m_data(data)
    , m_length(mask_size)
{
  if (mask_size > size() * 8)
    throw exception::illegal_argument("Network mask size cannot exceed 128 bits");
  // zero host part of the address
  m_data = m_data & detail::calculate_mask<16>(m_length);
}

network::network(std::size_t mask_size, std::initializer_list<uint8_t> args)
  : m_data(args)
  , m_length(mask_size)
{
  if (mask_size > size() * 8)
    throw exception::illegal_argument("Network mask size cannot exceed 128 bits");
  // zero host part of the address
  m_data = m_data & detail::calculate_mask<16>(m_length);
}

network::network(std::size_t mask_size, const in6_addr& data)
    : m_data(static_cast<const uint8_t*>(static_cast<const void*>(&data)))
    , m_length(mask_size)
{
  if (mask_size > size() * 8)
    throw exception::illegal_argument("Network mask size cannot exceed 128 bits");
  // zero host part of the address
  m_data = m_data & detail::calculate_mask<16>(m_length);
}

network::network(const std::string& str) : m_length(0)
{
  // NOTE: parser takes care not to corrupt existing values upon error.
  //       Since it does copy construction it also applies the mask
  std::stringstream ss(str);

  ss >> *this;
}

network::network(const ip::address& data)
{
  if (kind() != data.kind() || version() != data.version())
    throw exception::illegal_argument("input address is not an IPv6 address");
  m_data = static_cast<const uint8_t*>(data);
  m_length = dynamic_cast<const network&>(data).m_length;
}

bool network::equals(const ip::address &other) const
{
  if (kind() != other.kind() || version() != other.version())
    return false;
  return m_data == static_cast<const uint8_t*>(other)
      && m_length == dynamic_cast<const network&>(other).m_length;
}

network::operator std::string() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

network::operator struct in6_addr() const
{
  struct in6_addr result;

  ::memcpy(static_cast<void*>(&result), m_data, m_data.size());
  return result;
}

void network::assign(const ip::address& data)
{
  if (kind() != data.kind() || version() != data.version())
    throw exception::illegal_argument("Can only assign ipv6::network object.");
  m_data = static_cast<const uint8_t*>(data);
  m_length = dynamic_cast<const network&>(data).m_length;

  // zero host part of the address
  m_data = m_data & detail::calculate_mask<16>(m_length);
}

ip::address& network::operator =(const in6_addr& data)
{
  m_data = static_cast<const uint8_t*>(static_cast<const void*>(&data));
  // zero host part of the address
  m_data = m_data & detail::calculate_mask<16>(m_length);
  return *this;
}

ip::address& network::operator =(const std::string& arg)
{
  std::stringstream ss(arg);
  network aux(this->mask());
  ss >> aux;
  
  *this = aux;
  return *this;
}

bool network::is(ip::Attribute a) const
{
  switch (a)
  {
    case ip::Loopback:
      return false;
    case ip::Unspecified:
      return false;
    case ip::Ipv4Mapped:
      return false;
    case ip::Assigned:
      return detail::is_assigned(detail::ipv6::assigned_list,
                                   detail::ipv6::assigned_list_size,
                                   *this);
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

void network::visualize(std::ostream &os) const
{
  detail::ipv6::str_canonical(m_data, ':', os, 16);
  os << "/" << m_length;
}


} // namespace

// ==========================================================================
//
// IPv4 implementation
//
// ==========================================================================


namespace ipv4 {
// ============= host class


host::host(const std::string& str)
{
  std::stringstream ss(str);

  ss >> *this;
}

host::host(const in6_addr& data)
{
  if (::memcmp(&data, static_cast<const uint8_t*>(ipv6::rfc_ipv4map), 12) == 0 ||
      ::memcmp(&data, static_cast<const uint8_t*>(ipv6::rfc_ipv4translate), 12) == 0)
    m_data = static_cast<const uint8_t*>(static_cast<const void*>(&data)) + 12;
  else
    throw exception::illegal_argument("argument is not IPv4 mapped address");
}

host::host(const ip::address& other)
{
  if (kind() != other.kind())
    throw exception::illegal_argument("argument is not host address");
  if (other.version() == ip::IPv6)
  {
    if (!(other.in(ipv6::rfc_ipv4map) || other.in(ipv6::rfc_ipv4translate)))
      throw exception::illegal_argument("argument is not an IPv4 mapped address");
    m_data = static_cast<const uint8_t*>(other) + 12;
  }
  else
    m_data = static_cast<const uint8_t*>(other);
}

host::host(uint32_t addr)
{
  auto aux = ntohl(addr);
  m_data = static_cast<uint8_t*>(static_cast<void*>(&aux));
}

bool host::equals(const ip::address &other) const
{
  if (kind() != other.kind())
    return false;

  if (other.version() == version())
    return m_data == static_cast<const uint8_t*>(other);

  // special treatment for IPv4 mapped addresses
  if (kind() == ip::HostAddress)
  {
    if (other.in(ipv6::rfc_ipv4map) || other.in(ipv6::rfc_ipv4translate))
      return ::memcmp(m_data, static_cast<const uint8_t*>(other) + 12, 4) == 0;
  }

  return false;
}

void host::visualize(std::ostream& os) const
{
  detail::ipv4::str(m_data, os);
}

ip::address& host::operator =(const in_addr& data)
{
  m_data = static_cast<const uint8_t*>(static_cast<const void*>(&data));
  return *this;
}

ip::address& host::operator =(const in6_addr& data)
{
  ipv6::host aux = ipv6::host(data);

  if (!(aux.in(ipv6::rfc_ipv4map) || aux.in(ipv6::rfc_ipv4translate)))
    throw exception::illegal_argument("address is not IPv4 host address mapped to IPv6");
  m_data = static_cast<const uint8_t*>(static_cast<const void*>(&data)) + 12;
  return *this;
}

host::operator struct in_addr() const
{
  struct in_addr result;

  ::memcpy(static_cast<void*>(&result), m_data, m_data.size());
  return result;
}

host::operator struct in6_addr() const
{
  struct in6_addr result;

  ::memcpy(static_cast<void*>(&result), static_cast<const uint8_t*>(ipv6::rfc_ipv4map), 12);
  ::memcpy(static_cast<uint8_t*>(static_cast<void*>(&result)) + 12, m_data, 4);
  return result;
}


host::operator std::string() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

ip::address& host::operator =(const std::string& arg)
{
  std::stringstream ss(arg);
  auto aux = *this;

  try { ss >> *this; }
  catch (...) { *this = aux; throw; }

  return *this;
}

void host::assign(const ip::address& val)
{
  if (kind() != val.kind())
    throw exception::illegal_argument("Cannot assign network address to host");
  switch (val.version())
  {
    case ip::IPv4:
      m_data = static_cast<const uint8_t*>(val);
      break;

    case ip::IPv6:
      if (!(val.in(ipv6::rfc_ipv4map) || val.in(ipv6::rfc_ipv4translate)))
        throw exception::illegal_argument("Cannot assign IPv6 host address");
      m_data = static_cast<const uint8_t*>(val) + (val.size() - size());
      break;
  }
}

bool host::is(ip::Attribute a) const
{
  switch (a)
  {
    case ip::Loopback:
      return *this == loopback;
    case ip::Unspecified:
      return *this == any;
    case ip::Ipv4Mapped:
      return false;
    case ip::Assigned:
      return detail::is_assigned(detail::ipv4::assigned_list,
                                   detail::ipv4::assigned_list_size,
                                   *this);
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


network::network(std::size_t mask_size, uint8_t const data[])
    : m_data(data)
    , m_length(mask_size)
{
  if (mask_size > size() * 8)
    throw exception::illegal_argument("Network mask size cannot exceed 32 bits");
  m_data = m_data & detail::calculate_mask<4>(m_length);
}

network::network(std::size_t mask_size, std::initializer_list<uint8_t> args)
  : m_data(args)
  , m_length(mask_size)
{
  if (mask_size > size() * 8)
    throw exception::illegal_argument("Network mask size cannot exceed 32 bits");
  m_data = m_data & detail::calculate_mask<4>(m_length);
}

network::network(std::size_t mask_size, const in_addr& data)
    : m_data(static_cast<const uint8_t*>(static_cast<const void*>(&data)))
    , m_length(mask_size)
{
  if (mask_size > size() * 8)
    throw exception::illegal_argument("Network mask size cannot exceed 32 bits");
  // zero host part of the address
  m_data = m_data & detail::calculate_mask<4>(m_length);
}

network::network(const std::string& str) : m_length(0)
{
  // NOTE: parser takes care not to corrupt existing values upon error.
  //       Since it does copy construction it also applies the mask
  std::stringstream ss(str);

  ss >> *this;
}

network::network(const ip::address& data)
{
  if (kind() != data.kind() || version() != data.version())
    throw exception::illegal_argument("input address is not an IPv4 address");
  m_data = static_cast<const uint8_t*>(data);
  m_length = dynamic_cast<const network&>(data).m_length;
}

bool network::equals(const ip::address &other) const
{
  if (kind() != other.kind() || version() != other.version())
    return false;
  return m_data == static_cast<const uint8_t*>(other)
      && dynamic_cast<const network&>(other).m_length;//TODO:????
}

void network::visualize(std::ostream &os) const
{
  detail::ipv4::str(m_data, os);
  os << "/" << m_length;
}

network::operator std::string() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

ip::address& network::operator =(const in_addr& data)
{
  m_data = static_cast<const uint8_t*>(static_cast<const void*>(&data));
  return *this;
}

network::operator struct in_addr() const
{
  struct in_addr result;

  ::memcpy(static_cast<void*>(&result), m_data, m_data.size());
  return result;
}

ip::address& network::operator =(const std::string& arg)
{
  std::stringstream ss(arg);
  network aux(this->mask());
  ss >> aux;

  *this = aux;
  return *this;
}

bool network::is(ip::Attribute a) const
{
  switch (a)
  {
    case ip::Loopback:
      return false;
    case ip::Unspecified:
      return false;
    case ip::Ipv4Mapped:
      return false;
    case ip::Assigned:
      return detail::is_assigned(detail::ipv4::assigned_list,
                                   detail::ipv4::assigned_list_size,
                                   *this);
  }
  return false;
}


void network::assign(const ip::address& data)
{
  if (kind() != data.kind() || version() != data.version())
    throw exception::illegal_argument("Can only assign ipv4::network object.");
  m_data = static_cast<const uint8_t*>(data);
  m_length = dynamic_cast<const network&>(data).m_length;
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
