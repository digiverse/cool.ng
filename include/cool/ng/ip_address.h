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

#if !defined(cool_ng_d2aa94ac_15ec_4748_9d69_a7d196d1b861)
#define      cool_ng_d2aa94ac_15ec_4748_9d69_a7d196d1b861

#include <iostream>
#include <string>
#include <cstdint>

#if defined(WINDOWS_TARGET)
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <sys/types.h>
# include <netinet/in.h>
#endif

#include "cool/ng/impl/platform.h"
#include "cool/ng/exception.h"
#include "cool/ng/binary.h"
#include "impl/ip_address.h"

namespace cool { namespace ng {

namespace net {

#if defined(WINDOWS_TARGET)

using handle = SOCKET;
const handle invalid_handle = INVALID_SOCKET;

#else

using handle = int;
const handle invalid_handle = -1;

#endif

/**
 * @defgroup net-global Global Operator Overloads
 */
/**
 * This namespace contains IP network related classes.
 *
 * <b>Global Operator Overloads</b><br>
 * - @ref operator ==(const cool::ng::net::ip::address& lhs, const cool::ng::net::ip::address& rhs) "comparison operator =="
 * - @ref operator !=(const cool::ng::net::ip::address& lhs, const cool::ng::net::ip::address& rhs) "comparison operator !="
 * - @ref operator <<(std::ostream&, const cool::ng::net::ip::address&) "stream operator <<"
 * - @ref operator >>(std::istream&, cool::ng::net::ip::address&) "stream operator >>"
 */
namespace ip {

/**
 * Enumeration determining the version of the IP protocol.
 */
enum class version {
  ipv4,  //!< IP address is 32-bit IPv4 address
  ipv6   //!< IP address is 128-bit IPv6 address
};

/**
 * Enumeration determining the kind of the IP address.
 */
enum class kind {
  host,   //!< The address is the host address
  network //!< The address is the network address
};

/**
 * Enumeration listing possible attributes of IP address.
 */
enum class attribute
{
  loopback,     //!< IP address is a loopback address of device
  unspecified,  //!< IP address is not specified (includes IPv4 INADDR_ANY)
  ipv4mapped,   //!< IP address is IPv6 address mapped from IPv4 space
  assigned      //!< IP address is reserved number by one or more RFCs
};

enum class protocol
{
  tcp,
  udp
};

class network;
class host;

class address
{
 public:
  /**
   * Assignment operator.
   *
   * Assigns the IP address contained in another address object to this
   * address object. In general both IP address object must be of the same
   * kind and the same IP protocol version, with the following exceptions:
   *  - it is possible to assign address contained in @ref ipv4::host "IPv4 host"
   *    address object to the @ref ipv6::host "IPv6 host" address object. In this
   *    case the resulting IPv6 address object will get the @ref
   *    ipv6::rfc_ipv4map "standard SIIT prefix" <tt>::%ffff:0:0/96</tt>
   *  - it is possible to assign address contained in @ref ipv6::host "IPv6 host"
   *    address object to the @ref ipv4::host "IPv4 host" address object if the
   *    provided IPV6 host address is mapped or translated
   *    @ref cool::ng::net::ip::version::ipv4 "IPv4" host address.
   *
   * @param rhs the IP address to be assigned to this address object
   *
   * @exception exception::illegal_argument if the provided IP address cannot
   *   be assigned to this address object
   *
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
   */
  virtual address& operator =(const address& rhs)
  {
    assign(rhs); return *this;
  }
  virtual ~address() { /* noop */ }
  /**
   * Address comparison.
   *
   * @return true if two address objects contain the same IP address
   * @return false if IP addresses in two address objects are different
   *
   * @note In general, two addresses that differ in kind() or version() do
   *   not compare equal. However, the @ref cool::ng::net::ipv6::host "IPv6 host" addresses
   *   that represent the @ref ipv4::host "IPv4 host" addresses mapped into
   *   @ref cool::ng::net::ip::version::ipv6 "IPv6" address space with either
   *   @ref ipv6::rfc_ipv4map or @ref ipv6::rfc_ipv4translate network prefix
   *   will compare equal to the corresponding @ref cool::ng::net::ip::version::ipv4 "IPv4" addresses.
   *
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
   * @see @ref cool::ng::net::ip::operator ==(const cool::ng::net::ip::address&, const cool::ng::net::ip::address&)
   *   "== operator" overload
   * @see @ref cool::ng::net::ip::operator !=(const cool::ng::net::ip::address&, const cool::ng::net::ip::address&)
   *   "global != operator" overload
   */
  virtual bool equals(const address& other) const = 0;
  /**
   * Translate the IPv6 address into textual format.
   *
   * Translates the IPv6 address into textual format for visualization. The
   * chosen format depends on the address and is one of the following:
   *
   * - standard dotted format for all IPv4 addresses, e.g. <tt>203.0.2.15</tt>
   * - canonical format as defined in RFC 5952 for most IPv6 addresses, e.g.
   *   <tt>2001:db8:8:2::15</tt>
   * - dotted-quad format for IPv6 host addresses that were mapped or translated
   *   from the IPv4 address, e.g. <tt>::%ffff:203.0.2.15</tt>
   *
   * @param os Output stream to write the visual presentation to
   *
   * @see @ref cool::ng::net::ip::operator <<(std::ostream& os, const cool::ng::net::ip::address& val)
   *   "<< operator" overload
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
  */
  virtual void visualize(std::ostream& os) const = 0;
  /**
   * Return the version of IP address.
   *
   * @return IPv4 This address object represents IPv4 address
   * @return IPV6 This address object represents IPV6 address
   */
  virtual ip::version version() const = 0;
  /**
   * Return the kind of the IP address.
   *
   * @return HostAddress This address object represents host address.
   * @return NetworkAddress This address object represents network address.
   */
  virtual ip::kind kind() const = 0;
  /**
   * Type conversion operator.
   *
   * @return Pointer to internal array of bytes containing binary
   *   representation of an IP address. The array is size() bytes long.
   */
  virtual EXPLICIT_ operator const uint8_t * () const = 0;
  /**
   * Type conversion operator.
   *
   * @return Pointer to internal array of bytes containing binary
   *   representation of an IP address. The array is size() bytes long.
   */
  virtual EXPLICIT_ operator uint8_t * () = 0;
  /**
   * Type conversion operator.
   *
   * @return <tt>struct in_addr</tt> filled with IPv4 address represented by
   *   this address object.
   *
   * @exception exception::bad_conversion if this address object is not
   *   IPv4 address or is not IPv6 host address mapped or translated from
   *   the IPv4 address.
   *
   * @note The IPv6 host addresses that are IPv4 mapped or translated host
   *   addresses can be expressed using <tt>struct in_addr</tt> structure.
   *   Such addresses are converted to IPv4 address structure by using the
   *   rightmost four bytes of the IPv6 host address.
   *
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
   */
  dlldecl virtual EXPLICIT_ operator struct in_addr() const;
  /**
   * Type conversion operator.
   *
   * @return <tt>struct in6_addr</tt> filled with IPv6 address represented by
   *   this address object.
   * @exception exception::bad_conversion if this address object is not
   *  IPv6 address or cannot be converted to one.
   *
   * @note IPv4 host addresses can be converted into an IPv6 address structure.
   * These addresses are prefixed with the @ref ipv6::rfc_ipv4map "standard
   *  network prefix" for IPv4 mapped addresses.
   *
   * @exception exception::illegal_argument thrown if this address is an
   *   IPv4 network address that cannot be converted to IPv6 address.
   *
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   */
  dlldecl virtual EXPLICIT_ operator struct in6_addr() const;
  /**
   * Type conversion operator.
   *
   * Converts the IP address into textual presentation. The IPv6 addresses are
   * presented using @ref ipv6::Canonical visual style.
   */
  virtual EXPLICIT_ operator std::string() const = 0;
  /**
   * Return size of the internal array.
   *
   * @return Number of <tt>uint8_t</tt> elements in internal array holding the
   *   binary representation of an IP address.
   */
  virtual std::size_t size() const = 0;
  /**
   * Assignment operator.
   *
   * Assigns the binary representation of IP address to this address object.
   *
   * @warning It is the user responsibility to ensure that the array contains
   *   at least size() elements and that they represent valid IP address of
   *   the respective IP protocol version.
   */
  virtual address& operator =(uint8_t const []) = 0;
  /**
   * Assignment operator.
   *
   * Assigns the IP address contained in <tt>struct in_addr</tt> to this
   * address object. If this address object is an IPv6 host address, the
   * supplied IPv4 address is mapped into IPv6 address using the
   * @ref ipv6::rfc_ipv4map "standard network prefix" for IPv4 mapped addresses.
   *
   * @exception exception::bad_conversion if this address object is
   *   neitherIPv4 address nor IPv6 host address.
   *
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   */
  dlldecl virtual address& operator =(const in_addr&);
  /**
   * Assignment operator.
   *
   * Assigns the IPv6 address contained in <tt>struct in6_addr</tt> to this
   * address object. If this address object is an IPv4 address object and
   * the supplied IPv6 address belongs to one of the IPv4 mapping networks,
   * the IPv4 address is set by using the trailing 4 bytes of the IPv6 address.
   *
   * @exception exception::bad_conversion thrown if
   *     - this address object is IPv4 host address but the provided IPv6 address
   *       does not belong to one of the IPv4 mapping IPv6 networks, or
   *     - this address object is IPv4 network address
   *
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
   */
  dlldecl virtual address& operator =(const in6_addr&);
  /**
   * Assignment operator
   *
   * Sets the IP address from the textual presentation.
   *
   * @exception exception::illegal_state The textual presentation is not parsable
   * @exception exception::illegal_argument The textual presentation contains
   *   invalid characters.
   */
  virtual address& operator =(const std::string&) = 0;
  /**
   * Determine whether address belongs to the specified network or not.
   *
   * Determines whether this IP address belongs to the specified network,
   * either a a host address on the network or as its sub-network.
   *
   * @return true if this address belongs to the specified network, false
   * otherwise.
   *
   * @note IPv6 addresses never belong to IPv4 networks and vice versa.
   */
  virtual bool in(const network& net) const = 0;
  /**
   * Determine whether IP address demonstrates specified attribute.
   *
   * @param attr Attribute to check.
   */
  virtual bool is(attribute attr) const = 0;

