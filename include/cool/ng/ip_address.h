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
 * Enumeration listing attributes of an IP address.
 * @see address::is() method
 */
enum class attribute
{
  loopback         = 0x0001, //!< IP address is a loopback address of a device
  unspecified      = 0x0002, //!< IP address is not specified (includes IPv4 INADDR_ANY)
  /**
   * IP address is an IPv6 address mapped from the IPv4 address space
   *
   * @see @ref ipv6::assigned::ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see IPv6 %network prefix for @ref ipv6::assigned::ipv4xlat "translated" and
   *  @ref ipv6::assigned::ipv4xlat_local "locally translated" IPv4 addresses
   */
  ipv4originated   = 0x0004,
  assigned         = 0x0008,  //!< IP address is in the address range reserved number by one or more RFCs
  /**
   * Globally reachable addresses may be reached from any point in the Internat. The IP addresses are
   * globally reachable by default. A scope og certain address ranges may be limited by one or more
   * protocol standards (FCSs) and an appropriate address range reservation is registered with IANA. Also
   * note that while some reserved address ranges may be declared not to be globally reachable, the
   * applicaiton level protocols may, in turn, specify their subranges to be globally reachable again.
   */
   global          = 0x0010,
   /**
    * Forwardable addresses may be forwarded by one or more routing devices. The IP addresses are
    * forwadable by default. A scope og certain address ranges may be limited to a host or to a single
    * link by one or more protocol standards (FCSs) and an appropriate address range reservation is
    * registered with IANA.
    */
   forwardable     = 0x0020,
   source          = 0x0040, //!< The IP address is valid for use as a source address of the IP frame.
   destination     = 0x0080, //!< The IP address is valid for use as a destiantion address of the IP frame.
   multicast       = 0x0100, //!< The IP address is a multicast address
};

/**
 * Transport layer protocol enumeration
 */
enum class transport
{
  unknown, //!< the protocol is unknown
  udp,     //!< user datagram protocol (UDP)
  tcp      //!< transmission control protocol (TCP)
};
/**
 * Enumeration lising the valid visual styles of IPv4 and IPv6 addresses.
 *
 * @see RFC3513: <i>Internet Protocol Version 6 (IPv6) Addressing Architecture</i>
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
   * sequence of zero quads is compressed to '<tt>::</tt>'. @ref attribute::ipv4originated "IPv4 originated"
   * host addresses are shown in @ref style::dotted_quad "dotted-quad" style. This style can be used for
   * both host and network  addresses with the latter having a network mask size suffix (CIDR notation)
   * separated by the forward slash ('<tt>/</tt>). Examples:
   *
   * <code>
   * 3:f12:1:1:ac4f:600:a0:3f40
   * ::1
   * ::%ffff:192.168.3.14
   * 12::%ac4f:600:0:0/96
   * </code>
   * @see @ref ipv6::assigned::ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see IPv6 %network prefix for @ref ipv6::assigned::ipv4xlat "translated" and
   *  @ref ipv6::assigned::ipv4xlat_local "locally translated" IPv4 addresses
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

/**
 * Convert style enumeration to text.
 *
 * Helper function to convert style enumeration to text.
 * @param style_ style enumerator
 * @return enumerator text
 */
