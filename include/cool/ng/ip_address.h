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

namespace ip {

/**
 * Platform agnostic type representing network socket handle.
 */
#if defined(WINDOWS_TARGET)
using socket_handle = SOCKET;
#else
using socket_handle = int;
#endif
/**
 * Platform agnostic constant representing invalid socket handle.
 */
#if defined(WINDOWS_TARGET)
const socket_handle invalid_socket_handle = INVALID_SOCKET;
#else
const socket_handle invalid_socket_handle = -1;
#endif

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

/**
 * Enumeration lising the valid visual styles of IPv4 and IPv6 addresses.
 */
enum class style {
  /**
   * Denotes the visual style that is the most common for a specific IP address
   * @ref address::version() "version". This is @ref style::dot_decimal "dot-decimal" for IPv4 and
   * @ref style::canonical "canonical" for IPv6. Unless otherwise requested, the @ref ip
   * will always use the standard style to convert IP address to its textual presentation.
   */
  customary,
  /**
   * Visual style used for IPv4 addresses. This visual style consists of four decimal
   * numbers with values in the range 0-255 (byte values) seperated by dots. This style is used  for
   * both host and network addresses. Network addresses have a network mask size suffix, separated
   * by the forward slash ('<tt>/</tt>). Examples:
   *
   * <code>
   *   192.168.4.13
   *   172.17.128.0/15
   * </code>
   */
  dot_decimal,
  /**
   * Canonical style of writing the IPv6 addresses, as specified by RFC 5952. In this style the leading
   * zeros in each group of four hex digits (quad) are not shown and the longest
   * sequence of zero quads is compressed to '<tt>::</tt>'. IPv4 mapped host addresses are shown in
   * @ref style::dotted_quad "dotted-quad" style. This style can be used for both host and network  addresses.
   * Network addresses have a network mask size suffix, separated
   * by the forward slash ('<tt>/</tt>). Examples:
   *
   * <code>
   * 3:f12:1:1:ac4f:600:a0:3f40
   * ::1
   * ::%ffff:192.168.3.14
   * 12::%ac4f:600:0:0/96
   * </code>
   */
  canonical,
  /**
   * Strictly canonical style of writing the IPv6 address, as specified by RFC 5952. This visual style is similar
   * to @ref style::canonical but with no special treatment for IPv4 mapped addresses. This style can be
   * used for both host and network addresses. Network addresses have a network mask size suffix, separated
   * by the forward slash ('<tt>/</tt>). Examples:
   *
   * <code>
   * 3:f12:1:1:ac4f:600:a0:3f40
   * ::1
   * ::%ffff:c0:a8:3:e
   * 12::%ac4f:600:0:0/96
   * </code>
   */
  strict,
  /**
   * This style of writing the IPv6 address yields the visual presentation of each group of
   * four hexadecimal digits (quad), but without the  leading zeros in the group. The
   * special network prefixes do not produce prefix specific presentations. This style can be used for both
   * host and network addresses. Network addresses have a network mask size suffix, separated
   * by the forward slash ('<tt>/</tt>).
   * Examples:
   *
   * <code>
   * 3:f12:1:1:ac4f:600:a0:3f40
   * 0:0:0:0:0:0:0:1
   * 0:0:0:ffff:c0:a8:3:e
   * 12:0:0:0:ac4f:600:0:0/96
   * </code>
   */
  expanded,
  /**
   * This style of writing the IPv6 address  was developed by Microsoft
   * for use in UNC addresses without the <tt>.ipv6-literal.net</tt> suffix. The  style is based on
   * @ref style::strict but uses dashes ('<tt>-</tt>') instead of colons. This style can be used for both host
   * and network addresses. Network addresses have a network mask size suffix, separated
   * by the forward slash ('<tt>/</tt>). Examples:
   *
   * <code>
   * 3-f12-1-1-ac4f-600-a0-3f40
   * --1
   * --ffff-c0-a8-3-e
   * 12--ac4f-600-0-0/96
   * </code>
   */
  microsoft,
  /**
   * This visual style of writing the IPv6 address  is based on @ref style::canonical but enforces the use of
   * IPv4 dotted decimal notation for the last 32 bits of the IPv6 address regardless of its network prefix. It
   * is used for host addresses only.
   * Examples:
   *
   * <code>
   * 3:f12:1:1:ac4f:600:0.160.63.64
   * ::0.0.0.1
   * ::%ffff:192.168.3.14
   * </code>
   */
  dotted_quad
};

class network;
class host;

/**
 * An interface representing the IP address.
 *
 * This is an interface representing all Internet Protocol (IP) addresses. I covers both @ref ip::version
 * "versions", IPv4 and IPv6, and can be used to represent both @ref ip::host "host" and @ref ip::network
 * "network" addresses. Its assignment and type conversion operators, along with the constructors of the
 * implementation clases, provide simple means for converting the IP addresses from and to to the most
 * common alternative formats, such as <tt>struct in_addr</tt>, <tt>struct in_addr</tt>, binary arrays and
 * the textual presentation.
 */
class address
{
 public:
  /**
   * Virtual dtor to support deletion via the interface pointer.
   */
  virtual ~address()
  { /* noop */ }
  /**
   * IP address comparison.
   *
   * Two IP address objects compare equal if they are of the same @ref kind() "kind", are of the same
   * @ref version() "IP protocol version", and contain the same IP address value. However, the
   * @ref host address object of @ref version::ipv6 "IPv6" protocol version may compare the equal to
   * the @ref host address object of @ref version::ipv4 "IPv4" protocol version if the former was created
   * by @ref ipv6::rfc_ipv4map "mapping" or @ref ipv6::rfc_ipv4translate "translating" the latter into the
   * IPv6 address space. When comparing the two @ref ip::network "network" address object they have
   * to have equal address mask lengths, too.
   *
   * @param other an IP address object to compare to this object
   * @return true if two address objects compare equal, false if not
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
   * @see @ref cool::ng::ip::operator ==(const cool::ng::ip::address&, const cool::ng::ip::address&)
   *   "== operator" overload
   * @see @ref cool::ng::ip::operator !=(const cool::ng::ip::address&, const cool::ng::ip::address&)
   *   "!= operator" overload
   */
  virtual bool equals(const address& other) const = 0;
  /**
   * Convert the IPv6 address into textual format.
   *
   * Translates the IPv6 address into textual format for visualization.
   *
   * @param os reference to output  stream to write the visual presentation to
   * @param style_ visual style to use for visualization
   * @return
   * @see @ref cool::ng::ip::operator <<(std::ostream& os, const cool::ng::ip::address& val)
   *   "<< operator" overload
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
  */
  virtual std::ostream& visualize(std::ostream& os, style style_ = style::customary) const = 0;
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
   * @return kind::host This address object represents host address.
   * @return kind::network This address object represents network address.
   */
  virtual ip::kind kind() const = 0;
  /**
   * Type conversion operator.
   *
   * Converts this IP address into a sequence of byte values that correnspond to this IP address, in the
   * network byte order. The length of the sequence depends on the IP protocol  @ref version() "version" and
   * can be determined via the @ref size() method.
   *
   * @return Pointer to a sequence of bytes, in the network order, containing the byte representation of
   * this IP address. The sequence is @ref size() bytes long.
   */
  virtual EXPLICIT_ operator const uint8_t * () const = 0;
  /**
   * Type conversion operator.
   *
   * Converts this IP address into the IPv4 address structure. The IPv6 host and network addresses in
   * general cannot be converted into the IPv4 address structure due to the lack of the address space;
   * however, IPv6 host addresses that were generated by either @ref ipv6::rfc_ipv4map "mapping" or
   * @ref ipv6::rfc_ipv4translate "translating" the IPv4 host address into the IPv6 host address can
   * be converted by using the least significant four bytes of the IPv6 address only.
   *
   * @return <tt>struct in_addr</tt> filled with IPv4 address corresponding to the address contained in
   *   this address object.
   *
   * @exception exception::bad_conversion if this address object cannot be converted into the IPv4 address
   *  structure.
   *
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
   */
  virtual EXPLICIT_ operator struct in_addr() const = 0;
  /**
   * Type conversion operator.
   *
   * Converts this IP address into the IPv4 address structure. The IPv4 host addresses can be converted
   * into the IPv6 address structure by creating a @ref ipv6::rfc_ipv4map "mapped" IPv6 address first
   * and the using this to fill the IPv6 address structure. Such mapping woud be meaningless for IPv4
   * network addresses which thus cannot be converted into the IPv6 address structure.
   *
   * @return <tt>struct in6_addr</tt> filled with IPv6 address represented by
   *   this address object.
   * @exception exception::bad_conversion if this address object cannot be converted into the  IPv6
   * address structure
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   */
  virtual EXPLICIT_ operator struct in6_addr() const = 0;
  /**
   * Type conversion operator.
   *
   * Converts the IP address into the textual presentation. The IPv4 addresses are converted into the text
   * using common @ref style::dot_decimal "dot-decimal" visual  style. The IPv6 addresses are converted
   * into the text using the @ref style::canonical "canonical" visual style. If this IP address object is a network
   * address, the network mask length is appended using the CIDR notation.
   *
   * @return string containing the textual presentation of this IP address
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
   * Replaces the current values of an IP address with the value of another IP adress. The assignment is
   * permitted if both this object and the <tt>other</tt> parameter match in @ref cool::ng::ip::kind "kind" and
   * @ref cool::ng::ip::version "version", with the following exceptions:
   *   - it is possible to assign @ref ipv4::host "IPv4 host" address to @ref ipv6::host "IPv6 host" address object; the
   *    resulting IPv6 address will receive the @ref ipv6::rfc_ipv4map "standard SIIT prefix" <tt>::%ffff:0:0/96</tt>.
   *   - it is possible to assign an @ref ipv6::host "IPv6 host" to the @ref ipv4::host "IPv4  host" address object
   *    if the IPv6 host address is either @ref ipv6::rfc_ipv4map "mapped" or @ref ipv6::rfc_ipv4translate
   *    "translated" IPv4 host address.
   *   - it is possible to assign @ref ip::host "host" address to a @ref ip::network "network" address object
   *    of the same @ref ip::version "version". The network object will apply its @ref ip::network::mask()
   *    "network mask" to the host address and use it as the network address.
   *   - as an exception to the above rule, it is possible to assign IPv6 host address object to the IPv4
   *    network address object if the IPv6 host address is either @ref ipv6::rfc_ipv4map "mapped" or
   *    @ref ipv6::rfc_ipv4translate "translated" IPv4 host address.
   *
   *  In all other cases the operator will throw.
   *
   * @param rhs the IP address object which value to assign to this object.
   * @exception exception::bad_conversion thrown if an address cannot be assigned
   */
  virtual address& operator =(const address& rhs)
  {
    assign(rhs); return *this;
  }
  /**
   * Assignment operator.
   *
   * Assigns the binary representation of IP address to this address object. If this address object
   * is a @ref network object, a network mask is applied to the source bytes before assignment.
   *
   * @warning It is the user responsibility to ensure that the array contains
   *   at least <tt>@ref address::size() "this.size()"</tt> elements and that they represent
   *   a valid IP address of the respective IP protocol @ref ip::version "version".
   */
  virtual address& operator =(uint8_t const rhs[])
  {
    assign(rhs); return *this;
  }
  /**
   * Assignment operator.
   *
   * Assigns the IPv4 address contained in <tt>struct in_addr</tt> to this
   * address object, as follows:
   *  - if this address object is an @ref ipv4::host "IPv4 host object" the
   *   binary value is used directly, without further ado
   *  - if this address object is an @ref ipv4::network "IPv4 network object", the current
   *   network mask is applied to the supplied address and the result is used as an IPv4
   *   network address.
   *  - if this address objec is an @ref ipv6::host "IPv6 host object", the supplied address
   *   is prefixed with the  @ref ipv6::rfc_ipv4map "standard SIIT network prefix"
   *   <tt>::%ffff:0:0/96</tt> and used as an IPv6 host address.
   *
   *  In all other cases the operator will throw.
   *
   * @exception exception::bad_conversion if the source address is
   *   incompatible with the desitnation address
   *
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   */
  virtual address& operator =(const in_addr& rhs)
  {
    assign(rhs); return *this;
  }
  /**
   * Assignment operator.
   *
   * Assigns the IPv6 address contained in <tt>struct in6_addr</tt> to this
   * address object, as follows:
   *  - if this address object is an @ref ipv4::host "IPv4 host object", the supplied
   *   the supplied IPv6 address must be a host address and must belong to one
   *   of the IPv4 mapping networks, The the IPv4 host address is set by using the
   *   trailing 4 bytes of the IPv6 address.
   *  - if this address object is an @ref ipv6::host "IPv6 host object", the supplied address
   *   is used directly, without further checks.
   *  - if this address object is an @ref ipv6::network "IPv6 network object", the current
   *   network mask is applied to the supplied address and the result is used as an IPv6
   *   network address.
   *
   *  In all other cases the operator will throw.
   *
   * @exception exception::bad_conversion if the source address is
   *   incompatible with the desitnation address
   *
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
   */
  virtual address& operator =(const in6_addr& rhs)
  {
    assign(rhs); return *this;
  }
  /**
   * Assignment operator
   *
   * Sets the IP address from the textual presentation.
   *
   * @exception exception::illegal_state The textual presentation is not parsable
   * @exception exception::illegal_argument The textual presentation contains
   *   invalid characters.
   */
  virtual address& operator =(const std::string& rhs)
  {
    assign(rhs); return *this;
  }
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
  /**
   * Assign another IP address to this IP address.
   *
   * Implementation classes must provide this method to support the @ref operator=(const address&)
   * "assignment operator".
   *
   * @param rhs the IP address object which value to assign to this object.
   * @exception exception::bad_conversion thrown if an address cannot be assigned
   * @see @ref operator=(const address&) for the implementation requirements.
   */
  virtual void assign(const address& rhs) = 0;
  /**
   * Assign <tt>in6_addr</tt> to this IP address.
   *
   * Implementation classes must provide this method to support the @ref operator=(const in6_addr&)
   * "assignment operator".
   *
   * @param rhs <tt>in6_addr</tt> structure to be assigned to  this object.
   * @exception exception::bad_conversion thrown if an address cannot be assigned
   * @see @ref operator=(const in6_addr&) for the implementation requirements.
   */
  virtual void assign(const in6_addr& rhs) = 0;
  /**
   * Assign <tt>in_addr</tt> to this IP address.
   *
   * Implementation classes must provide this method to support the @ref operator=(const in_addr&)
   * "assignment operator".
   *
   * @param rhs <tt>in_addr</tt> structure to be assigned to  this object.
   * @exception exception::bad_conversion thrown if an address cannot be assigned
   * @see @ref operator=(const in_addr&) for the implementation requirements.
   */
  virtual void assign(const in_addr& rhs) = 0;
  /**
   * Assign array of bytes to this IP address.
   *
   * Implementation classes must provide this method to support the @ref operator=(uint8_t const [])
   * "assignment operator".
   *
   * @param rhs array of bytes to be assigned to  this object.
   * @exception exception::bad_conversion thrown if an address cannot be assigned
   * @see @ref operator=(uint8_t const []) for the implementation requirements.
   */
  virtual void assign(uint8_t const rhs[]) = 0;
  /**
   * Assign text presentation to this IP address.
   *
   * Implementation classes must provide this method to support the @ref operator=(const std::string&)
   * "assignment operator".
   *
   * @param rhs textual presentation of an IP addres to assign to this object.
   * @exception exception::bad_conversion thrown if an address cannot be assigned
   * @see @ref operator=(const std::string&) for the implementation requirements.
   */
  virtual void assign(const std::string& rhs) = 0;
};

/**
 * Interface class representing host/interface address.
 */
class host : public address
{
 public:
  EXPLICIT_ operator const uint8_t * () const override = 0;
  EXPLICIT_ operator struct in_addr() const override = 0;
  EXPLICIT_ operator struct in6_addr() const override = 0;
  EXPLICIT_ operator std::string() const override = 0;
  host& operator =(const address& rhs) override
  {
    address::operator =(rhs); return *this;
  }
  host& operator =(uint8_t const rhs[]) override
  {
    address::operator =(rhs); return *this;
  }
  host& operator =(const in_addr& rhs) override
  {
    address::operator =(rhs); return *this;
  }
  host& operator =(const in6_addr& rhs) override
  {
    address::operator =(rhs); return *this;
  }
  host& operator =(const std::string& rhs) override
  {
    address::operator =(rhs); return *this;
  }