 protected:
  virtual void assign(const address&) = 0;
};

/**
 * Interface class representing host/interface address.
 */
class host : public address
{
 public:
  /**
   * Assignment operator.
   *
   * Assigns the IP address contained in another address object to this
   * host address object. In general both IP address object must be of the same
   * kind and the same IP protocol version, with the following exceptions:
   *  - it is possible to assign address contained in @ref ipv4::host "IPv4 host"
   *    address object to the @ref ipv6::host "IPv6 host" address object. In this
   *    case the resulting IPv6 address object will get the @ref
   *    ipv6::rfc_ipv4map "standard SIIT prefix" <tt>::%ffff:0:0/96</tt>
   *  - it is possible to assign address contained in @ref ipv6::host "IPv6 host"
   *    address object to the @ref ipv4::host "IPv4 host" address object if the
   *    provided IPV6 host address is mapped or translated IPv4 host address.
   *
   * @param rhs the IP address to be assigned to this address object
   *
   * @exception exception::illegal_argument if the provided IP address cannot
   *   be assigned to this address object
   *
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
   */
  address& operator =(const address& rhs) override = 0;
  virtual ip::kind kind() const override { return ip::kind::host; }
};

class network : public address
{
 public:
  /**
   * Assignment operator.
   *
   * Assigns the IP address contained in another address object to this
   * address object. The assignment will fail if the other address object is
   * not network address of the same @ref cool::ng::net::ip::version "IP version".
   *
   * @param rhs the IP address to be assigned to this address object
   *
   * @exception exception::illegal_argument if the provided IP address cannot
   *   be assigned to this address object
   *
   */
  address& operator =(const address& rhs) override = 0;
  /**
   * Determine whether IP address belongs to the network.
   *
   * @return true the specified IP address is either a host on this network
   *    or a sub-network of this network.
   *
   * @note IPv6 network does not contain IPv4 hosts or sub-networks, and vice versa.
   */
  virtual bool has(const address& address) const = 0;
  virtual ip::kind kind() const override { return ip::kind::network; }
  virtual std::size_t mask() const = 0;
};

} // namespace

/**
 * This namespace contains IPv6 network related classes.
 *
 * @see @ref net-global "Global operator overloads"
 */
namespace ipv6 {

/**
 * Enumeration selecting the visual style of IPv6 addresses.
 */
enum Style {
  /**
   * Canonical format as specified by RFC 5952. In this format the leading
   * zeros in each group of four hex digits (quad) are not shown and the longest
   * sequence of zero quads is compressed to '<tt>::</tt>'
   * (e.g. <tt>2001:db8::8.32</tt>). IPv4 mapped addresses shown in
   * @ref DottedQuad format.
   */
  Canonical,
  /**
   * Strictly canonical format as specified by RFC 5952 with no special
   * treatment for IPv4 mapped addresses.
   */
  StrictCanonical,
  /**
   * This style yields the visual presentation of each group of
   * four hexadecimal digits, but without leading zeros in the group. The
   * special network prefixes do not produce prefix specific presentations.
   * Example: <tt>2001:db8:0:0:0:0:8:32</tt>
   */
  Expanded,
  /**
   * This style yields a visual presentation Microsoft developed
   * for use in UNC addresses without <tt>.ipv6-literal.net</tt> suffix.
   * In this presentation dashes ('<tt>-</tt>') are used instead of colons
   * and no special visualization is used for special network prefixes.
   * Example: <tt>2001-db8-</tt><tt>-8-32</tt>
   */
  Microsoft,
  /**
   * This style enforces the use of IPv4 dotted notation for
   * the last 32 bits of the IPv6 address regardless of its network
   * prefix. Example: <tt>2001:db8:0.0.8.50</tt>
   */
  DottedQuad
};


typedef cool::ng::util::binary<16> binary_t;

/**
 * IPv6 host address implementation class.
 */
class host : public ip::host
{
 public:
  ip::address& operator =(const ip::address& rhs) override
  {
    assign(rhs);
    return *this;
  }