dlldecl std::string to_string(style style_) NOEXCEPT_;

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
class dlldecl address
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
   * by @ref attribute::ipv4originated "mapping or translating" the latter into the
   * IPv6 address space. When comparing the two @ref ip::network "network" address object they have
   * to have equal address mask lengths, too.
   *
   * @param other_ an IP address object to compare to this object
   * @return true if two address objects compare equal, false if not
   * @see @ref ipv6::assigned::ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see IPv6 %network prefix for @ref ipv6::assigned::ipv4xlat "translated" and
   *  @ref ipv6::assigned::ipv4xlat_local "locally translated" IPv4 addresses
   * @see @ref cool::ng::ip::operator ==(const cool::ng::ip::address&, const cool::ng::ip::address&)
   *   "== operator" overload
   * @see @ref cool::ng::ip::operator !=(const cool::ng::ip::address&, const cool::ng::ip::address&)
   *   "!= operator" overload
   */
  virtual bool equals(const address& other_) const = 0;
  /**
   * Convert the IPv6 address into textual format.
   *
   * Translates the IPv6 address into textual format for visualization.
   *
   * @param os_ reference to output  stream to write the visual presentation to
   * @param style_ visual style to use for visualization
   * @return
   * @see @ref cool::ng::ip::operator <<(std::ostream& os, const cool::ng::ip::address& val)
   *   "<< operator" overload
   */
  virtual std::ostream& visualize(std::ostream& os_, style style_ = style::customary) const = 0;
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
  EXPLICIT_ virtual operator const uint8_t * () const = 0;
  /**
   * Type conversion operator.
   *
   * Converts this IP address into the IPv4 address structure. The IPv6 host and network addresses in
   * general cannot be converted into the IPv4 address structure due to the lack of the address space;
   * however, IPv6 host addresses that have @ref attribute::ipv4originated "IPv4 origins" can
   * be converted to an IPv4 address structure by using the least significant four bytes of the IPv6 address only.
   *
   * @return <tt>struct in_addr</tt> filled with IPv4 address corresponding to the address contained in
   *   this address object.
   *
   * @exception exception::bad_conversion if this address object cannot be converted into the IPv4 address
   *  structure.
   *
   * @see @ref ipv6::assigned::ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see IPv6 %network prefix for @ref ipv6::assigned::ipv4xlat "translated" and
   *  @ref ipv6::assigned::ipv4xlat_local "locally translated" IPv4 addresses
   *   addresses
   */
  EXPLICIT_ virtual operator struct in_addr() const = 0;
  /**
   * Type conversion operator.
   *
   * Converts this IP address into the IPv4 address structure. The IPv4 host addresses can be converted
   * into the IPv6 address structure by creating a @ref attribute::ipv4originated "mapped" IPv6 address first
   * and then using this address to fill the IPv6 address structure. Such mapping would be meaningless for
   * the IPv4 network addresses which thus cannot be converted into the IPv6 address structure.
   *
   * @return <tt>struct in6_addr</tt> filled with IPv6 address represented by
   *   this address object.
   * @exception exception::bad_conversion if this address object cannot be converted into the  IPv6
   * address structure
   * @see @ref ipv6::assigned::ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   */
  EXPLICIT_ virtual operator struct in6_addr() const = 0;
  /**
   * Type conversion operator.
   *
   * Converts the IP address into the textual presentation using the @ref style::customary
   * "common visual style". If this IP address object is a network address, the network mask length is
   * appended using the CIDR notation.
   *
   * @return string containing the textual presentation of this IP address
   */
  EXPLICIT_ virtual operator std::string() const = 0;
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
   *    resulting IPv6 address will receive the @ref ipv6::assigned::ipv4map "standard SIIT prefix" <tt>::%ffff:0:0/96</tt>.
   *   - it is possible to assign an @ref ipv6::host "IPv6 host" to the @ref ipv4::host "IPv4  host" address object
   *    if the IPv6 host address @ref attribute::ipv4originated "originated in the IPv4" address space
   *   - it is possible to assign @ref ip::host "host" address to a @ref ip::network "network" address object
   *    of the same @ref ip::version "version". The network object will apply its @ref ip::network::mask()
   *    "network mask" to the host address and use it as the network address.
   *   - as an exception to the above rule, it is possible to assign IPv6 host address object to the IPv4
   *    network address object if the IPv6 host address @ref attribute::ipv4originated "originated in the IPv4"
   *    address space
   *
   *  In all other cases the operator will throw.
   *
   * @param rhs_ the IP address object which value to assign to this object.
   * @return <tt>*this</tt>
   * @exception exception::bad_conversion thrown if an address cannot be assigned
   * @see @ref ipv6::assigned::ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see IPv6 %network prefix for @ref ipv6::assigned::ipv4xlat "translated" and
   *  @ref ipv6::assigned::ipv4xlat_local "locally translated" IPv4 addresses
   */
  virtual address& operator =(const address& rhs_)
  {
    assign(rhs_); return *this;
  }
  /**
   * Assignment operator.
   *
   * Assigns the binary representation of IP address to this address object. If this address object
   * is a @ref network object, a network mask is applied to the source bytes before assignment.
   *
   * @param rhs_ byte array containing and IP address
   * @return <tt>*this</tt>
   * @warning It is the user responsibility to ensure that the array contains
   *   at least <tt>@ref address::size() "this.size()"</tt> elements and that they represent
   *   a valid IP address of the respective IP protocol @ref ip::version "version".
   */
  virtual address& operator =(uint8_t const rhs_[])
  {
    assign(rhs_); return *this;
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
   *  - if this address object is an @ref ipv6::host "IPv6 host object", the supplied address
   *   is prefixed with the  @ref ipv6::assigned::ipv4map "standard SIIT network prefix"
   *   <tt>::%ffff:0:0/96</tt> and used as an IPv6 host address.
   *
   *  In all other cases the operator will throw.
   * @param rhs_ IPv4 address structure containing the address
   * @return <tt>*this</tt>
   * @exception exception::bad_conversion if the source address is
   *   incompatible with the desitnation address
   * @see @ref ipv6::assigned::ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   */
  virtual address& operator =(const struct in_addr& rhs_)
  {
    assign(rhs_); return *this;
  }
  /**
   * Assignment operator.
   *
   * Assigns the IPv6 address contained in <tt>struct in6_addr</tt> to this
   * address object, as follows:
   *  - if this address object is an @ref ipv4::host "IPv4 host object", the supplied
   *   the supplied IPv6 address must be a host address @ref attributte::ipv4originated "originated in the IPv4"
   *   address space. The the IPv4 host address is set by using the trailing 4 bytes of the IPv6 address.
   *  - if this address object is an @ref ipv6::host "IPv6 host object", the supplied address
   *   is used directly, without further checks.
   *  - if this address object is an @ref ipv6::network "IPv6 network object", the current
   *   network mask is applied to the supplied address and the result is used as an IPv6
   *   network address.
   *
   *  In all other cases the operator will throw.
   *
   * @param rhs_ IPv6 address structure containing an IP address
   * @return <tt>*this</tt>
   * @exception exception::bad_conversion if the source address is
   *   incompatible with the destination address type
   * @see @ref ipv6::assigned::ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see IPv6 %network prefix for @ref ipv6::assigned::ipv4xlat "translated" and
   *  @ref ipv6::assigned::ipv4xlat_local "locally translated" IPv4 addresses
   */
  virtual address& operator =(const struct in6_addr& rhs_)
  {
    assign(rhs_); return *this;
  }
  /**
   * Assignment operator
   *
   * Sets the IP address from the textual presentation.
   *
   * @param rhs_ string contanining the textual presentation of an IP address
   * @return <tt>*this</tt>
   * @exception exception::illegal_state The textual presentation is not parsable
   * @exception exception::illegal_argument The textual presentation contains
   *   invalid characters.
   */
  virtual address& operator =(const std::string& rhs_)
  {
    assign(rhs_); return *this;
  }
  /**
   * Determine whether address belongs to the specified network or not.
   *
   * Determines whether this IP address belongs to the specified network,
   * either a a host address on the network or as its sub-network.
   *
   * @param net_ network address to examine the membership of
   * @return true if this address belongs to the specified network, false
   * otherwise.
   *
   * @note IPv6 addresses never belong to IPv4 networks and vice versa.
   */
  virtual bool in(const network& net_) const = 0;
  /**
   * Determine whether IP address demonstrates specified attribute.
   *
   * @param attr_ Attribute to check.
   * @return true if this IP addresses demonstrates the attribute, false if not
   */
  virtual bool is(attribute attr_) const = 0;

 protected:
  /**
   * Assign another IP address to this IP address.
   *
   * Implementation classes must provide this method to support the @ref operator=(const address&)
   * "assignment operator".
   *
   * @param rhs_ the IP address object which value to assign to this object.
   * @exception exception::bad_conversion thrown if an address cannot be assigned
   * @see @ref operator=(const address&) for the implementation requirements.
   */
  virtual void assign(const address& rhs_) = 0;
  /**
   * Assign <tt>in6_addr</tt> to this IP address.
   *
   * Implementation classes must provide this method to support the @ref operator=(const struct in6_addr&)
   * "assignment operator".
   *
   * @param rhs_ <tt>in6_addr</tt> structure to be assigned to  this object.
   * @exception exception::bad_conversion thrown if an address cannot be assigned
   * @see @ref operator=(const struct in6_addr&) for the implementation requirements.
   */
  virtual void assign(const struct in6_addr& rhs_) = 0;
  /**
   * Assign <tt>in_addr</tt> to this IP address.
   *
   * Implementation classes must provide this method to support the @ref operator=(const struct in_addr&)
   * "assignment operator".
   *
   * @param rhs_ <tt>in_addr</tt> structure to be assigned to  this object.
   * @exception exception::bad_conversion thrown if an address cannot be assigned
   * @see @ref operator=(const struct in_addr&) for the implementation requirements.
   */
  virtual void assign(const struct in_addr& rhs_) = 0;
  /**
   * Assign array of bytes to this IP address.
   *
   * Implementation classes must provide this method to support the @ref operator=(uint8_t const [])
   * "assignment operator".
   *
   * @param rhs_ array of bytes to be assigned to  this object.
   * @exception exception::bad_conversion thrown if an address cannot be assigned
   * @see @ref operator=(uint8_t const []) for the implementation requirements.
   */
  virtual void assign(uint8_t const rhs_[]) = 0;
  /**
   * Assign text presentation to this IP address.
   *
   * Implementation classes must provide this method to support the @ref operator=(const std::string&)
   * "assignment operator".
   *
   * @param rhs_ textual presentation of an IP addres to assign to this object.
   * @exception exception::bad_conversion thrown if an address cannot be assigned
   * @see @ref operator=(const std::string&) for the implementation requirements.
   */
  virtual void assign(const std::string& rhs_) = 0;
};

/**
 * Interface class representing host/interface address.
 */
class dlldecl host : public address
{
 public:
  EXPLICIT_ operator const uint8_t * () const override = 0;
  EXPLICIT_ operator struct in_addr() const override = 0;
  EXPLICIT_ operator struct in6_addr() const override = 0;
  EXPLICIT_ operator std::string() const override = 0;
  host& operator =(const address& rhs_) override
  {
    address::operator =(rhs_); return *this;
  }
  host& operator =(uint8_t const rhs_[]) override
  {
    address::operator =(rhs_); return *this;
  }
  host& operator =(const struct in_addr& rhs_) override
  {
    address::operator =(rhs_); return *this;
  }
  host& operator =(const struct in6_addr& rhs_) override
  {
    address::operator =(rhs_); return *this;
  }
  host& operator =(const std::string& rhs_) override
  {
    address::operator =(rhs_); return *this;
  }

  virtual ip::kind kind() const override
  {
    return ip::kind::host;
  }
};

/**
 * Interface class representing network address.
 */