  virtual ip::kind kind() const override
  {
    return ip::kind::host;
  }
};

/**
 * Interface class representing network address.
 */
class network : public address
{
 public:
  EXPLICIT_ operator const uint8_t * () const override = 0;
  EXPLICIT_ operator struct in_addr() const override = 0;
  EXPLICIT_ operator struct in6_addr() const override = 0;
  EXPLICIT_ operator std::string() const override = 0;
  network& operator =(const address& rhs) override
  {
    address::operator =(rhs); return *this;
  }
  network& operator =(uint8_t const rhs[]) override
  {
    address::operator =(rhs); return *this;
  }
  network& operator =(const in_addr& rhs) override
  {
    address::operator =(rhs); return *this;
  }
  network& operator =(const in6_addr& rhs) override
  {
    address::operator =(rhs); return *this;
  }
  network& operator =(const std::string& rhs) override
  {
    address::operator =(rhs); return *this;
  }
  /**
   * Determine whether IP address belongs to the network.
   *
   * @return true the specified IP address is either a host on this network
   *    or a sub-network of this network.
   *
   * @note IPv6 network does not contain IPv4 hosts or sub-networks, and vice versa.
   */
  virtual bool has(const address& address) const = 0;
  virtual ip::kind kind() const override
  {
    return ip::kind::network;
  }
  /**
   * Return the size of the network mask, in bits.
   */
  virtual std::size_t mask() const = 0;
};

/**
 * This namespace contains implementation related classes for IPv6 host nad network addresses.
 */
namespace ipv6 {

/**
 * A type used for binary representation of IPv6 addresses.
 */
using binary_t = cool::ng::util::binary<16>;

/**
 * Implementation class for IPv6 host addresses.
 */
class host : public ip::host
{
 public:
  // ==== ctors
  /**
   * Construct a host address object with the @ref unspecified "unspecified host address".
   */
  explicit host()
  { /* noop */ }
  /**
   * Construct a host address object from another address object.
   *
   * The other address object can be another IPv6 host address or an @ref ipv4::host "IPv4 host"
   * address. In the latter case the new host address object will receive a stanard @ref rfc_ipv4map
   * "SIIT prefix" for mapped addresses (<tt>:</tt><tt>:ffff:0:0/96</tt>).
   *
   * @param other the other IP address
   * @exception exception::bad_conversion thrown if other address object
   *   is not IPv6 host or IPv4 host address object.
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   */
  explicit host(const ip::address& other)
  {
    assign(other);
  }
  /**
   * Construct a host address object with the address read from the byte array.
   *
   * @param data      byte array containing network address
   * @warning The ctor uses the first 16 bytes of the array. Providing a byte array of less than 16
   *  elements results in an undefined behavior. This ctor is to be used only if external checks are applied
   *  to the parameter prior the construction of the address object.
   */
  explicit host(uint8_t const data[])
  {
    assign(data);
  }
  /**
   * Construct a host address object from the IPv6 address structure.
   *
   * @param data      IPv6 structure with network address.
   */
  explicit host(const in6_addr& data)
  {
    assign(data);
  }
  /**
   * Construct a host address object from IPv4 address structure.
   *
   * @param data      IPv4 structure with network address.
   * @note The constructed object is an IPv4 mapped address with the
   *   @ref rfc_ipv4map "standard network prefix" for mapped addresses.
   */
  explicit host(const struct in_addr& data)
  {
    assign(data);
  }
  /**
   * Construct a host address object from textual presentation of the IPv6 host address.
   *
   * The string parameter must contain a valid IPv6 host address in one of the @ref ip::style
   * "visual styles for IPV6 addresses" supported by the @ref ip. Additionally, the @ref ip::style::expanded
   * "expanded" style may be further expanded by providing leading zeros in the quad.
   *
   * @param data textual presentation of the IPv6 host address
   * @exception exception::bad_conversion thrown if the textual presentation of the address is not parsable
   */
  explicit host(const std::string& data)
  {
    assign(data);
  }
  /**
   * Construct a host address object with address from the initializer list.
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
   *
   * @param data     initializer list containing network address bytes.
   */
  host(std::initializer_list<uint8_t> data) : m_data(data)
  { /* noop */ }