  /**
   * Construct a host address object with unspecified address
   */
  explicit host()
  { /* noop */ }

  /**
   * Construct a host address object with address from byte array.
   *
   * @warning The ctor uses the first 16 bytes of the array. Providing the byte
   *   array of less than 16 elements results in undefined behavior.
   */
  explicit host(uint8_t const data[]) : m_data(data)
  { /* noop */ }

  /**
   * Construct a host address object with address from initializer list.
   *
   * This constructor makes the following initialization possible:
   * @code
   * host h = { 0x20. 0x01, 0x00, 0x00,
   *            0xab, 0x00, 0x11, 0x20,
   *            0x00, 0x00, 0x00, 0x00,
   *            0x01, 0x02, 0x03, 0x04 };
   * @endcode
   * If the list contains fewer than 16 values the remaining values are
   * set to 0.
   */
  host(std::initializer_list<uint8_t> data) : m_data(data)
  { /* noop */ }

  /**
   * Construct a host address object from IPv4 address structure.
   *
   * @note The constructed object is an IPv4 mapped address with the
   *  @ref rfc_ipv4map "standard network prefix" for mapped addresses.
   */
  dlldecl explicit host(const struct in_addr& data);

  /**
   * Construct a host address object from IPv6 address structure.
   */
  explicit host(const in6_addr& data)
      : m_data(static_cast<const uint8_t*>(static_cast<const void*>(&data)))
  { /* noop */ }