class dlldecl network : public address
{
 public:
  EXPLICIT_ operator const uint8_t * () const override = 0;
  EXPLICIT_ operator struct in_addr() const override = 0;
  EXPLICIT_ operator struct in6_addr() const override = 0;
  EXPLICIT_ operator std::string() const override = 0;
  network& operator =(const address& rhs_) override
  {
    address::operator =(rhs_); return *this;
  }
  network& operator =(uint8_t const rhs_[]) override
  {
    address::operator =(rhs_); return *this;
  }
  network& operator =(const struct in_addr& rhs_) override
  {
    address::operator =(rhs_); return *this;
  }
  network& operator =(const struct in6_addr& rhs_) override
  {
    address::operator =(rhs_); return *this;
  }
  network& operator =(const std::string& rhs_) override
  {
    address::operator =(rhs_); return *this;
  }
  /**
   * Determine whether IP address belongs to the network.
   *
   * @return true if the specified IP address is either a host on this network
   *    or a sub-network of this network, false if not/
   *
   * @note IPv6 network does not contain IPv4 hosts or sub-networks, and vice versa.
   */
  virtual bool has(const address& addr_) const = 0;
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
 * A structure representing data about address range reserved for special purposes.
 *
 * @see @ref ipv6::assigned "IPv6 numbers reserved for special purposes"
 * @see @ref ipv4::assigned "IPv4 numbers reserved for special purposes"
 */
class dlldecl assigned_number
{
 public:
  /**
   * Virtual dtor for interface class.
   */
  virtual ~assigned_number()
  { /* noop */ }
  /**
   * Return the address range reserved for special purposes.
   */
  virtual const network& address() const = 0;
  /**
   * Return the brief description of the reserved address range.
   */
  virtual const char* description() const = 0;
  /**
   * Return the name of the standard that reserved the address range.
   */
  virtual const char* reference() const = 0;
  /**
   * Return true if the addresses from the reserved range are valid as source addresses of the IP frame, false if not.
   * @see @ref attribute::source
   */
  virtual bool valid_as_source() const = 0;
  /**
   * Return true if the addresses from the reserved range are valid as destination addresses of the IP frame, false if not.
   * @see @ref attribute::destination
   */
  virtual bool valid_as_destination() const = 0;
  /**
   * Return true if the addresses from the reserved range can be forwarded by network routing nodes.
   * @see @ref attribute::forwardable
   */
  virtual bool is_forwardable() const = 0;
  /**
   * Return true if the addresses from the reserved range may be globally reachable.
   * @see @ref attribute::global.
   */
  virtual bool is_global() const = 0;
  /**
   * Return true if the addresses from the reserved range are multicast addresses.
   * @see @ref attribute::multicast.
   */
  virtual bool is_mcast() const = 0;
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
 * IANA special purpose IPv6 addresses.
 *
 * Various protocol and application level RFC specifications reserve certain address ranges for the
 * special purposes. The Internet Assigned Numbers Authority (IANA) maintains the registry of reserved ranges.
 * This enumeration lists the special purpose addresses as of December 2019. For the latest status
 * check the <a href="https://www.iana.org/assignments/iana-ipv6-special-registry/iana-ipv6-special-registry.xhtml">
 * IANA IPv6 Special-Purpose Address Registry</a>.
 */
class dlldecl assigned
{
 public:
  enum {
    unspecified = 0,  //!< IPv6 address range ::/128 reserved by RFC4291
    loopback,         //!< IPv6 address range ::1/128 reserved by RFC4291
    ipv4map,          //!< IPv6 address range ::%ffff:0:0/96 reserved by RFC4291
    ipv4xlat,         //!< IPv6 address range 64:ff9b::/96 reserved by RFC6052
    ipv4xlat_local,   //!< IPv6 address range 64:ff9b:1::/48 reserved by RFC8215
    discard,          //!< IPv6 address range 100::/64 reserved by RFC8215
    ietf,             //!< IPv6 address range 2001::/23 reserved by RFC2928
    ietf_teredo,      //!< IPv6 address range 2001::/32 reserved by RFC4380
    ietf_pcp_anycast, //!< IPv6 address range 2001:1::1/128 reserved by RFC7723
    ietf_nat_anycast, //!< IPv6 address range 2001:1::2/128 reserved by RFC8155
    ietf_benchmark,   //!< IPv6 address range 2001:2::/48 reserved by RFC5180
    ietf_amt,         //!< IPv6 address range 2001:3::/32 reserved by RFC7450
    ietf_as112,       //!< IPv6 address range 2001:4:112::/48 reserved by RFC7535
    ietf_orchid_v2,   //!< IPv6 address range 2001:20::/28 reserved by RFC7343
    documentation,    //!< IPv6 address range 2001:db8::/32 reserved by RFC3849
    rfc_6to4,         //!< IPv6 address range 2002::/16 reserved by RFC3056
    as112_delegation, //!< IPv6 address range 2620:4f:8000::/48 reserved by RFC7534
    multicast,        //!< IPV6 address range ff00::/8 reserved by RFC 2373 for multicast
    local,            //!< IPv6 address range fc00::/7 reserved by RFC4193
    link_local,       //!< IPv6 address range fe80::/10 reserved by RFC4291
  };
  /**
   * Number of builtin special purpose addresses.
   */
  static const std::size_t count;
  /**
   * Return special purpose address identified by its enumerator.
   * @param what_ an integer value of the special purpose addresses' enumerator
   * @return the special purpose address range identified by its enumerator
   * @exception cool::ng::exception::out_of_range thrown if the enumerator's integer value exceeds the
   * number of registered special purpose addresses.
   */
  static const network& get(std::size_t what_);
  /**
   * Return special purpose address identified by its enumerator.
   * @param what_ an integer value of the special purpose addresses' enumerator
   * @return the @ref assigned_number structure  identified by its enumerator
   * @exception cool::ng::exception::out_of_range thrown if the enumerator's integer value exceeds the
   * number of registered special purpose addresses.
   */
  static const assigned_number& get_info(std::size_t what_);
  /**
   * Add a special purpose address range.
   *
   * Adds a special purpose address range to the table of registered address ranges reserved for special
   * purposes. The added address range will be taken into account by queries made via @ref get(), @ref get_info()
   * and @ref address::is() methods.
   *
   * @param addr_  the special purpose address range denoted as the network address combined with
   * the bit mask
   * @param desc_ a brief description of the reserved address range
   * @param ref_ a reference to the document that is a base for reserving the range
   * @param is_src_ set to @c true if the addresses from this range are valid as source addresses of
   * the IP frames
   * @param is_dst_ set to @c true if the addresses from this range are valid as destiantion addresses of
   * the IP frames
   * @param is_fwd_ set to @c true if the addresses from this range may be forwarded by the network
   * routing nodex
   * @param is_glob_ set to @c true if the addresses from this range may be globally reachable
   * @param is_mcast_ set to @c true if the addresses from this range ara multicast addresses
   *
   * @return an address range enumerator that can be used for queries via @ref get() or @ref get_info()
   * methods.
   * @warning The table of the special purpose numbers @em does @em not make a copy of the
   * @ref network object specified via @c addr_ parameter. It stores the address of the original object.
   * This implies that the original object must exist from the moment it was added to the table of special
   * purpose addresses until the end of the program execution. It must not be dynamically allocated.
   * Modifying the original of the added object may yield an undefined behavior. The same ilimitations apply
   * to the description and reference texts specified via @c desc_ and @c ref_ parameters, if their
   * values are not @c nullptr.
   * @note This function allows the users to extend the table with their custom address ranges.  It also
   * allows the user code to adjust the provided table with the recent reservations that may have been
   * made to the reference
   * <a href="https://www.iana.org/assignments/iana-ipv6-special-registry/iana-ipv6-special-registry.xhtml">
   * IANA IPv6 Special-Purpose Address Registry</a> and are not yet reflected in the provided table.
   */
  static std::size_t add(
      const network& addr_
    , const char* desc_
    , const char* ref_
    , bool is_src_
    , bool is_dst_
    , bool is_fwd_
    , bool is_glob_
    , bool is_mcast_ = false
  );
};


/**
 * Implementation class for IPv6 host addresses.
 */
class dlldecl host : public ip::host
{
 public:
 /**
  * Constant representing IPv6 loopback address.
  */
  static host const loopback;
 /**
  * Constant representing unspecified IPv6 host address.
  */
  static host const unspecified;
 /**
  * Constant representing <tt>IN6_ADDR_ANY</tt>.
  */
  static host const any;

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
   * address. In the latter case the new host address object will receive  stanard @ref assigned::ipv4map
   * "SIIT prefix" for mapped addresses (<tt>:</tt><tt>:ffff:0:0/96</tt>).
   *
   * @param other_ the other IP address
   * @exception exception::bad_conversion thrown if other address object
   *   is not IPv6 host or IPv4 host address object.
   * @see @ref ipv6::assigned::ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   */
  explicit host(const ip::address& other_)
  {
    assign(other_);
  }
  /**
   * Construct a host address object with the address read from the byte array.
   *
   * @param data_ byte array containing network address
   * @warning The ctor uses the first 16 bytes of the array. Providing a byte array of less than 16
   *  elements results in an undefined behavior. This ctor is to be used only if external checks are applied
   *  to the parameter prior the construction of the address object.
   */
  explicit host(uint8_t const data_[])
  {
    assign(data_);
  }
  /**
   * Construct a host address object from the IPv6 address structure.
   *
   * @param data_ IPv6 structure with network address.
   */
  explicit host(const struct in6_addr& data_)
  {
    assign(data_);
  }
  /**
   * Construct a host address object from IPv4 address structure.
   *
   * @param data_     IPv4 structure with network address.
   * @note The constructed object is an @ref attribute::ipv4originated "IPv4 mapped address" with the
   *   @ref ipv6::assigned::ipv4map "standard network prefix" for mapped addresses.
   * @see @ref ipv6::assigned::ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   */
  explicit host(const struct in_addr& data_)
  {
    assign(data_);
  }
  /**
   * Construct a host address object from textual presentation of the IPv6 host address.
   *
   * The string parameter must contain a valid IPv6 host address in one of the @ref ip::style
   * "visual styles for IPV6 addresses" supported by the @ref ip. Additionally, the @ref ip::style::expanded
   * "expanded" style may be further expanded by providing leading zeros in the quad.
   *
   * @param data_ textual presentation of the IPv6 host address
   * @exception exception::bad_conversion thrown if the textual presentation of the address is not parsable
   */
  explicit host(const std::string& data_)
  {
    assign(data_);
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
   * If the list contains fewer than 16 values the remaining bytes to the left are set to 0.
   *
   * @param data_     initializer list containing network address bytes.
   */
  host(std::initializer_list<uint8_t> data_) : m_data(data_)
  { /* noop */ }

  // ==== address interface
  // ---- type conversion operators
  EXPLICIT_ operator const uint8_t * () const override
  {
    return m_data.data();
  }
  EXPLICIT_ operator struct in_addr() const override;
  EXPLICIT_ operator struct in6_addr() const override;
  EXPLICIT_ operator std::string () const override;

  // ---- assignment operators
  host& operator =(const ip::address& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  host& operator =(uint8_t const rhs_[]) override
  {
    ip::address::operator =(rhs_); return *this;
  }
  host& operator =(const struct in_addr& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  host& operator =(const struct in6_addr& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  host& operator =(const std::string& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  //---- other methods
  bool equals(const ip::address& other_) const override;
  std::ostream& visualize(std::ostream& os_, style style_ = style::customary) const override;
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
  bool in(const ip::network& net_) const override;
  bool is(ip::attribute attr_) const override;

 private:
  void assign(const ip::address& rhs_) override;
  void assign(const struct in_addr& rhs_) override;
  void assign(const struct in6_addr& rhs_) override;
  void assign(uint8_t const rhs_[]) override;
  void assign(const std::string& rhs_) override;

 private:
  binary_t m_data;
};

/**
 * Implementation class for IPv6 network addresses.
 */
class dlldecl network : public ip::network
{
 public:
  /**
   * Construct a network address object with the @ref ipv6::assigned::unspecified "unspecified network address".
   */
  explicit network() : m_length(128)
  { /* noop */ }
  /**
   * Construct a network address object with the specified mask size and an unspecified address.
   * @param mask_size_ number of bits, from the left, of the network address part (network mask)
   * @exception exception::out_of_range thrown if mask size exceeds 128 bits.
   * @note All address bytes are set to 0.
   */
  explicit network(std::size_t mask_size_) : m_length(mask_size_)
  {
    if (mask() > 128)
      throw exception::out_of_range();
  }
  /**
   * Construct a network address object with the address read from the byte array.
   *
   * @param mask_size_ Network mask size, in bits
   * @param data_      Byte array containing network address.
   * @exception exception::out_of_range thrown if the mask size exceeds 128 bits.
   *
   * @warning The ctor uses the first 16 bytes of the array. Providing a byte array of less than 16
   *  elements results in an undefined behavior. This ctor is to be used only if external checks are applied
   *  to the parameter prior the construction of the address object.
   */
  explicit network(std::size_t mask_size_, uint8_t const data_[]) : m_length(mask_size_)
  {
    assign(data_);
  }
  /**
   * Construct a network address object from the IPv6 address structure.
   *
   * @param mask_size_ Network mask size, in bits
   * @param data_      IPv6 structure with network address.
   * @exception exception::out_of_range thrown if mask size exceeds 128 bits.
   */
  explicit network(std::size_t mask_size_, const struct in6_addr& data_) : m_length(mask_size_)
  {
    assign(data_);
  }
  /**
   * Construct a network address object from textual presentation of the IPv6 network address.
   *
   * The string parameter must contain a valid IPv6 network address in one of the @ref ip::style
   * "visual styles for IPV6 addresses" supported by the @ref ip. Additionally, the @ref ip::style::expanded
   * "expanded" style may be further expanded by providing leading zeros in the quad.
   *
   * @param data_ textual presentation of the IPv6 network address
   * @exception exception::bad_conversion thrown if the textual presentation of IPv6 network address is not parsable
   * @exception exception::out_of_range thrown if the network mask length exceeds 128 bits
   * @note The network address string must include the network mask size.
   */
  explicit network(const std::string& data_)
  {
    assign(data_);
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
   * @param mask_size_ network mask size, in bits
   * @param data_      initializer list containing network address bytes.
   * @exception exception::out_of_range thrown if the mask size exceeds 128 bits.
   */
  network(std::size_t mask_size_, std::initializer_list<uint8_t> data_);
  /**
   * Construct a network address object from the host address.
   *
   * Constructs a network address object by applying the specified network mask to
   * the host address object. This constructor can be used to determine the network
   * adddress of the known host, if the mask is known.
   *
   * @param mask_size_ number of bits, from the left, of the network address part (network mask)
   * @param other_   the host  address object which IP address to  use
   * @exception exception::out_of_range thrown if mask_size exceeds 128 bits.
   */
  explicit network(std::size_t mask_size_, const host& other_) : m_length(mask_size_)
  {
    assign(other_);
  }

  // address interface
  bool equals(const ip::address& other_) const override;
  std::ostream& visualize(std::ostream &os_, style style_ = style::customary) const override;
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
    return m_data.data();
  }
  EXPLICIT_ operator struct in_addr() const override;
  EXPLICIT_ operator struct in6_addr() const override;
  EXPLICIT_ operator std::string () const override;
  std::size_t size() const override
  {
    return m_data.size();
  };
  network& operator =(const ip::address& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  network& operator =(const struct in_addr& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  network& operator =(const struct in6_addr& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  network& operator =(uint8_t const rhs_[]) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  network& operator =(const std::string& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }

  bool in(const ip::network& net_) const override;
  bool is(ip::attribute) const override;

  // network interface
  bool has(const ip::address& other_) const override;
  std::size_t mask() const override
  {
    return m_length;
  }

 private:
  void assign(const ip::address& rhs_) override;
  void assign(const struct in_addr& rhs_) override;
  void assign(const struct in6_addr& rhs_) override;
  void assign(uint8_t const rhs_[]) override;
  void assign(const std::string& rhs_) override;

 private:
  binary_t m_data;
  std::size_t m_length;
};

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
 * IANA special purpose IPv4 addresses.
 *
 * Various protocol and application level RFC specifications reserve certain address ranges for the
 * special purposes. The Internet Assigned Numbers Authority (IANA) maintains the registry of reserved ranges.
 * This enumeration lists the special purpose addresses as of December 2019. For the latest status
 * check the <a href="https://www.iana.org/assignments/iana-ipv4-special-registry/iana-ipv4-special-registry.xhtml">
 * IANA IPv4 Special-Purpose Address Registry</a>.
 */
class dlldecl assigned
{
 public:
  enum {
    unspecified = 0,   //!< Unspecified IPv4 network address range <tt>0.0.0.0/8</tt> (RFC 1122)
    private_1,         //!< IPv4 address range <tt>10.0.0.0/24</tt> reserved by RFC 1918
    shared_space,      //!< IPv4 address range <tt>100.64.0.0/10</tt> reserved by RFC 6598
    loopback,          //!<  IPv4 address range<tt> 127.0.0.0/8</tt>  reserved by RFC 1122 for localhost.
    private_2,         //!< IPv4 address range <tt>172.16.0.0/12</tt> reserved by RFC 1918
    ietf,              //!< IPv4 address range <tt>192.0.0.0/24</tt> reserved by RFC 6980
    ietf_continuity,   //!< IPv4 address range <tt>192.0.0.0/32</tt> reserved by RFC 7335
    ietf_dummy,        //!< IPv4 address range <tt>192.0.0.8/32</tt> reserved by RFC 7600
    ietf_pcp_anycast,  //!< IPv4 address range <tt>192.0.0.9/32</tt> reserved by RFC 7723
    ietf_nat_anycast,  //!< IPv4 address range <tt>192.0.0.10/32</tt> reserved by RFC 7723
    ietf_discovery_1,  //!< IPv4 address range <tt>192.0.0.170/32</tt> reserved by RFC 7050
    ietf_discovery_2,  //!< IPv4 address range <tt>192.0.0.171/32</tt> reserved by RFC 7050
    test_net_1,        //!< IPv4 address range <tt>192.0.2.0/24</tt> reserved by RFC 5737
    as112,             //!< IPv4 address range <tt>192.31.196.0/24</tt> reserved by RFC 7535
    amt,               //!< IPv4 address range <tt>192.52.193.0/24</tt> reserved by RFC 7535
    private_3,         //!< IPv4 address range <tt>192.168.0.0/16</tt> reserved by by RFC 1918
    as112_delegation,  //!< IPv4 address range <tt>192.175.48.0/24</tt> reserved by RFC 7534
    benchmark,         //!< IPv4 address range <tt>198.18.0.0/15</tt> reserved by by RFC 2544
    test_net_2,        //!< IPv4 address range <tt>198.51.100.0/24</tt> reserved by RFC 5737
    test_net_3,        //!< IPv4 address range <tt>203.0.113.0/24</tt> reserved by RFC 5737
    multicast,         //!< IPv4 address range <tt>224.0.0.0/4</tt> reserved by RFC 1112 for multicast (former class D)
    future_use,        //!< IPv4 address range <tt>240.0.0.0/4</tt> reserved by RFC 5771
    broadcast,         //!< IPv4 address range <tt>255.255.255.255/32</tt> reserved by RFC 8190
    link_local         //!< IPv4 address range <tt>169.254.0.0/16</tt> reserved by RFC 3927 for link local addresses
  };

  /**
   * Number of builtin special purpose addresses.
   */
  static const std::size_t count;
  /**
   * Return special purpose address identified by its enumerator.
   * @param what_ an integer value of the special purpose addresses' enumerator
   * @exception cool::ng::exception::out_of_range thrown if the enumerator's integer value exceeds the
   * number of registered special purpose addresses.
   */
  static const network& get(std::size_t what_);
  /**
   * Return special purpose address identified by its enumerator.
   * @param what_ an integer value of the special purpose addresses' enumerator
   * @return the @ref assigned_number structure  identified by its enumerator
   * @exception cool::ng::exception::out_of_range thrown if the enumerator's integer value exceeds the
   * number of registered special purpose addresses.
   */
  static const assigned_number& get_info(std::size_t what_);
  /**
   * Add a special purpose address range.
   *
   * Adds a special purpose address range to the table of registered address ranges reserved for special
   * purposes. The added address range will be taken into account by queries made via @ref get(), @ref get_info()
   * and @ref address::is() methods.
   *
   * @param addr_  the special purpose address range denoted as the network address combined with
   * the bit mask
   * @param desc_ a brief description of the reserved address range
   * @param ref_ a reference to the document that is a base for reserving the range
   * @param is_src_ set to @c true if the addresses from this range are valid as source addresses of
   * the IP frames
   * @param is_dst_ set to @c true if the addresses from this range are valid as destiantion addresses of
   * the IP frames
   * @param is_fwd_ set to @c true if the addresses from this range may be forwarded by the network
   * routing nodex
   * @param is_glob_ set to @c true if the addresses from this range may be globally reachable
   * @param is_mcast_ set to @c true if the addresses from this range ara multicast addresses
   *
   * @return an address range enumerator that can be used for queries via @ref get() or @ref get_info()
   * methods.
   * @warning The table of the special purpose numbers @em does @em not make a copy of the
   * @ref network object specified via @c addr_ parameter. It stores the address of the original object.
   * This implies that the original object must exist from the moment it was added to the table of special
   * purpose addresses until the end of the program execution. It must not be dynamically allocated.
   * Modifying the original of the added object may yield an undefined behavior. The same ilimitations apply
   * to the description and reference texts specified via @c desc_ and @c ref_ parameters, if their
   * values are not @c nullptr.
   * @note This function allows the users to extend the table with their custom address ranges.  It also
   * allows the user code to adjust the provided table with the recent reservations that may have been
   * made to the reference
   * <a href="https://www.iana.org/assignments/iana-ipv4-special-registry/iana-ipv4-special-registry.xhtml">
   * IANA IPv4 Special-Purpose Address Registry</a> and are not yet reflected in the provided table.
   */
  static std::size_t add(
      const network& addr_
    , const char* desc_
    , const char* ref_
    , bool is_src_
    , bool is_dst_
    , bool is_fwd_
    , bool is_glob_
    , bool is_mcast_ = false
  );
};

/**
 * Implementation class for IPv4 host addresses.
 */
class dlldecl host : public ip::host
{
 public:
  /**
   * Constant representing IPv4 loopback address.
  */
  static const host loopback;
  /**
   * Constant representing unspecified IPv4 address.
  */
  static const host unspecified;
  /**
   * Constant representing IPv4 INADDR_ANY address.
  */
  static const host any;

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
   * an @ref ipv6::host "IPv6 host" address object that @ref attribute::ipv4originated
   * "that originated from the IPv4 address space".
   *
   * @param other_ the other IP address
   * @exception exception::bad_conversion thrown if other address object
   *   is neither IPv4 host address nor mapped or translated IPv6 host address.
   * @see @ref ipv6::assigned::ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see IPv6 %network prefix for @ref ipv6::assigned::ipv4xlat "translated" and
   *  @ref ipv6::assigned::ipv4xlat_local "locally translated" IPv4 addresses
   */
  explicit host(const ip::address& other_)
  {
    assign(other_);
  }
  /**
   * Construct a host address object with the address read from the byte array.
   *
   * @param data_      byte array containing network address.
   *
   * @warning The ctor uses the first 4 bytes of the array. Providing a byte array of less than 4
   *  elements results in an undefined behavior. This ctor is to be used only if external checks are applied
   *  to the parameter prior the construction of the address object.
   */
  explicit host(uint8_t const data_[])
  {
    assign(data_);
  }
  /**
   * Construct a host address object from IPv6 address structure.
   *
   * Such construction is only possible if the IPv6 address structure contains
   * an IPv6 address that @ref attribute::ipv4originated "that originated from the IPv4 address space".
   *
   * @param data_   IPv6 structure with network address.
   * @exception exception::bad_conversion thrown if specified address
   *   is not valid.
   *
   * @see @ref ipv6::assigned::ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see IPv6 %network prefix for @ref ipv6::assigned::ipv4xlat "translated" and
   *  @ref ipv6::assigned::ipv4xlat_local "locally translated" IPv4 addresses
   */
  explicit host(const struct in6_addr& data_)
  {
    assign(data_);
  }
  /**
   * Construct a host address object from the IPv4 address structure.
   *
   * @param data_      IPv4 structure with network address.
   */
  explicit host(const struct in_addr& data_)
  {
    assign(data_);
  }
  /**
   * Construct a host address object from textual presentation of the IPv4 host address.
   *
   * The string parameter must contain a valid IPv4 host address in @ref ip::style::dot_decimal
   * "dot-decimal" visual style.
   *
   * @param data_ textual presentation of the IPv4 host address
   *
   * @exception exception::bad_conversion thrown if the textual presentation is not parsable
   */
  explicit host(const std::string& data_)
  {
    assign(data_);
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
   * @param data_     initializer list containing network address bytes.
   */
  host(std::initializer_list<uint8_t> data_) : m_data(data_)
  { /* noop */ }

  // address interface
  bool equals(const ip::address& other_) const override;
  std::ostream& visualize(std::ostream& os_, style style_ = style::customary) const override;
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
    return m_data.data();
  }
  EXPLICIT_ operator struct in_addr() const override;
  EXPLICIT_ operator struct in6_addr() const override;
  EXPLICIT_ operator std::string() const override;
  std::size_t size() const override
  {
    return m_data.size();
  };
  host& operator =(const ip::address& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  host& operator =(uint8_t const rhs_[]) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  host& operator =(const struct in_addr& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  host& operator =(const struct in6_addr& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  host& operator =(const std::string& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }

  bool in(const ip::network& net) const override;
  bool is(ip::attribute) const override;

 private:
  void assign(const ip::address& rhs_) override;
  void assign(const struct in_addr& rhs_) override;
  void assign(const struct in6_addr& rhs_) override;
  void assign(uint8_t const rhs_[]) override;
  void assign(const std::string& rhs_) override;

 private:
  binary_t m_data;
};

/**
 * Implementation class for IPv4 network addresses.
 */
class dlldecl network : public ip::network
{
 public:
  /**
   * Construct a network address object with the @ref reserved::unspecified "unspecified network address".
   */
  explicit network() : m_length(8)
  { /* noop */ }
  /**
   * Construct a network address object with the specified mask size and an unspecified address.
   * @param mask_size_ number of bits, from the left, of the network address part (network mask)
   * @exception exception::out_of_range thrown if mask size exceeds 32 bits.
   */
  explicit network(std::size_t mask_size_) : m_length(mask_size_)
  {
    if (mask() > 32)
      throw exception::out_of_range();
  }
  /**
   * Construct a network address object from the host or network address.
   *
   * Constructs a network address object by applying the specified network mask to
   * the host address object, or a new network mask to the nework address object.  The parameter may
   * be an IPv4 network address, or either an IPv4 host address or an IPv6 host address
   * @ref attribute::ipv4originated "that originated from the IPv4 address space".
   *
   * @param mask_size_ number of bits, from the left, of the network address part (network mask)
   * @param other_   the host  address object which IP address to  use
   * @exception exception::out_of_range thrown if mask_size exceeds 32 bits.
   * @exception exception::bad_conversion thown if the host address object is an @ref ipv6::host
   * "IPv6 host" address object  @ref attribute::ipv4originated "not originated from the IPv4 address space".
   * @see @ref ipv6::assigned::ipv4map "IPv6 network prefix" for IPv4 mapped
   *   addresses.
   * @see IPv6 %network prefix for @ref ipv6::assigned::ipv4xlat "translated" and
   *  @ref ipv6::assigned::ipv4xlat_local "locally translated" IPv4 addresses
   */
  explicit network(std::size_t mask_size_, const ip::address& other_) : m_length(mask_size_)
  {
    assign(other_);
  }
  /**
   * Construct a network address object with the address read from the byte array.
   *
   * @param mask_size_ network mask size, in bits
   * @param data_      byte array containing network address.
   * @exception exception::bad_conversion if the specified address is not a host address
   * @exception exception::out_of_range thrown if mask size exceeds 32 bits.
   *
   * @warning The ctor uses the first 4 bytes of the array. Providing a byte array of less than 4
   *  elements results in an undefined behavior. This ctor is to be used only if external checks are applied
   *  to the parameter prior the construction of the address object.
   */
  explicit network(std::size_t mask_size_, uint8_t const data_[]) : m_length(mask_size_)
  {
    assign(data_);
  }
  /**
   * Construct a network  address object from the  IPv4 address structure.
   *
   * @param mask_size_ network mask size, in bits
   * @param data_      IPv4 structure with network address.
   * @exception exception::out_of_range thrown if mask size exceeds 32 bits.
   */
  explicit network(std::size_t mask_size_, const struct in_addr& data_) : m_length(mask_size_)
  {
    assign(data_);
  }
  /**
   * Construct a network address object from textual presentation of the IPv4 network address.
   *
   * The string parameter must contain a valid IPv4 network address in @ref ip::style::dot_decimal
   * "dot-decimal" visual style.
   *
   * @param data_ textual presentation of the IPv4 network address
   * @exception exception::bad_conversion thrown if the textual presentation of IPv4 network address is not parsable
   * @exception exception::out_of_range thrown if the network mask length exceeds 32 bits
   * @note The network address string must include the network mask size.
   */
  explicit network(const std::string& data_)
  {
    assign(data_);
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
   * @param mask_size_ network mask size, in bits
   * @param data_     initializer list containing network address bytes.
   * @exception exception::out_of_range thrown if mask size exceeds 32 bits.
   */
  network(std::size_t mask_size_, std::initializer_list<uint8_t> data_);

  // address interface
  bool equals(const ip::address& other_) const override;
  std::ostream& visualize(std::ostream &os_, style style_ = style::customary) const override;
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
    return m_data.data();
  }
  EXPLICIT_ operator struct in_addr() const override;
  EXPLICIT_ operator struct in6_addr() const override
  {
    throw cool::ng::exception::bad_conversion();
  }
  EXPLICIT_ operator std::string () const override;
  std::size_t size() const override
  {
    return m_data.size();
  };
  network& operator =(const ip::address& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  network& operator =(uint8_t const rhs_[]) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  network& operator =(const struct in_addr& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  network& operator =(const struct in6_addr& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }
  network& operator =(const std::string& rhs_) override
  {
    ip::address::operator =(rhs_);
    return *this;
  }

  bool in(const ip::network& net_) const override;
  bool is(ip::attribute attr_) const override;

  // network interface
  bool has(const ip::address& other_) const override;
  std::size_t mask() const override
  {
    return m_length;
  }

 private:
  void assign(const ip::address& rhs_) override;
  void assign(const struct in_addr& rhs_) override;
  void assign(const struct in6_addr& rhs_) override;
  void assign(uint8_t const rhs_[]) override;
  void assign(const std::string& rhs_) override;

 private:
  binary_t m_data;
  std::size_t m_length;
};

} // namespace ipv4

/**
 * Host address container that can hold either IPv4 or IPv6 host address.
 *
 * This container is a replacement for cumbersone <tt>struct sockaddr_storage</tt>
 * address container.
 */
class dlldecl host_container
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
  host_container() : m_v6(ipv6::host::unspecified)
  { /* noop */ }
  /**
   * Copy constructor.
   */
  host_container(const host_container& other_) : host_container(static_cast<const address&>(other_))
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
  host_container(const struct in_addr& addr_) : host_container(ipv4::host(addr_))
  { /* noop */  }
  /**
   * Constuct a host_container instance from the IPv6 address structure.
   *
   * The address is interpreted to be an @ref ipv6::host "IPv6 host" address.
   */
  host_container(const struct in6_addr& addr_) : host_container(ipv6::host(addr_))
  { /* noop */  }
  /**
   * Constuct a host_container instance from the <tt>sockaddr_storage</tt> structure.
   *
   * @param addr_ the <tt>sockaddr_storage</tt> structure
   */
  host_container(const struct sockaddr_storage& addr_) : host_container()
  {
    *this = addr_;
  }
  /**
   * Assignment operator.
   *
   * Replaces current value of this host_container object with the value of another host_container object.
   * This is a custom implementation of the copy-assignment operator.
   *
   * @param other_ the host_container object to copy the value from
   * @return reference to this host_container object
   */
  host_container& operator =(const host_container& other_);
  /**
   * Assignment operator.
   *
   * Sets the value of this host_container to the specified address, which must be either IPv6 or
   * IPv4 @ref host address.
   *
   * @param data_ an @ref ip::address to store in this host_container
   * @return reference to this host_container object
   * @exception cool::ng::exception::bad_conversion thrown if the provided address is not a @ref host
   *  address.
   */
  host_container& operator =(const address& data_);
  /**
   * Assignment operator.
   *
   * Sets the value of this host_container to the host address provided in the IPv4 <tt>struct in_addr</tt>
   * address structure. The address is interpreted to be an @ref ipv4::host "IPv4 host" address.
   *
   * @param data_ an IPv4 address structure containing the host address
   * @return reference to this host_container object
   */
  host_container& operator =(const struct in_addr& data_);
  /**
   * Assignment operator.
   *
   * Sets the value of this host_container to the host address provided in the IPv6 <tt>struct in6_addr</tt>
   * address structure. The address is interpreted to be an @ref ipv6::host "IPv6 host" address.
   *
   * @param data_ an IPv6 address structure containing the host address
   * @return reference to this host_container object
   */
  host_container& operator =(const struct in6_addr& data_);
  /**
   * Assignment operator.
   *
   * Sets the value of this host_container to the host address provided in the  <tt>sockaddr_storage</tt>
   * structure. Depending on the parameter's value, the address is interpreted to be either an @ref ipv6::host
   * "IPv6 host" address or an @ref ipv4::host "IPv4 host" address.
   *
   * @param data_ the <tt>sockaddr_storage</tt> address structure containing the host address
   * @return reference to this host_container object
   */
  host_container& operator =(const struct sockaddr_storage& data_);
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
  explicit operator const address*() const { return static_cast<const address*>(static_cast<const void*>(&m_v4)); }
  /**
   * Type conversion operator.
   *
   * Converts this object into the <tt>struct sockaddr_storage</tt> used by traditional C networking API
   * to store either IPv4 or IPv6 IP address. The resulting structure will have all elements set accordingly
   * the IP address @ref version and value currently held by this object.
   *
   * @return <tt>sockaddr_storage</tt> structure reflecting the currently stored IP address.
   */
  explicit operator struct sockaddr_storage() const;

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
inline bool operator ==(const address& lhs_, const address& rhs_)
{
  return lhs_.equals(rhs_);
}

/**
 * @ingroup ip
 * Binary compare two IP addresses.
 *
 * @return false if two addresses are binary equal
 * @return true if two addresses are binary different
 * @see @ref address::equals() for the definition of equality.
 */
inline bool operator !=(const address& lhs_, const address& rhs_)
{
  return !lhs_.equals(rhs_);
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
inline ipv4::host operator "" _ipv4(const char* lit_, std::size_t len_)
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
inline ipv6::host operator "" _ipv6(const char* lit_, std::size_t len_)
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
inline ipv4::network operator "" _ipv4_net(const char* lit_, std::size_t len_)
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
inline ipv6::network operator "" _ipv6_net(const char* lit_, std::size_t len_)
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
std::shared_ptr<address> operator "" _ip(const char * lit_, std::size_t len_)
{
  return detail::literal_ip(lit_);
}

/**
 * A class representing a network service.
 *
 * The network service is identified with the three data elements
 *    - the @ref transport level protocol this service is using,
 *    - the @ref address of the host running the service, and
 *    - a service port
 *
 * This class combines all three data elements and can be used to represent either the service provides side
 * or the service consumer side of the communication.
 */
class dlldecl service
{
 public:
  /**
   * Default constructor.
   *
   * Constructs and instance of the service object with invalid service data. Such service object will throw
   * on most operations.
   */
  explicit service() : m_proto(transport::unknown), m_port(0)
  { /* noop */ }
  /**
   * Construct a service object for the service using the specified transport protocol.
   *
   * Such service object, while having valid service data, cannot be used to implement or contact
   * actual network service since its network address is set to @ref ipv6::host::unspecified and it service
   * port to 0. It can be used, however, as a target for assigning <tt>sockaddr</tt> structure with the
   * actual service information.
   * @param t_ The @ref transport used by the service
   * @exception cool::ng::exception::illegal_argument thrown if the <tt>t_</tt> parameter is set to
   * <tt>transport::unknown</tt>.
   */
  explicit service(transport t_) : m_proto(t_), m_port(0)
  {
    if (t_ == transport::unknown)
      throw cool::ng::exception::illegal_argument();
    sync();
  }
  /**
   * Construct a service object for the service using the specified transport protocol and <tt>sockaddr</tt> structure.
   *
   * Constructs a service object for the specified @ref transport, and the IP address and the service port
   * number fetched from the provided <tt>sockaddr</tt> structure.
   * @param t_ the @ref transport used by this service
   * @param sa_ the pointer to <tt>sockaddr</tt> structure
   * @exception cool::ng::exception::illegal_argument thrown if the provided pointer is <tt>nullptr</tt>
   * or if the <tt>t_</tt> parameter is set to <tt>transport::unknown</tt>.
   * @exception cool::ng::exception::bad_conversion thrown is <tt>sockaddr</tt> is neither AF_INET
   * nor AF_INET6 socket address.
   */
  explicit service(transport t_, const struct sockaddr* sa_) : service(t_)
  {
    assign(sa_);
  }
  /**
   * Construct a service object for the specified service using the specified transport, address and the service port
   *
   * @param t_ the @ref transport used by this service
   * @param a_ the IP @ref address of the host sunning the service , or connecting to the local service
   * @param p_ the service port used by the service, or the consumer port using to the service
   * @exception cool::ng::exception::bad_conversion trown  if the @ref address is not a @ref host address
   * @exception cool::ng::exception::illegal_argument thrown if the <tt>t_</tt> parameter is set to
   * <tt>transport::unknown</tt>.
   */
  explicit service(transport t_, const address& a_, uint16_t p_)
    : service(t_)
  {
    m_host = a_;
    m_port = p_;
  }
  /**
   * Construct a service object for the URI string
   *
   * Construct the service object from the URI string.
   * @param uri_ the URI s tring containing the service information
   * @exception cool::ng::exception::bad_conversion trown  if the URI string is malformed
   * @see service::operator =(const std::string&)
   */
  explicit service(const std::string& uri_) : service()
  {
    assign(uri_);
  }
  /**
   * Assignment operator.
   *
   * Assigns the service information from the URI string. The URI string contains the transport level
   * protocol (either <i>tcp</i> or <i>udp</i>), followed by the @ref address "IP address" of the host
   * providing or consuming the service, followed by the service port, as in the following examples:
   *
   *<code>
   *  tcp://127.0.0.1:442
   *  udp://[::1]:80
   *  tcp://[::%ffff:192.168.3.42]:7776
   *</code>
   *
   * @param uri_ the URI string with the service information
   * @exception cool::ng::exception::bad_conversion thrown if the input string is malformed.
   * @return reference to this object
   * @note In order to disambiguate between the separator for the service port and a quad deliimiter used
   * in textual presentation of IPv6 addresses, the latter must be enclosed in square brackets ('<tt>[</tt>' and
   * '<tt>]</tt>'). The IPv4 address <i>must not</i> be enclosed in square brackers. The IPv4 and IPv6
   * addresses themselves must be written in one of the recognized @ref style "visual styles".
   * @see @ref  style "Visual styles" for textual presentation of IP addresses
   * @see RFC 3986: <i>Uniform Resource Identifier (URI): Generic Syntax</i>
   */
  service& operator =(const std::string& uri_)
  {
    assign(uri_); return *this;
  }
  /**
   * Assignment operator.
   *
   * Assign the IP address and the service port number, contained in the <tt>sockaddr</tt> structure,
   * to this service object. The assignment does not modify the @ref transport of the service, which must be
   * set prior to assignment. An attempt to use this assignment operator on a default constructed object
   * will throw.
   *
   * @param sa_ pointer to the <tt>sockaddr</tt> structure.
   * @exception cool::ng::exception::invalid_state thrown if this service object is not valid
   * @exception cool::ng::exception::illegal_argument thrown if the provided pointer is <tt>nullptr</tt>
   * @exception cool::ng::exception::bad_conversion thrown is <tt>sockaddr</tt> is neither AF_INET
   * nor AF_INET6 socket address.
   * @return reference to this object
   * @note This assignment operator will not modify the @ref transport protocol used by the service.
   * Consequenlty, it is illegal to use this assignment operator on invalid (@ref service::service()
   * "default constructed") service object.
   */
  service& operator =(const sockaddr* sa_)
  {
    assign(sa_); return *this;
  }
  /**
   * Type conversion operator.
   *
   * Determines the validity of this service object.
   *
   * @return true if the service information was set
   * @return false if this object was default constructed and the service information was not set
   */
  explicit operator bool () const  { return m_proto != transport::unknown; }
  /**
   * Type conversion operator.
   *
   * Returns a string representation of this service information in a form of URI specification.
   *
   * @return an URI string containing this seervice information
   * @exception cool::ng::exception::bad_conversion thrown if this object does not contain a valid
   * service information due to the default construction.
   * @see service::operator =(const std::string&)
   */
  explicit operator std::string() const;
  /**
   * Get the <tt>sockaddr</tt> pointer.
   *
   * Return a pointer to the <tt>sockaddr</tt> structure which corresponds to the service information
   * stored in this service object. This pointer can be directly passed to the functions that pass this
   * data to the kernel, eg:
   *
   * <code>
   *   auto res = connect(sock, service.sockaddr(), service.sockaddr_len());
   * </code>
   * @return <tt>const</tt> pointer to the internal sockaddr structure containing the service address
   * @exception cool::ng::exception::bad_conversion thrown if the service object is not valid
   */
  const struct sockaddr* sockaddr() const;
  /**
   * Return the size of the socket address structure.
   *
   * Returns the size of the <tt>sockaddr</tt> structure needed to represent the service @ref address.
   *
   * @return the size of the <tt>sockaddr</tt> structure.
   * @exception cool::ng::exception::bad_conversion thrown if this object does not contain a valid
   * service information due to the default construction.
   */
  socklen_t sockaddr_len() const;
  /**
   * Return the domain of the network socket required for the service.
   *
   * Returns a numerical value of the network domain, required for the communication for this service. The
   * returned value is such that can be directly used with the socket(2) call to allocate a new network
   * socket, as in the following code fragment:
   *
   * <code>
   *   auto sock = socket(service.socket_domain(), service.socket_type(), 0);
   * </code>
   * @return AF_INET if the if the address of the network service is @ref version::ipv4 "IPv4" address
   * @return AF_INET6 if the if the address of the network service is @ref version::ipv6 "IPv6" address
   */
  int socket_domain() const
  {
    return static_cast<const address&>(m_host).version() == version::ipv6 ? AF_INET6 : AF_INET;
  }
  /**
   * Return the type of the network socket required for the service.
   *
   * Returns a numerical value of the socket type, required for the communication for this service. The
   * returned value is such that can be directly used with the socket(2) call to allocate a new network
   * socket, as in the following code fragment:
   *
   * <code>
   *   auto sock = socket(service.socket_domain(), service.socket_type(), 0);
   * </code>
   * @return SOCK_DGRAM if the transport protocol for this service is transport::udp
   * @return SOCK_STREAM if the transport protocol for this service is transport::tcp
   * @exception cool::ng::exception::bad_conversion thrown if the transport protocol is not known due
   *  to the default construction
   */
  int socket_type() const
  {
    switch (m_proto)
    {
      case transport::udp:     return SOCK_DGRAM;
      case transport::tcp:     return SOCK_STREAM;
      case transport::unknown:
      default:                  throw cool::ng::exception::bad_conversion();
    }
  }
  /**
   * Return the @ref address "IP address" of the service endpoint.
   */
  const address& host() const
  {
    return static_cast<const address&>(m_host);
  }
  /**
   * Return the port number of the service endpoint.
   */
  uint16_t port() const
  {
    return m_port;
  }
  /**
   * Return the @ref transport "transport protocol" used by the service.
   */
  transport transport_protocol() const
  {
    return m_proto;
  }
  /**
   * Present the service information in a textual format as URI.
   *
   * Returns the URI string containing the service information.
   *
   * @param os_ reference to the output stream to write the text to
   * @param style_ the @ref style "visual style" to use for textual presentation of the IP address of the service
   * @exception cool::ng::bad_conversion thrown if this service object is not valid (was default constructed), or
   *  if the requested @ref style "visual style" cannot be applied to the @ref address "IP address" of the service
   */
  std::ostream& visualize(std::ostream& os_, style style_ = style::customary) const;

 private:
  void assign(const struct sockaddr* sa_);
  void assign(const std::string& s_);
  void sync();

 private:
  transport      m_proto;
  host_container m_host;
  uint16_t       m_port;
  union {
    mutable struct sockaddr_in   m_in;
    mutable struct sockaddr_in6  m_in6;
  };
};

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
 * Write a service URI to the output character stream.
 *
 * Generates the URI of the service to the output stream.
 *
 * @see @ref cool::ng::ip::service::visualize() "service::visualize()" for more details.
 */
inline std::ostream& operator <<(std::ostream& os, const service& val)
{
  return val.visualize(os, style::customary);
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
 * Read a service URI from the input character stream.
 *
 * Reads and parsers the service URI from the input stream and updates the service object  with the new
 * information. Reading of the input stream will stop at the first character that cannot be
 * interpreted as a part of the URI. This character will be the next character available in the input stream.
 *
 * @param is_ the input stream
 * @param svc_ service object to be updated
 * @exception cool::exception::bad_conversion thrown if the URI is malformed.
 * @see service::visualize() for more information on the URI format
 */
inline std::istream& operator >>(std::istream& is_, service& svc_)
{
  return detail::sin(is_, svc_);
}

} } } // namespaces cool::ng::ip

#if COOL_DONT_POLLUTE_GLOBAL_NAMESPACE != 1
using cool::ng::ip::operator "" _ip;
using cool::ng::ip::operator "" _ipv4;
using cool::ng::ip::operator "" _ipv6;
#endif

#endif