  // ==== address interface
  // ---- type conversion operators
  EXPLICIT_ operator const uint8_t * () const override
  {
    return m_data;
  }
  dlldecl EXPLICIT_ operator struct in_addr() const override;
  dlldecl EXPLICIT_ operator struct in6_addr() const override;
  dlldecl EXPLICIT_ operator std::string () const override;

  // ---- assignment operators
  host& operator =(const ip::address& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  host& operator =(uint8_t const rhs[]) override
  {
    ip::address::operator =(rhs); return *this;
  }
  host& operator =(const in_addr& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  host& operator =(const in6_addr& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  host& operator =(const std::string& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  //---- other methods
  dlldecl bool equals(const ip::address& other) const override;
  dlldecl std::ostream& visualize(std::ostream& os, style style_ = style::customary) const override;
  ip::kind kind() const override
  {
    return ip::kind::host;
  }
  ip::version version() const override
  {
    return ip::version::ipv6;

  }
  std::size_t size() const override
  {
    return m_data.size();
  };
  dlldecl bool in(const ip::network& net) const override;
  dlldecl bool is(ip::attribute) const override;

 private:
  dlldecl void assign(const ip::address& rhs) override;
  dlldecl void assign(const in_addr& rhs) override;
  dlldecl void assign(const in6_addr& rhs) override;
  dlldecl void assign(uint8_t const rhs[]) override;
  dlldecl void assign(const std::string& rhs) override;

 private:
  binary_t m_data;
};

/**
 * Implementation class for IPv6 network addresses.
 */
class network : public ip::network
{
 public:
  /**
   * Construct a network address object with the @ref unspecified_network "unspecified network address".
   */
  explicit network() : m_length(0)
  { /* noop */ }
  /**
   * Construct a network address object with the specified mask size and an unspecified address.
   * @param mask_size number of bits, from the left, of the network address part (network mask)
   * @exception exception::out_of_range thrown if mask size exceeds 128 bits.
   */
  explicit network(std::size_t mask_size) : m_length(mask_size)
  {
    if (mask() > 128)
      throw exception::out_of_range();
  }
  /**
   * Construct a network address object with the address read from the byte array.
   *
   * @param mask_size Network mask size, in bits
   * @param data      Byte array containing network address.
   * @exception exception::out_of_range thrown if the mask size exceeds 128 bits.
   *
   * @warning The ctor uses the first 16 bytes of the array. Providing a byte array of less than 16
   *  elements results in an undefined behavior. This ctor is to be used only if external checks are applied
   *  to the parameter prior the construction of the address object.
   */
  explicit network(std::size_t mask_size, uint8_t const data[]) : m_length(mask_size)
  {
    assign(data);
  }
  /**
   * Construct a network address object from the IPv6 address structure.
   *
   * @param mask_size Network mask size, in bits
   * @param data      IPv6 structure with network address.
   * @exception exception::out_of_range thrown if mask size exceeds 128 bits.
   */
  explicit network(std::size_t mask_size, const in6_addr& data) : m_length(mask_size)
  {
    assign(data);
  }
  /**
   * Construct a network address object from textual presentation of the IPv6 network address.
   *
   * The string parameter must contain a valid IPv6 network address in one of the @ref ip::style
   * "visual styles for IPV6 addresses" supported by the @ref ip. Additionally, the @ref ip::style::expanded
   * "expanded" style may be further expanded by providing leading zeros in the quad.
   *
   * @param data textual presentation of the IPv6 network address
   * @exception exception::bad_conversion thrown if the textual presentation of IPv6 network address is not parsable
   * @exception exception::out_of_range thrown if the network mask length exceeds 128 bits
   * @note The network address string must include the network mask size.
   */
  explicit network(const std::string& data)
  {
    assign(data);
  }
  /**
   * Construct a network address object with address from the initializer list.
   *
   * This constructor makes the following initialization possible:
   * @code
   * network n = { 96, { 0x00, 0x00, 0x00, 0x00,
   *                     0x00, 0x00, 0x00, 0x00,
   *                     0x00, 0x00, 0x00, 0x00,
   *                     0xff, 0xff } };
   * @endcode
   * If the list contains fewer than 16 values the remaining values are
   * set to 0. The first number (96 in above example) is the network mask size,
   * in bits.
   *
   * @param mask_size network mask size, in bits
   * @param data      initializer list containing network address bytes.
   * @exception exception::out_of_range thrown if the mask size exceeds 128 bits.
   */
  dlldecl network(std::size_t mask_size, std::initializer_list<uint8_t> data);
  /**
   * Construct a network address object from the host address.
   *
   * Constructs a network address object by applying the specified network mask to
   * the host address object. This constructor can be used to determine the network
   * adddress of the known host, if the mask is known.
   *
   * @param mask_size number of bits, from the left, of the network address part (network mask)
   * @param other   the host  address object which IP address to  use
   * @exception exception::out_of_range thrown if mask_size exceeds 128 bits.
   */
  explicit network(std::size_t mask_size, const host& other) : m_length(mask_size)
  {
    assign(other);
  }

  // address interface
  dlldecl bool equals(const ip::address& other) const override;
  dlldecl std::ostream& visualize(std::ostream &os, style style_ = style::customary) const override;
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
  dlldecl EXPLICIT_ operator struct in_addr() const override;
  dlldecl EXPLICIT_ operator struct in6_addr() const override;
  dlldecl EXPLICIT_ operator std::string () const override;
  dlldecl std::size_t size() const override
  {
    return m_data.size();
  };
  network& operator =(const ip::address& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  network& operator =(const in_addr& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  network& operator =(const in6_addr& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  network& operator =(uint8_t const rhs[]) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  network& operator =(const std::string& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }

  bool in(const ip::network& net) const override;
  bool is(ip::attribute) const override;

  // network interface
  dlldecl bool has(const ip::address& other) const override;
  std::size_t mask() const override
  {
    return m_length;
  }

 private:
  dlldecl void assign(const ip::address& rhs) override;
  dlldecl void assign(const in_addr& rhs) override;
  dlldecl void assign(const in6_addr& rhs) override;
  dlldecl void assign(uint8_t const rhs[]) override;
  dlldecl void assign(const std::string& rhs) override;

 private:
  binary_t m_data;
  std::size_t m_length;
};

/**
 * Constant representing IPv6 loopback address.
 */
const host loopback = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
/**
 * Constant representing IPv6 unspecified host address.
 */
const host unspecified = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
/**
 * Constant representing IPv6 unspecified network address.
 */
const network unspecified_network = { 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
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
const network rfc_ipv4map = { 96, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff } };
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
 * This block is is reserved for multicast addresses.
 */
const network rfc_mcast = { 8, { 0xff } };

} // namespace ipv6

/**
 * This namespace contains implementation related classes for IPv4 host nad network addresses.
 */
namespace ipv4 {
/**
 * A type used for binary representation of IPv4 addresses.
 */
using binary_t = cool::ng::util::binary<4>;

/**
 * Implementation class for IPv4 host addresses.
 */
class host : public ip::host
{
 public:
  /**
   * Construct a host address object with the @ref unspecified "unspecified host address".
   */
  explicit host()
  { /* noop */ }
  /**
   * Construct a host address object from another address object.
   *
   * The other address object must be either another IPv4 host address object, or
   * an @ref ipv6::host "IPv6 host" address object that was @ref ipv6::rfc_ipv4map
   * "mapped" or @ref ipv6::rfc_ipv4translate "translated" from the IPv4 host address
   *
   * @param other the other IP address
   * @exception exception::bad_conversion thrown if other address object
   *   is neither IPv4 host address nor mapped or translated IPv6 host address.
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
   */
  explicit host(const ip::address& other)
  {
    assign(other);
  }
  /**
   * Construct a host address object with the address read from the byte array.
   *
   * @param data      byte array containing network address.
   *
   * @warning The ctor uses the first 4 bytes of the array. Providing a byte array of less than 4
   *  elements results in an undefined behavior. This ctor is to be used only if external checks are applied
   *  to the parameter prior the construction of the address object.
   */
  explicit host(uint8_t const data[])
  {
    assign(data);
  }
  /**
   * Construct a host address object from IPv6 address structure.
   *
   * Such construction is only possible if the IPv6 address structure contains
   * an IPv6 address that was mapped or translated from the IPv4 address..
   *
   * @param data      IPv6 structure with network address.
   * @exception exception::bad_conversion thrown if specified address
   *   is not valid.
   *
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
   */
  explicit host(const in6_addr& data)
  {
    assign(data);
  }
  /**
   * Construct a host address object from the IPv4 address structure.
   *
   * @param data      IPv4 structure with network address.
   */
  explicit host(const struct in_addr& data)
  {
    assign(data);
  }
  /**
   * Construct a host address object from textual presentation of the IPv4 host address.
   *
   * The string parameter must contain a valid IPv4 host address in @ref ip::style::dot_decimal
   * "dot-decimal" visual style.
   *
   * @param data textual presentation of the IPv4 host address
   *
   * @exception exception::bad_conversion thrown if the textual presentation is not parsable
   */
  explicit host(const std::string& data)
  {
    assign(data);
  }
  /**
   * Construct a host address object with address from the initializer list.
   *
   * This constructor makes the following initialization possible:
   * @code
   * host h = { 192, 168, 3, 20 };
   * @endcode
   * If the list contains fewer than 4 values the remaining values are
   * set to 0.
   *
   * @param data     initializer list containing network address bytes.
   */
  host(std::initializer_list<uint8_t> data) : m_data(data)
  { /* noop */ }

  // address interface
  dlldecl  bool equals(const ip::address& other) const override;
  dlldecl  std::ostream& visualize(std::ostream& os, style style_ = style::customary) const override;
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
  dlldecl EXPLICIT_ operator struct in_addr() const override;
  dlldecl EXPLICIT_ operator struct in6_addr() const override;
  dlldecl EXPLICIT_ operator std::string() const override;
  std::size_t size() const override
  {
    return m_data.size();
  };
  host& operator =(const ip::address& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  host& operator =(uint8_t const rhs[]) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  host& operator =(const in_addr& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  host& operator =(const in6_addr& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  host& operator =(const std::string& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }

  dlldecl bool in(const ip::network& net) const override;
  dlldecl bool is(ip::attribute) const override;

 private:
  dlldecl void assign(const ip::address& rhs) override;
  dlldecl void assign(const in_addr& rhs) override;
  dlldecl void assign(const in6_addr& rhs) override;
  dlldecl void assign(uint8_t const rhs[]) override;
  dlldecl void assign(const std::string& rhs) override;

 private:
  binary_t m_data;
};

/**
 * Implementation class for IPv4 network addresses.
 */
class network : public ip::network
{
 public:
  /**
   * Construct a network address object with the @ref unspecified_network "unspecified network address".
   */
  explicit network() : m_length(0)
  { /* noop */ }
  /**
   * Construct a network address object with the specified mask size and an unspecified address.
   * @param mask_size number of bits, from the left, of the network address part (network mask)
   * @exception exception::out_of_range thrown if mask size exceeds 32 bits.
   */
  explicit network(std::size_t mask_size) : m_length(mask_size)
  {
    if (mask() > 32)
      throw exception::out_of_range();
  }
  /**
   * Construct a network address object from the host or network address.
   *
   * Constructs a network address object by applying the specified network mask to
   * the host address object, or a new network mask to the nework address object.  The parameter may
   * be an IPv4 network address, or either an IPv4 host address or an IPv6 host address which was
   * @ref ipv6::rfc_ipv4map "mapped" or @ref ipv6::rfc_ipv4translate "translated" from IPv4 host address.
   *
   * @param mask_size number of bits, from the left, of the network address part (network mask)
   * @param other   the host  address object which IP address to  use
   * @exception exception::out_of_range thrown if mask_size exceeds 32 bits.
   * @exception exception::bad_conversion thown if the host address object is an @ref ipv6::host
   * "IPv6 host" address object which was not @ref ipv6::rfc_ipv4map "mapped" or @ref ipv6::rfc_ipv4translate
   * "translated" from IPv4 host address, or an IPv6 network address object.
   * @see @ref ipv6::rfc_ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see @ref ipv6::rfc_ipv4translate "IPv6 network prefix" for IPv4 translated
   *   addresses
   */
  explicit network(std::size_t mask_size, const ip::address& other) : m_length(mask_size)
  {
    assign(other);
  }
  /**
   * Construct a network address object with the address read from the byte array.
   *
   * @param mask_size network mask size, in bits
   * @param data      byte array containing network address.
   * @exception exception::bad_conversion if the specified address is not a host address
   * @exception exception::out_of_range thrown if mask size exceeds 32 bits.
   *
   * @warning The ctor uses the first 4 bytes of the array. Providing a byte array of less than 4
   *  elements results in an undefined behavior. This ctor is to be used only if external checks are applied
   *  to the parameter prior the construction of the address object.
   */
  explicit network(std::size_t mask_size, uint8_t const data[]) : m_length(mask_size)
  {
    assign(data);
  }
  /**
   * Construct a network  address object from the  IPv4 address structure.
   *
   * @param mask_size network mask size, in bits
   * @param data      IPv4 structure with network address.
   * @exception exception::out_of_range thrown if mask size exceeds 32 bits.
   */
  explicit network(std::size_t mask_size, const in_addr& data) : m_length(mask_size)
  {
    assign(data);
  }
  /**
   * Construct a network address object from textual presentation of the IPv4 network address.
   *
   * The string parameter must contain a valid IPv4 network address in @ref ip::style::dot_decimal
   * "dot-decimal" visual style.
   *
   * @param data textual presentation of the IPv4 network address
   * @exception exception::bad_conversion thrown if the textual presentation of IPv4 network address is not parsable
   * @exception exception::out_of_range thrown if the network mask length exceeds 32 bits
   * @note The network address string must include the network mask size.
   */
  explicit network(const std::string& data)
  {
    assign(data);
  }
  /**
   * Construct a network address object with address from the initializer list.
   *
   * This constructor makes the following construction possible:
   * @code
   * network n = { 24, { 192, 168, 3, 0 } };
   * @endcode
   * If the list contains fewer than 4 values the remaining values are
   * set to 0. The first number (24 in above example) is the network mask size,
   * in bits.
   *
   * @param mask_size network mask size, in bits
   * @param data     initializer list containing network address bytes.
   * @exception exception::out_of_range thrown if mask size exceeds 32 bits.
   */
  dlldecl network(std::size_t mask_size, std::initializer_list<uint8_t> data);

  // address interface
  dlldecl bool equals(const ip::address& other) const override;
  dlldecl std::ostream& visualize(std::ostream &os, style style_ = style::customary) const override;
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
  dlldecl EXPLICIT_ operator struct in_addr() const override;
  EXPLICIT_ operator struct in6_addr() const override
  {
    throw cool::ng::exception::bad_conversion();
  }
  dlldecl EXPLICIT_ operator std::string () const override;
  std::size_t size() const override
  {
    return m_data.size();
  };
  network& operator =(const ip::address& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  network& operator =(uint8_t const rhs[]) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  network& operator =(const in_addr& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  network& operator =(const in6_addr& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }
  network& operator =(const std::string& rhs) override
  {
    ip::address::operator =(rhs);
    return *this;
  }

  dlldecl bool in(const ip::network& net) const override;
  dlldecl bool is(ip::attribute) const override;

  // network interface
  dlldecl bool has(const ip::address& other) const override;
  std::size_t mask() const override
  {
    return m_length;
  }

 private:
  dlldecl void assign(const ip::address& rhs) override;
  dlldecl void assign(const in_addr& rhs) override;
  dlldecl void assign(const in6_addr& rhs) override;
  dlldecl void assign(uint8_t const rhs[]) override;
  dlldecl void assign(const std::string& rhs) override;

 private:
  binary_t m_data;
  std::size_t m_length;
};

/**
 * Constant representing IPv4 loopback address.
 */
const host loopback = { 127, 0, 0, 1 };
/**
 * Constant representing unspecified IPv4 host address.
 */
const host unspecified = { 0, 0, 0, 0 };
/**
 * Constant representing unspecified IPv4 network address.
 */
const network unspecified_network = { 0, { 0, 0, 0, 0 } };
/**
 * Constant representing IPv4 INADDR_ANY address.
 */
const host any;
/**
 * IPv4  broadcast IP address.
 */
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

/**
 * Host address container that can hold either IPv4 or IPv6 host address.
 *
 * This container is a replacement for cumbersone <tt>struct sockaddr_storage</tt>
 * address container.
 */
class host_container
{
 public:
 /**
  * Destructor
  */
  ~host_container();
  /**
   * Default constructor.
   *
   * Constructs a new instance of the host_container such that the following is true:
   *
   * <code>
   *     host_container() == ipv6::unspecified
   * </code>
   */
  host_container() : m_v6(ipv6::unspecified)
  { /* noop */ }
  /**
   * Copy constructor.
   */
  host_container(const host_container& other) : host_container(static_cast<const address&>(other))
  { /* noop */ }
  /**
   * Construct a host_container instance from the IP address.
   *
   * The address must be either IPv6 or IPv4 @ref host address.
   *
   * @throw cool::ng::exception::bad_conversion thrown if the IP address is not @ref host address.
   */
  host_container(const address& addr_) : host_container()
  {
    *this = addr_;
  }
  /**
   * Constuct a host_container instance from the IPv4 address structure.
   *
   * The address is interpreted to be an @ref ipv4::host "IPv4 host" address.
   */
  host_container(const in_addr& addr_) : host_container(ipv4::host(addr_))
  { /* noop */  }
  /**
   * Constuct a host_container instance from the IPv6 address structure.
   *
   * The address is interpreted to be an @ref ipv6::host "IPv6 host" address.
   */
  host_container(const in6_addr& addr_) : host_container(ipv6::host(addr_))
  { /* noop */  }
  /**
   * Constuct a host_container instance from the <tt>sockaddr_storage</tt> structure.
   */
  host_container(const sockaddr_storage& addr_) : host_container()
  {
    *this = addr_;
  }
  /**
   * Assignment operator.
   *
   * Replaces current value of this host_container object with the value of another host_container object.
   * This is a custom implementation of the copy-assignment operator.
   *
   * @param other the host_container object to copy the value from
   * @return reference to this host_container object
   */
  dlldecl host_container& operator =(const host_container& other);
  /**
   * Assignment operator.
   *
   * Sets the value of this host_container to the specified address, which must be either IPv6 or
   * IPv4 @ref host address.
   *
   * @param data an @ref ip::address to store in this host_container
   * @return reference to this host_container object
   * @exception cool::ng::exception::bad_conversion thrown if the provided address is not a @ref host
   *  address.
   */
  dlldecl host_container& operator =(const address& data);
  /**
   * Assignment operator.
   *
   * Sets the value of this host_container to the host address provided in the IPv4 <tt>struct in_addr</tt>
   * address structure. The address is interpreted to be an @ref ipv4::host "IPv4 host" address.
   *
   * @param data an IPv4 address structure containing the host address
   * @return reference to this host_container object
   */
  dlldecl host_container& operator =(const in_addr& data);
  /**
   * Assignment operator.
   *
   * Sets the value of this host_container to the host address provided in the IPv6 <tt>struct in6_addr</tt>
   * address structure. The address is interpreted to be an @ref ipv6::host "IPv6 host" address.
   *
   * @param data an IPv6 address structure containing the host address
   * @return reference to this host_container object
   */
  dlldecl host_container& operator =(const in6_addr& data);
  /**
   * Assignment operator.
   *
   * Sets the value of this host_container to the host address provided in the  <tt>sockaddr_storage</tt>
   * structure. Depending on the parameter's value, the address is interpreted to be either an @ref ipv6::host
   * "IPv6 host" address or an @ref ipv4::host "IPv4 host" address.
   *
   * @param data the <tt>sockaddr_storage</tt> address structure containing the host address
   * @return reference to this host_container object
   */
  dlldecl host_container& operator =(const sockaddr_storage& data);
  /**
   * Type conversion operator
   *
   * Converts the host_container into the <tt>const</tt> reference to the @ref address it holds. Note that
   * this type conversion operator is not <tt>explicit</tt> to enable a seamless use of this objects in
   * most contexts which expect @ref address.
   *
   * @return const reference to the @ref address stored in this object.
   */
  operator const address&() const          { return *static_cast<const address*>(static_cast<const void*>(&m_v4)); }
  /**
   * Type conversion operator
   *
   * Converts the host_container into the <tt>const</tt> pointer to the @ref address it holds.
   *
   * @return const pointer to the @ref address stored in this object.
   */
  EXPLICIT_ operator const address*() const { return static_cast<const address*>(static_cast<const void*>(&m_v4)); }
  /**
   * Type conversion operator.
   *
   * Converts this object into the <tt>struct sockaddr_storage</tt> used by traditional C networking API
   * to store either IPv4 or IPv6 IP address. The resulting structure will have all elements set accordingly
   * the IP address @ref version and value currently held by this object.
   *
   * @return <tt>sockaddr_storage</tt> structure reflecting the currently stored IP address.
   */
  dlldecl EXPLICIT_ operator sockaddr_storage() const;

 private:
  void release();
  void assign(const ipv4::host& addr_);
  void assign(const ipv6::host& addr_);

 private:
  union {
    ipv4::host m_v4;
    ipv6::host m_v6;
  };
};

/**
 * @ingroup ip
 * Binary compare two IP addresses.
 *
 * @return true if two addresses are binary equal
 * @return false if two addresses are binary different
 * @see @ref address::equals() for the definition of equality.
 */
inline bool operator ==(const address& lhs, const address& rhs)
{
  return lhs.equals(rhs);
}
/**
 * @ingroup ip
 * Binary compare two IP addresses.
 *
 * @return false if two addresses are binary equal
 * @return true if two addresses are binary different
 * @see @ref address::equals() for the definition of equality.
 */
inline bool operator !=(const address& lhs, const address& rhs)
{
  return !lhs.equals(rhs);
}
/**
 * @ingroup ip
 * Write an IP address to the output character stream.
 *
 * Generates a textual presentation of the IP address to the output stream.
 *
 * @see @ref cool::ng::ip::address::visualize() "address::visualize()" for more details.
 */
inline std::ostream& operator <<(std::ostream& os, const address& val)
{
  return val.visualize(os);
}
/**
 * @ingroup ip
 * Read an IP address from the input character stream.
 *
 * Parses the textual presentation of the IP address from the input stream. The
 * recognized presentation styles depend on the @ref cool::ng::ip::version "version"
 * and the @ref cool::ng::ip::kind "kind" of the provided
 * @ref cool::ng::ip::address "address" parameter as follows:
 *  - the @ref cool::ng::ip::version::ipv4 "IPv4" addresses are recognized in the
 *    usual @ref style::dot_decimal "dot-decimal" format, e.g. <tt>198.18.3.65</tt>
 *  - the @ref cool::ng::ip::version::ipv6 "IPv6" addresses are recognized in all styles
 *    listed in @ref cool::ng::ip::style enumeration as suitable for IPv6 addresses.
 *
 * For @ref cool::ng::ip::network "network" addresses of any
 * @ref cool::ng::ip::version "version" the network mask width can be specified
 * by appending a <tt>/</tt> character followed by an integer number to the
 * address, e.g. <tt>::%ffff:0:0/96</tt>.
 *
 * The reading of the input stream will stop at the first character that cannot be
 * interpreted as a part of the address. This character will be the next
 * character available in the input stream.
 *
 * @exception cool::exception::bad_conversion thrown if the address cannot
 *   be parsed.
 * @exception cool::exception::out_of_range thrown if the network mask size
 * exceeds <tt>@ref address::size() "size()" * 8</tt> bits.
 */
inline std::istream& operator >>(std::istream& is, address& val)
{
  return detail::sin(is, val);
}
/**
 * @ingroup ip
 * @ref ipv4::host "IPv4 host address" literal.
 *
 * Enables coding the IPv4 host addresses as a literal constants, eg:
 *
 * <code>
 *   auto host = "192.168.1.1"_ipv4;
 * </code>
 * @exception cool::ng::exception::bad_conversion thrown if the literal is not a valid IPv4 host address.
*/
inline ipv4::host operator "" _ipv4(const char* lit_, std::size_t len)
{
  return detail::literal_ipv4(lit_);
}
/**
 * @ingroup ip
 * @ref ipv6::host "IPv6 host address" literal.
 *
 * Enables coding the IPv6 host addresses as a  literal constants, eg:
 *
 * <code>
 *   auto host = "0:12f5:3:a3f8::3:1"_ipv6;
 * </code>
 *
 * The address string can be written in any @ref style "visual style" suitable for IPv6 host addresses.
 * @exception cool::ng::exception::bad_conversion thrown if the literal is not a valid IPv6 host address.
*/
inline ipv6::host operator "" _ipv6(const char* lit_, std::size_t len)
{
  return detail::literal_ipv6(lit_);
}
/**
 * @ingroup ip
 * @ref ipv4::network "IPv4 network address" literal.
 *
 * Enables coding the IPv4 network addresses as a  literal constants, eg:
 *
 * <code>
 *   auto net = "192.168.3.0/24"_ipv4_net;
 * </code>
 *
 * @exception cool::ng::exception::bad_conversion thrown if the literal is not a valid IPv4 network address.
*/

inline ipv4::network operator "" _ipv4_net(const char* lit_, std::size_t len)
{
  return detail::literal_ipv4_net(lit_);
}
/**
 * @ingroup ip
 * @ref ipv6::network "IPv6 network address" literal.
 *
 * Enables coding the IPv6 network addresses as a  literal constants, eg:
 *
 * <code>
 *   auto net = "::ffff:0:0/96"_ipv6_net;
 * </code>
 *
 * The address string can be written in any @ref style "visual style" suitable for IPv6 network addresses.
 * @exception cool::ng::exception::bad_conversion thrown if the literal is not a valid IPv6 network address.
*/
inline ipv6::network operator "" _ipv6_net(const char* lit_, std::size_t len)
{
  return detail::literal_ipv6_net(lit_);
}

/**
 * @ingroup ip
 * @ref address "IP address" literal.
 *
 * Enables coding of any IP addresses as a  literal constants, eg:
 *
 * <code>
 *   auto something = "0:12f5:3:a3f8::3:1"_ip;
 * </code>
 *
 * The implementation will examine the address literal to determine what the address represents and
 * return the shared pointer to the appropriate object. The address string can be written in any recognized
 * @ref style "visual style".
 *
 * @exception cool::ng::exception::bad_conversion thrown if the literal is not a valid IP address.
 * @exception cool::ng::exception::out_of_range thrown if the network mask exceeds the number of
 * address bits for given @ref ip::version "IP address version".
*/
std::shared_ptr<address> operator "" _ip(const char * lit_, std::size_t len)
{
  return detail::literal_ip(lit_);
}

} } } // namespaces cool::ng::ip

using cool::ng::ip::operator "" _ip;
using cool::ng::ip::operator "" _ipv4;
using cool::ng::ip::operator "" _ipv6;

#endif