  /**
   * Construct a host address object from textual presentation of address.
   *
   * @exception exception::illegal_state The textual presentation is not parsable
   * @exception exception::illegal_argument The textual presentation contains
   *   invalid characters.
   */
  dlldecl explicit host(const std::string& data);

  /**
   * Construct host address from another address object.
   *
   * @exception exception:: illegal_argument thrown if another address object
   *   is not IPv6 host or IPv4 host address object.
   *
   * @note If provided address object is an IPv4 host address the resulting
   *   IPv6 host address is prefixed with the
   *   @ref rfc_ipv4map "standard network prefix" for mapped addresses.
   */
  dlldecl explicit host(const ip::address& data);

  // address interface
  dlldecl bool equals(const ip::address& other) const override;
  dlldecl void visualize(std::ostream& os) const override;
  ip::kind kind() const override
  {
    return ip::kind::host;
  }
  ip::version version() const override
  {
    return ip::version::ipv6;

  }
  EXPLICIT_ operator const uint8_t * () const override
  {
    return m_data;
  }
  EXPLICIT_ operator uint8_t * () override
  {
    return m_data;
  }
  dlldecl EXPLICIT_ operator struct in_addr() const override;
  dlldecl EXPLICIT_ operator struct in6_addr() const override;
  dlldecl EXPLICIT_ operator std::string () const override;
  std::size_t size() const override
  {
    return m_data.size();
  };
  ip::address& operator =(uint8_t const data[]) override
  {
    m_data = data;
    return *this;

  }
  dlldecl ip::address& operator =(const in_addr&) override;
  dlldecl ip::address& operator =(const in6_addr&) override;
  dlldecl ip::address& operator =(const std::string&) override;
  dlldecl bool in(const ip::network& net) const override;
  dlldecl bool is(ip::attribute) const override;

  // ipv6::host specific
  /**
   * Translate the IPv6 address into textual format.
   *
   * Translates the IPv6 address into textual format for visualization using
   * the requested visual style.
   *
   * @param os    Output stream to use
   * @param style Visual style to use
   *
   * @see @ref cool::ng::net::ipv6::Style "Visual styles for IPv6 addresses"
   */
  dlldecl void visualize(std::ostream& os, Style style) const;

 private:
  dlldecl void assign(const ip::address& other) override;

 private:
  binary_t m_data;
};

class network : public ip::network
{
 public:
  ip::address& operator =(const ip::address& rhs) override
  {
    assign(rhs); return *this;
  }

  /**
   * Construct a network address object with unspecified address and mask size 0.
   */
  explicit network() : m_length(0)
  { /* noop */ }

  /**
   * Construct a network address object with specified mask size and unspecified address.
   */
  explicit network(std::size_t mask_size) : m_length(mask_size)
  { /* noop */ }

  /**
   * Construct a network address object with address from byte array.
   *
   * @param mask_size Network mask size, in bits
   * @param data      Byte array containing network address.
   * @exception exception::illegal_argument Thrown if mask size exceeds 128 bits.
   *
   * @warning The ctor uses the first 16 bytes of the array. Providing the byte
   *   array of less than 16 elements results in undefined behavior.
   */
  dlldecl explicit network(std::size_t mask_size, uint8_t const data[]);

  /**
   * Construct a host address object with address from initializer list.
   *
   * @param mask_size Network mask size, in bits
   * @param data      Initializer list containing network address bytes.
   * @exception exception::illegal_argument Thrown if mask size exceeds 128 bits.
   *
   * This constructor makes the following initialization possible:
   * @code
   * host h = { 96, { 0x00. 0x00, 0x00, 0x00,
   *                  0x00, 0x00, 0x00, 0x00,
   *                  0x00, 0x00, 0x00, 0x00,
   *                  0xff, 0xff } };
   * @endcode
   * If the list contains fewer than 16 values the remaining values are
   * set to 0. The first number (96 in above example) is the network mask size,
   * in bits.
   */
  dlldecl network(std::size_t mask_size, std::initializer_list<uint8_t> data);

  /**
   * Construct a host address object from IPv6 address structure.
   *
   * @param mask_size Network mask size, in bits
   * @param data      IPv6 structure with network address.
   * @exception exception::illegal_argument Thrown if mask size exceeds 128 bits.
   */
  dlldecl explicit network(std::size_t mask_size, const in6_addr& data);

  /**
   * Construct a network address object from textual presentation of address.
   *
   * @exception exception::illegal_state The textual presentation is not parsable
   * @exception exception::illegal_argument The textual presentation contains
   *   invalid characters.
   */
  dlldecl explicit network(const std::string& data);

  /**
   * Construct network address from another address object.
   *
   * @param data      Another network object.
   * @exception exception:: illegal_argument Thrown if another address object
   *   is not IPv6 network address object
   */
  dlldecl explicit network(const ip::address& data);

  // address interface
  dlldecl bool equals(const ip::address& other) const override;
  dlldecl void visualize(std::ostream &os) const override;
  ip::kind kind() const override
  {
    return ip::kind::network;

  }
  ip::version version() const override
  {
    return ip::version::ipv6;
  }
  EXPLICIT_ operator const uint8_t * () const override
  {
    return m_data;
  }
  EXPLICIT_ operator uint8_t * () override
  {
    return m_data;
  }
  dlldecl EXPLICIT_ operator struct in6_addr() const override;
  dlldecl EXPLICIT_ operator std::string () const override;
  dlldecl std::size_t size() const override
  {
    return m_data.size();
  };
  ip::address& operator =(uint8_t const data[]) override
  {
    m_data = data;
    return *this;
  }
  dlldecl ip::address& operator =(const in6_addr&) override;
  dlldecl ip::address& operator =(const std::string&) override;
  dlldecl bool in(const ip::network& net) const override;
  dlldecl bool is(ip::attribute) const override;

  // network interface
  dlldecl bool has(const ip::address& other) const override;
  std::size_t mask() const override
  {
    return m_length;
  }

 private:
  dlldecl void assign(const ip::address& other) override;

 private:
  binary_t m_data;
  std::size_t m_length;
};

/**
 * Constant representing IPv6 loopback address.
 */
const host loopback = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
/**
 * Constant representing IPv6 unspecified address.
 */
const host unspecified;
/**
 * Naming convenience - IPv6 equivalent of ipv4::any for bind.
 */
const host any;
/**
 * Reserved IPv6 address range ::%ffff:0:0/96.
 *
 * The IPv6 network prefix for IPv4 @ref ipv4::host "host" addresses
 * mapped into IPv6 address space via Stateless IP/ICMP Translation (SIIT)
 * mechanism, as specified in RFC 4291. Addresses within this range should
 * not appear on the public Internet.
 */
const network rfc_ipv4map= { 96, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff } };
/**
 * Reserved IPv6 address range 100::/64.
 *
 * This is Discard Prefix, as specified in RFC 6666.
 */
const network rfc_discard = { 64, { 0x01 } };
/**
 * Reserved IPv6 address range 64:ff9b::/96.
 *
 * This block is Well-known Prefix used for IPv4/Ipv6 address translation,
 * as specified in RFC 6052.
 */
const network rfc_ipv4translate = { 96, { 0x00, 0x64, 0xff, 0x9b } };
/**
 * Reserved IPv6 address range 2001::/32.
 *
 * This block is is used for Teredo tunneling, as specified in RFC 4380.
 *
 * @note Teredo tunneling is a technology that allows nodes located behind one or
 * more IPv4 Network Address Translations (NATs) to obtain IPv6
 * connectivity by tunneling packets over UDP.
 */
const network rfc_teredo = { 8, { 0x20, 0x01 } };
/**
 * Reserved IPv6 address range 2001:db8::/32.
 *
 * This block is is used for addresses used in documentation,
 * as specified in RFC 5737.
 */
const network rfc_doc = { 32, { 0x20, 0x01, 0xdb, 0x80 } };
/**
 * Reserved IPv6 address range fc00::/7.
 *
 * This block is is used for unique local addresses,
 * as specified in RFC 4193.
 */
const network rfc_local = { 7, { 0xfc } };
/**
 * Reserved IPv6 address range fe80::/10.
 *
 * This block is is reserved for link local addresses. The actual allocation
 * range for link local addresses is fe80::/64.
 */
const network rfc_link = { 10, { 0xfe, 0x80 } };
/**
 * Reserved IPv6 address range ff00::/8.
 *
 * This block is is reserved for multicast addresses. */
const network rfc_mcast = { 0, { 0xff } };

} // namespace ipv6

/**
 * This namespace contains IPv4 network related classes.
 *
 * @copydoc net-global
 */
namespace ipv4 {

typedef cool::ng::util::binary<4> binary_t;

/**
 * Implementation class for IPv4 addresses.
 *
 */

/**
 * IPv4 host address implementation class.
 */
class host : public ip::host
{
 public:
  ip::address& operator =(const ip::address& rhs) override
  {
    assign(rhs);
    return *this;
  }
  /**
   * Construct a host address object with unspecified address
   */
  explicit host()
  { /* noop */ }
  /**
   * Construct a host address object with address from byte array.
   *
   * @warning The ctor uses the first 4 bytes of the array. Providing the byte
   *   array of less than 4 elements results in undefined behavior.
   */
  explicit host(uint8_t const data[]) : m_data(data)
  { /* noop */ }
  /**
   * Construct a host address object with address from initializer list.
   *
   * This constructor makes the following initialization possible:
   * @code
   * host h = { 192, 168, 3, 20 }
   * @endcode
   * If the list contains fewer than 4 values the remaining values are
   * set to 0.
   */
  host(std::initializer_list<uint8_t> data) : m_data(data)
  { /* noop */ }
  /**
   * Construct a host address object from IPv4 address structure.
   */
  explicit host(const struct in_addr& data)
      : m_data(static_cast<const uint8_t*>(static_cast<const void*>(&data)))
  { /* noop */ }
  /**
   * Construct a host address object from IPv6 address structure.
   *
   * Such construction is only possible if the IPv6 address structure contains
   * IPv4 mapped or translated address.
   *
   * @exception exception:: illegal_argument Thrown if specified address
   *   is not valid.
   *
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
   */
  dlldecl explicit host(const in6_addr& data);
  /**
   * Construct a host address object from textual presentation of address.
   *
   * @exception exception::illegal_state The textual presentation is not parsable
   * @exception exception::illegal_argument The textual presentation contains
   *   invalid characters.
   */
  dlldecl explicit host(const std::string& data);
  /**
   * Construct host address from another address object.
   *
   * Valid parameters are IPv4 host addresses, or IPv6 host addresses that are
   * IPv4 mapped or translated addresses.
   *
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
   * @exception exception:: illegal_argument Thrown if argument address object
   *   is not valid.
   */
  dlldecl explicit host(const ip::address& data);

  dlldecl explicit host(uint32_t addr);

  // address interface
  dlldecl  bool equals(const ip::address& other) const override;
  dlldecl  void visualize(std::ostream& os) const override;
  ip::kind kind() const override
  {
    return ip::kind::host;
  }
  ip::version version() const override
  {
    return ip::version::ipv4;
  }
  EXPLICIT_ operator const uint8_t * () const override
  {
    return m_data;
  }
  EXPLICIT_ operator uint8_t * () override
  {
    return m_data;
  }
  dlldecl EXPLICIT_ operator struct in_addr() const override;
  dlldecl EXPLICIT_ operator struct in6_addr() const override;
  dlldecl EXPLICIT_ operator std::string() const override;
  std::size_t size() const override
  {
    return m_data.size();
  };
  ip::address& operator =(uint8_t const data[]) override
  {
    m_data = data;
    return *this;
  }
  dlldecl ip::address& operator =(const in_addr&) override;
  dlldecl ip::address& operator =(const in6_addr&) override;
  dlldecl ip::address& operator =(const std::string&) override;
  dlldecl bool in(const ip::network& net) const override;
  dlldecl bool is(ip::attribute) const override;

 private:
  dlldecl void assign(const ip::address& other) override;

 private:
  binary_t m_data;
};

/**
 * IPv4 network address implementation class.
 */
class network : public ip::network
{
 public:
  ip::address& operator =(const ip::address& rhs) override
  {
    assign(rhs); return *this;
  }
  /**
   * Construct a network address object with unspecified address and mask size 0.
   */
  explicit network() : m_length(0)
  { /* noop */ }
  /**
   * Construct a network address object with specified mask size and unspecified address.
   */
  explicit network(std::size_t mask_size) : m_length(mask_size)
  { /* noop */ }
  /**
   * Construct a network address object with address from byte array.
   *
   * @param mask_size Network mask size, in bits
   * @param data      Byte array containing network address.
   * @exception exception::illegal_argument Thrown if mask size exceeds 32 bits.
   *
   * @warning The ctor uses the first 4 bytes of the array. Providing the byte
   *   array of less than 4 elements results in undefined behavior.
   */
  dlldecl explicit network(std::size_t mask_size, uint8_t const data[]);
  /**
   * Construct a host address object with address from initializer list.
   *
   * @param mask_size Network mask size, in bits
   * @param data      Initializer list containing network address bytes.
   * @exception exception::illegal_argument Thrown if mask size exceeds 32 bits.
   *
   * This constructor makes the following initialization possible:
   * @code
   * host h = { 24, { 192, 168, 3, 0 } };
   * @endcode
   * If the list contains fewer than 4 values the remaining values are
   * set to 0. The first number (24 in above example) is the network mask size,
   * in bits.
   */
  dlldecl network(std::size_t mask_size, std::initializer_list<uint8_t> data);
  /**
   * Construct a host address object from IPv4 address structure.
   *
   * @param mask_size Network mask size, in bits
   * @param data      IPv4 structure with network address.
   * @exception exception::illegal_argument Thrown if mask size exceeds 32 bits.
   */
  dlldecl explicit network(std::size_t mask_size, const in_addr& data);
  /**
   * Construct a network address object from textual presentation of address.
   *
   * @exception exception::illegal_state The textual presentation is not parsable
   * @exception exception::illegal_argument The textual presentation contains
   *   invalid characters.
   */
  dlldecl explicit network(const std::string& data);
  /**
   * Construct network address from another address object.
   *
   * @param data      Another network object.
   * @exception exception:: illegal_argument Thrown if another address object
   *   is not IPv4 network address object
   */
  dlldecl explicit network(const ip::address& data);

  // address interface
  dlldecl bool equals(const ip::address& other) const override;
  dlldecl void visualize(std::ostream &os) const override;
  ip::kind kind() const override
  {
    return ip::kind::network;
  }
  ip::version version() const override
  {
    return ip::version::ipv4;
  }
  EXPLICIT_ operator const uint8_t * () const override
  {
    return m_data;
  }
  EXPLICIT_ operator uint8_t * () override
  {
    return m_data;
  }
  dlldecl EXPLICIT_ operator struct in_addr() const override;
  dlldecl EXPLICIT_ operator std::string () const override;
  std::size_t size() const override
  {
    return m_data.size();
  };
  ip::address& operator =(uint8_t const data[]) override
  {
    m_data = data;
    return *this;
  }
  dlldecl ip::address& operator =(const in_addr&) override;
  dlldecl ip::address& operator =(const std::string&) override;
  dlldecl bool in(const ip::network& net) const override;
  dlldecl bool is(ip::attribute) const override;

  // network interface
  dlldecl bool has(const ip::address& other) const override;
  std::size_t mask() const override
  {
    return m_length;
  }

 private:
  dlldecl void assign(const ip::address& other) override;

 private:
  binary_t m_data;
  std::size_t m_length;
};

/**
 * Constant representing IPv4 loopback address.
 */
const host loopback = { 127, 0, 0, 1 };
/**
 * Constant representing IPv4 INADDR_ANY address.
 */
const host any;

const host broadcast = { 255, 255, 255, 255 };
/**
 * Reserved IPv4 address range 0.0.0.0/8.
 *
 * Local broadcast source address as specified by RFC 1700.
 */
const network rfc_broadcast = { 8, { 0, 0, 0, 0} };  // TODO:
/**
 * Reserved IPv4 address range 10.0.0.0/24.
 *
 * 24-bit block private network (former class A) as specified by RFC 1918.
 */
const network rfc_private_24 = { 24, { 10 } };
/**
 * Reserved IPv4 address range 172.16.0.0/20.
 *
 * 20-bit block private network (formerly 16 class B private networks)
 * as specified by RFC 1918.
 */
const network rfc_private_20 = { 20, { 172, 16 } };
/**
 * Reserved IPv4 address range 192.168.0.0/16.
 *
 * 16-bit block private network (formerly 256 class C private networks)
 * as specified by RFC 1918.
 */
const network rfc_private_16 = { 16, { 192, 168 } };
/**
 * Reserved IPv4 address range 100.64.0.0/10.
 *
 * Reserved for use with Carrier-grade NAT, as specified by RFC 6598.
 */
const network rfc_carrier_nat = { 10, { 100, 64 } };
/**
 * Reserved IPv4 address range 127.0.0.0/8.
 *
 * Reserved for loopback addresses to the local host, as specified by
 * RFC 990.
 */
const network rfc_loopback = { 8, { 127 } };
/**
 * Reserved IPv4 address range 169.254.0.0/16.
 *
 * Reserved for dynamically auto-generated link local addresses used in situations
 * where stable IP address cannot be obtained by other means, as specified by
 * RFC 3927.
 */
const network rfc_unset = { 16, { 169, 254 } };
/**
 * Reserved IPv4 address range 192.0.0.0/24.
 *
 * Reserved for IANA IPv4 Special Purpose Address Registry, as specified by
 * RFC 5736.
 */
const network rfc_iana_private = { 24, { 192 } };
/**
 * Reserved IPv4 address range 192.0.2.0/24.
 *
 * Reserved as TEST-NET for use solely in documentation and example source
 * code, as specified by RFC 5737.
 */
const network rfc_test = { 24, { 192, 0, 2 } };
/**
 * Reserved IPv4 address range 198.51.100.0/24.
 *
 * Reserved as TEST-NET-2 for use solely in documentation and example source
 * code, as specified by RFC 5737.
 */
const network rfc_test_2 = { 24, { 198, 51, 100 } };
/**
 * Reserved IPv4 address range 203.0.113.0/24.
 *
 * Reserved as TEST-NET-3 for use solely in documentation and example source
 * code, as specified by RFC 5737.
 */
const network rfc_test_3 = { 24, { 203, 0, 113 } };
/**
 * Reserved IPv4 address range 192.88.99.0/24.
 *
 * Used by 6to4 anycast relays, as specified by RFC 3068.
 */
const network rfc_6to4_anycast = { 24, { 192, 88, 99 } };
/**
 * Reserved IPv4 address range 198.18.0.0/15.
 *
 * Used for testing inter-network communication between two separate
 * subnets, as specified by RFC 2544.
 */
const network rfc_test_comm = { 15, { 198, 18 } };
/**
 * Reserved IPv4 address range 224.0.0.0/4.
 *
 * Reserved as MCAST-TEST-NET for use solely in documentation and example source
 * code, as specified by RFC 5771.
 */
const network rfc_mcast = { 4, { 224 } };
/**
 * Reserved IPv4 address range 233.252.0.0/24.
 *
 * Reserved as MCAST-TEST-NET for use solely in documentation and example source
 * code, as specified by RFC 5771.
 */
const network rfc_test_mcast = { 24, { 233, 252 } };
/**
 * Reserved IPv4 address range 240.0.0.0/4.
 *
 * Reserved for future use, as specified by RFC 5771.
 */
const network rfc_future = { 4, { 240 } };

} // namespace ipv4

namespace detail {

namespace ipv6 {
extern void parse(std::istream&, cool::ng::net::ipv6::host&);
extern void parse(std::istream&, cool::ng::net::ipv6::network&);
extern void parse(std::istream&, cool::ng::net::ip::address&);
}

namespace ipv4 {
extern void parse(std::istream&, cool::ng::net::ipv4::host&);
extern void parse(std::istream&, cool::ng::net::ipv4::network&);
extern void parse(std::istream&, cool::ng::net::ip::address&);
}

}

namespace ip {
/**
 * @ingroup net-global
 * Binary compare two IP addresses.
 *
 * @return true if two addresses are binary equal
 * @return false if two addresses are binary different
 */
inline bool operator ==(const address& lhs, const address& rhs)
{
  return lhs.equals(rhs);
}
/**
 * @ingroup net-global
 * Binary compare two IP addresses.
 *
 * @return false if two addresses are binary equal
 * @return true if two addresses are binary different
 */
inline bool operator !=(const address& lhs, const address& rhs)
{
  return !lhs.equals(rhs);
}

/**
 * @ingroup net-global
 * Display IP address to the character stream.
 *
 * Generates a textual presentation of the IP address to the output stream.
 *
 * @see @ref cool::ng::net::ip::address::visualize() "address::visualize()" for more details.
 */
inline std::ostream& operator <<(std::ostream& os, const address& val)
{
  val.visualize(os);
  return os;
}
/**
 * @ingroup net-global
 * Read an IPv6 host address from the character stream.
 *
 * Parses the textual presentation of the IP address from the input stream. The
 * recognized presentation styles depend on the @ref cool::ng::net::ip::version "version"
 * and the @ref cool::ng::net::ip::kind "kind" of the provided
 * @ref cool::ng::net::ip::address "address" parameter as follows:
 *  - the @ref cool::ng::net::ip::version::ipv4 "IPv4" addresses are recognized in the
 *    usual dotted format, e.g. <tt>198.18.3.65</tt>
 *  - the @ref cool::ng::net::ip::version::ipv6 "IPv6" addresses are recognized in all styles
 *    as listed in @ref cool::ng::net::ipv6::Style "Style" enumeration. In addition,
 *    the IPv6 @ref cool::ng::net::ip::host "host" address may be enclosed in square
 *    brackets (<tt>[</tt> ... <tt>]</tt>), which are ignored.
 *
 * For @ref cool::ng::net::ip::network "network" addresses of any
 * @ref cool::ng::net::ip::version "version" the network mask width can be specified
 * by appending a <tt>/</tt> character followed by an integer number to the
 * address, e.g. <tt>::%ffff:0:0/96</tt>.
 *
 * The reading of the input stream will stop at the first character that cannot be
 * interpreted as a part of the address. This character will be the next
 * character available in the input stream.
 *
 * @exception cool::exception::illegal_argument Thrown if the address cannot
 *   be parsed.
 * @exception cool::exception::illegal_state Thrown if the address cannot
 *   be parsed.
 */
dlldecl std::istream& operator >>(std::istream& is, address& val);

/**
 * Host address container that can hold either IPv4 or IPv6 host address.
 *
 * This container is a replacement for cumbersone <tt>struct sockaddr_storage</tt>
 * address container.
 */
class host_container
{
 public:
  dlldecl host_container();
  dlldecl host_container(const host_container&);
  dlldecl host_container(const address&);
  dlldecl host_container(const in_addr&);
  dlldecl host_container(const in6_addr&);
  dlldecl host_container(const sockaddr_storage&);
  dlldecl const host_container& operator =(const host_container&);
  dlldecl const host_container& operator =(const address&);
  dlldecl const host_container& operator =(const in_addr&);
  dlldecl const host_container& operator =(const in6_addr&);
  dlldecl const host_container& operator =(const sockaddr_storage&);
  operator const address&() const { return *m_value; }
  explicit operator const address*() const { return m_value; }

 private:
  const address* m_value;
  ipv4::host m_v4;
  ipv6::host m_v6;
};

} // namespace ip


} } } // namespaces cool::ng::net


#endif
