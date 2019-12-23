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

#if !defined(cool_ng_fe6a3cb0_03a1_4de1_b25a_422f5121523a)
#define      cool_ng_fe6a3cb0_03a1_4de1_b25a_422f5121523a

#include <initializer_list>
#include <string>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <array>

#include "cool/ng/exception.h"
#include "impl/platform.h"
#include "impl/binary.h"

namespace cool { namespace ng { namespace util {

/**
 * Binary buffer.
 *
 * Binary buffer is a fixed size array of bits that supports the bitwise operations on the entire buffer. The
 * binary is modeled as a sequence of bits laid out in the following order:
 *
 * <pre>
 *   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 ...           8*size()-1
 * +-----------------------+-----------------------+-...----------------------+
 * |MSB                    |                       |                       LSB|
 * +-----------------------+-----------------------+-...----------------------+
 *      byte index 0           byte index 1              byte index size()-1
 * </pre>
 *
 * The MSB (most significant bit) has bit index 0 and the LSB (least significant bit) has bit index <tt>8 * size() - 1</tt>.
 * @tparam Size Number of bytes in binary buffer.
 */
template <std::size_t Size> class binary : public std::array<uint8_t, Size>
{
 public:
  /**
   * Visual style to use  for textual presentation of the binary.
   *
   * All visual styles will display the byte values in the form of initializer list. The chosen visual style determines
   * the numberic base and the form of the displayed numbers. The following are valid visual styles:
   *   - <tt>style:decimal</tt> will display bytes values as decimal numbers, eg: <tt>{ 12, 42, 255, 197 }</tt>
   *   - <tt>style:hex</tt> will display bytes values as hexadecimal  numbers, eg: <tt>{ 0xc, 0x2a, 0xff, 0xc5 }</tt>
   *   - <tt>style:hex_no_prefix</tt> will display bytes values as hexadecimal  numbers without the leading
   *    <tt>0x</tt> prefix, eg: <tt>{ c, 2a, ff, c5 }</tt>
   *   - <tt>style:octal</tt> will display bytes values as octal  numbers, eg: <tt>{ 014, 052, 0377, 0305 }</tt>
   *   - <tt>style:octal_no_prefix</tt> will display bytes values as octal  numbers without the leading
   *    <tt>0</tt> prefix, eg: <tt>{ 14, 52, 377, 305 }</tt>
   */
  using style = detail::binary::style;

 public:
  /**
   * Construct a zeroed binary.
   *
   * Constructs a new binary and sets  the value of all bits to 0.
   */
  inline binary() noexcept
  {
    detail::binary::initialize(this->data(), this->size());
  }
  /**
   * Construct a binary buffer from the array of bytes.
   *
   * Construct a binary buffer from the array of bytes. The specified byte array is assumed
   * to contain exactly the size of the binary bytes.
   *
   * @param data_ byte array to construct the binary from.
   */
  inline binary(uint8_t const data_[]) noexcept
  {
    detail::binary::copy(this->data(), this->size(), data_, this->size());
  }
  /**
   * Construct a binary buffer from the array of bytes.
   *
   * Construct a binary buffer from the array of bytes with the specified size. If the specfied array is smaller
   * than the size of this binary, only the <tt>size_</tt> of bytes will be copied to the rightmost
   * (LSB) bytes of the binary with the zero-padding to the left (MSB) bytes. If the specified array is larger
   * than the size of this binary, only the rightmost bytes from the specified array will be copied.
   * @param data_ byte array to construct the binary from.
   * @param size_ size of the byte array
   */
  inline binary(uint8_t const data_[], std::size_t size_) noexcept
  {
    detail::binary::copy(this->data(), this->size(), data_, size_);
  }
  /**
   * Construct a binary buffer from the initializer list.
   *
   * Constructs a binary buffer and initializes its values from the initializer list. If the list is larger than this
   * binary, only thesize of this  binary bytes from the end of the list will be used. If the list is smaller than
   * this binary, the righmost bytes (LSB) will be set from the list, with zero-padding to the left.
   * @param list_ initializer list to construct the binary from
   */
  inline binary(std::initializer_list<uint8_t> list_) noexcept
  {
    detail::binary::initialize(this->data(), this->size(), list_);
  }
  /**
   * Construct a binary from another binary
   *
   * Construct a binary from the fixed size array.  If the array is larger than this binary, only the
   * rightmost bytes from the fixed size array will be used. If the array is smaller than
   * this  binary, the righmost bytes (LSB) will be set from the the array bytes, with zero-padding to the left.
   * @tparam ArrSize the size, in bytes, of the array, auto-deduced.
   * @param arr_ the fixed size array to construct this binary from
   */
  template <std::size_t ArrSize>
  inline binary(const std::array<uint8_t, ArrSize>& arr_) noexcept
  {
    detail::binary::copy(this->data(), this->size(), arr_.data(), arr_.size());
  }
  /**
   * Assignment operator.
   *
   * Assigns the contents of the byte array to the consecutive bytes in this binary. The specified byte array
   * is assumed to contain exactly the size of this binary bytes.
   *
   * @param data_ byte array which elements to assign to this binary..
   * @return <tt>*this</tt>
   */
  inline binary& operator =(uint8_t const data_[]) noexcept
  {
    detail::binary::copy(this->data(), this->size(), data_, this->size());
    return *this;
  }
  /**
   * Assignment operator.
   *
   * Assigns the contents of the fixed size array to this binary. If the array is larger than this binary, only the
   * rightmost bytes from the array will  be used. If the array is smaller than this binary, bytes
   * from the array will be copied to the rightmost (LSB) byte positions of this binary, and the remaining
   * leftmost bytes will remain intact.
   *
   * @tparam ArrSize the size, in bytes, of the array, auto-deduced.
   * @param arr_ the fixed size array whcih conents to copy to this binary
   * @return <tt>*this</tt>
   */
  template <std::size_t ArrSize>
  inline binary& operator =(const std::array<uint8_t, ArrSize>& arr_) noexcept
  {
    detail::binary::copy(this->data(), this->size(), arr_.data(), arr_.size(), false);
    return *this;
  }
  // ----- Bitwise AND
  /**
   * Perform bitwise AND and assign.
   *
   * Perform bitwise AND operation on all bytes in the binary with the corresponding bytes from the
   * specified byte array. The size of the byte array is assumed to equal to the size of the binary.
   *
   * @param rhs_ byte array, right hand side operand
   * @return <tt>*this</tt>
   */
  inline binary& operator &=(uint8_t const rhs_[]) noexcept
  {
    detail::binary::op_and(this->data(), this->size(), rhs_, this->size());
    return *this;
  }
  /**
   * Perform bitwise AND and assign.
   *
   * Perform bitwise AND operation on all bytes in the binary with the corresponding bytes from the
   * fixed size array and assigns the result to this binary. If the sizes of the operands differ, only the
   * rightmost bytes of the longer are used or modified.
   *
   * @param rhs_ right hand side operand
   * @return <tt>*this</tt>
   */
  template <std::size_t ArrSize>
  inline binary& operator &=(const std::array<uint8_t, ArrSize>& rhs_) noexcept
  {
    detail::binary::op_and(this->data(), this->size(), rhs_.data(), rhs_.size());
    return *this;
  }
  /**
   * Perform bitwise AND on the binary and the byte array.
   *
   * It is assumed that the byte array is of the same size as binary.
   * @param lhs_ left hand side operand
   * @param rhs_ right hand side operand
   * @return binary containing the result of the operation
   * @see operator &=(const std::array<uint8_t&, ArrSize>& rhs_)
   */
  friend inline binary operator &(binary lhs_, uint8_t const rhs_[]) noexcept
  {
    lhs_ &= rhs_;
    return lhs_;
  }
  /**
   * Perform bitwise AND on the binary and the  fixed size array.
   *
   * @param lhs_ left hand side operand
   * @param rhs_ right hand side operand
   * @return binary containing the result of the operation
   * @see operator &=(const std::array<uint8_t&, ArrSize>& rhs_)
   * @note While this operator overload retains commutative property as far as the values of affected bytes
   * are concerned, it is not commutative regarding the type of the result.. The type of the result is the type
   * of the left operand.
   */
  template <std::size_t ArrSize>
  friend inline binary operator &(binary lhs_, const std::array<uint8_t, ArrSize>& rhs_) noexcept
  {
    lhs_ &= rhs_;
    return lhs_;
  }
  // ----- Bitwise OR
  /**
   * Perform bitwise OR and assign.
   *
   * Perform bitwise OR operation on all bytes in the binary with the corresponding bytes from the
   * specified byte array. The size of the byte array is assumed to equal to the size of the binary.
   *
   * @param rhs_ byte array, right hand side operand
   * @return <tt>*this</tt>
   */
  inline binary& operator |=(uint8_t const rhs_[]) noexcept
  {
    detail::binary::op_or(this->data(), this->size(), rhs_, this->size());
    return *this;
  }
  /**
   * Perform bitwise OR and assign.
   *
   * Perform bitwise OR operation on all bytes in the binary with the corresponding bytes from the
   * fixed size array and assigns the result to this binary. If the sizes of the operands differ, only the
   * rightmost bytes of the longer are used or modified.
   *
   * @param rhs_ right hand side operand
   * @return <tt>*this</tt>
   */
  template <std::size_t ArrSize>
  inline binary& operator |=(const std::array<uint8_t, ArrSize>& rhs_) noexcept
  {
    detail::binary::op_or(this->data(), this->size(), rhs_.data(), rhs_.size());
    return *this;
  }
  /**
   * Perform bitwise OR on the binary and the byte array.
   *
   * It is assumed that the byte array is of the same size as binary.
   * @param lhs_ left hand side operand
   * @param rhs_ right hand side operand
   * @return binary containing the result of the operation
   * @see operator |=(const std::array<uint8_t&, ArrSize>& rhs_)
   */
  friend inline binary operator |(binary lhs_, uint8_t const rhs_[]) noexcept
  {
    lhs_ |= rhs_;
    return lhs_;
  }
  /**
   * Perform bitwise OR on the binary and the  fixed size array.
   *
   * @param lhs_ left hand side operand
   * @param rhs_ right hand side operand
   * @return binary containing the result of the operation
   * @see operator |=(const std::array<uint8_t&, ArrSize>& rhs_)
   * @note While this operator overload retains commutative property as far as the values of affected bytes
   * are concerned, it is not commutative regarding the type of the result.. The type of the result is the type
   * of the left operand.
   */
  template <std::size_t ArrSize>
  friend inline binary operator |(binary lhs_, const std::array<uint8_t, ArrSize>& rhs_) noexcept
  {
    lhs_ |= rhs_;
    return lhs_;
  }
  // ----- Bitwise XOR
  /**
   * Perform bitwise XOR and assign.
   *
   * Perform bitwise XOR operation on all bytes in the binary with the corresponding bytes from the
   * specified byte array. The size of the byte array is assumed to equal to the size of the binary.
   *
   * @param rhs_ byte array, right hand side operand
   * @return <tt>*this</tt>
   */
  inline binary& operator ^=(uint8_t const rhs_[]) noexcept
  {
    detail::binary::op_xor(this->data(), this->size(), rhs_, this->size());
    return *this;
  }
  /**
   * Perform bitwise XOR and assign.
   *
   * Perform bitwise XOR operation on all bytes in the binary with the corresponding bytes from the
   * fixed size array and assigns the result to this binary. If the sizes of the operands differ, only the
   * rightmost bytes of the longer are used or modified.
   *
   * @param rhs_ right hand side operand
   * @return <tt>*this</tt>
   */
  template <std::size_t ArrSize>
  inline binary& operator ^=(const std::array<uint8_t, ArrSize>& rhs_) noexcept
  {
    detail::binary::op_xor(this->data(), this->size(), rhs_.data(), rhs_.size());
    return *this;
  }
  /**
   * Perform bitwise XOR on the binary and the byte array.
   *
   * It is assumed that the byte array is of the same size as binary.
   * @param lhs_ left hand side operand
   * @param rhs_ right hand side operand
   * @return binary containing the result of the operation
   * @see operator ^=(const std::array<uint8_t&, ArrSize>& rhs_)
   */
  friend inline binary operator ^(binary lhs_, uint8_t const rhs_[]) noexcept
  {
    lhs_ ^= rhs_;
    return lhs_;
  }
  /**
   * Perform bitwise XOR on the binary and the  fixed size array.
   *
   * @param lhs_ left hand side operand
   * @param rhs_ right hand side operand
   * @return binary containing the result of the operation
   * @see operator |=(const std::array<uint8_t&, ArrSize>& rhs_)
   * @note While this operator overload retains commutative property as far as the values of affected bytes
   * are concerned, it is not commutative regarding the type of the result.. The type of the result is the type
   * of the left operand.
   */
  template <std::size_t ArrSize>
  friend inline binary operator ^(binary lhs_, const std::array<uint8_t, ArrSize>& rhs_) noexcept
  {
    lhs_ ^= rhs_;
    return lhs_;
  }
/**
 * Perform bitwise NOT on the fixed size array.
 *
 * Performs bitwise NOT operation on all bytes in the fixed size array and returns
 * result in a new binary.
 *
 * @return new binary with the result
 */
 friend inline binary operator ~(const std::array<uint8_t, Size>& op_)
 {
    binary res;
    detail::binary::op_not(res.data(), res.size(), op_.data(), op_.size());
    return res;
 }
  /**
   * Set byte values in the binary buffer from the byte values in the
   * specified byte array starting at the specified offset.
   *
   * @param start start offset
   * @param data input data array
   * @exception cool::exception::out_of_range thrown if start offset is beyond
   *   the binary buffer size
   * @note The method assumes that the input byte array is large enough
   */
  void set(int start, uint8_t const data[])
  {
    if (start >= Size)
      throw exception::out_of_range();
    int n = Size - start;
    ::memcpy(&this->data()[start], data, n);
  }
  /**
   * Set byte values in the binary buffer from the byte values in the
   * specified byte array starting at the specified offset.
   *
   * @param start start offset
   * @param data input data array
   * @param size number of bytes to set
   * @exception cool::exception::out_of_range thrown if start offset is beyond
   *   the binary buffer size
   * @note The actual number of bytes set may be less than the requested size
   *   if the @c start + @c size would exceed the binary buffer size
   */
  void set(int start, uint8_t const data[], int size)
  {
    if (start >= Size)
      throw exception::out_of_range();

    int n = Size - start;
    if (n > size)
      n = size;
    ::memcpy(&this->data()[start], data, n);
  }
  /**
   * Present the binary in the textual format.
   *
   * Writes the byte values of the binary into the output formatted character stream. The byte values are
   * written according to the selected visual @ref style.
   *
   * @param os_ reference to the output character stream
   * @param style_ visual @ref style to use, <tt>style::hex</tt> being the default style
   * @return reference to the output character stream
   */
  inline std::ostream& visualize(std::ostream& os_, style style_ = style::hex) const
  {
    return detail::binary::visualize(os_, style_, this->data(), this->size());
  }
  /**
   * Operator overload to present the binary in the textual format.
   *
   * Writes the byte values of the binary into the output formatted character stream using the <tt>style::hex</tt>
   * visual @ref style.
   * @param os_ reference to the output character stream
   * @param b_ binary to write to the output stream
   * @return reference to the output character stream
   */
  friend inline std::ostream& operator <<(std::ostream& os_, const binary& b_)
  {
    return b_.visualize(os_);
  }
  /**
   * @defgroup comparison Comparison operators
   *
   * The comparison operator overloads compare two binary instances, or a binary with the array of bytes.
   * When comparing a binary with the array  of bytes, it is assumed that the length of the array matches the
   * size of the binary. The comparison is based on the following rules:
   *
   *   - if the operands differ in size, they are considered not to be equal. The shorter operand is considered
   *   to be _less_ than the longer;
   *   - if sizes are the same, the contents of the operands are lexicographically compared; the first
   *   mismatching byte value determines whether one operand is _less_ than the other
   */
  /// @ingroup comparison
  /// Comparison operator.
  template<std::size_t Size2>
  friend inline bool operator ==(const binary& lhs_, const binary<Size2>& rhs_) noexcept
  {
    return lhs_.equals(rhs_);
  }
  /// @ingroup comparison
  /// Comparison operator.
  friend inline bool operator ==(const binary& lhs_, const uint8_t rhs_[]) noexcept
  {
    return lhs_.equals(rhs_);
  }
  /// @ingroup comparison
  /// Comparison operator.
  friend inline bool operator ==(const uint8_t lhs_[], const binary& rhs_) noexcept
  {
    return rhs_.equals(lhs_);
  }
  /// @ingroup comparison
  /// Comparison operator.
  template<std::size_t Size2>
  friend inline bool operator !=(const binary& lhs_, const binary<Size2>& rhs_) noexcept
  {
    return !(lhs_ == rhs_);
  }
  /// @ingroup comparison
  /// Comparison operator.
  friend inline bool operator !=(const binary& lhs_, const uint8_t rhs_[]) noexcept
  {
    return !(lhs_ == rhs_);
  }
  /// @ingroup comparison
  /// Comparison operator.
  friend inline bool operator !=(const uint8_t lhs_[], const binary& rhs_) noexcept
  {
    return !(lhs_ == rhs_);
  }
  /// @ingroup comparison
  /// Comparison operator.
  template<std::size_t Size2>
  friend inline bool operator <(const binary& lhs_, const binary<Size2>& rhs_) noexcept
  {
    return lhs_.less(rhs_);
  }
  /// @ingroup comparison
  /// Comparison operator.
  friend inline bool operator <(const binary& lhs_, uint8_t const rhs_[]) noexcept
  {
    return lhs_.less(rhs_);
  }
  /// @ingroup comparison
  /// Comparison operator.
  friend inline bool operator <(uint8_t const lhs_[], const binary& rhs_) noexcept
  {
    return !rhs_.less_eq(lhs_);
  }
  /// @ingroup comparison
  /// Comparison operator.
  template<std::size_t Size2>
  friend inline bool operator <=(const binary& lhs_, const binary<Size2>& rhs_) noexcept
  {
    return lhs_.less_eq(rhs_);
  }
  /// @ingroup comparison
  /// Comparison operator.
  friend inline bool operator <=(const binary& lhs_, uint8_t const rhs_[]) noexcept
  {
    return lhs_.less_eq(rhs_);
  }
  /// @ingroup comparison
  /// Comparison operator.
  friend inline bool operator <=(uint8_t const lhs_[], const binary& rhs_) noexcept
  {
    return !rhs_.less(lhs_);
  }
  /// @ingroup comparison
  /// Comparison operator.
  template<std::size_t Size2>
  friend inline bool operator >(const binary& lhs_, const binary<Size2>& rhs_) noexcept
  {
    return rhs_ < lhs_;
  }
  /// @ingroup comparison
  /// Comparison operator.
  friend inline bool operator >(const binary& lhs_, uint8_t const rhs_[]) noexcept
  {
    return rhs_ < lhs_;
  }
  /// @ingroup comparison
  /// Comparison operator.
  friend inline bool operator >(uint8_t const lhs_[], const binary& rhs_) noexcept
  {
    return rhs_ < lhs_;
  }
  /// @ingroup comparison
  /// Comparison operator.
  template<std::size_t Size2>
  friend inline bool operator >=(const binary& lhs_, const binary<Size2>& rhs_) noexcept
  {
    return rhs_ <= lhs_;
  }
  /// @ingroup comparison
  /// Comparison operator.
  friend inline bool operator >=(const binary& lhs_, uint8_t const rhs_[]) noexcept
  {
    return rhs_ <= lhs_;
  }
  /// @ingroup comparison
  /// Comparison operator.
  friend inline bool operator >=(uint8_t const lhs_[], const binary& rhs_) noexcept
  {
    return rhs_ <= lhs_;
  }

 private:
  inline bool equals(const uint8_t data_[]) const noexcept
  {
    return ::memcmp(this->data(), data_, this->size()) == 0;
  }
  template<std::size_t ArrSize>
  inline bool equals(const binary<ArrSize>& arr_) const noexcept
  {
    return this->size() != arr_.size() ? false : (0 == ::memcmp(this->data(), arr_.data(),  this->size()));
  }
  inline bool less(uint8_t const data_[]) const noexcept
  {
    return ::memcmp(this->data(), data_, this->size()) < 0;
  }
  template<std::size_t ArrSize>
  inline bool less(const binary<ArrSize>& arr_) const noexcept
  {
    return this->size() != arr_.size() ?
          (this->size() < arr_.size())  :
          (::memcmp(this->data(), arr_.data(), this->size()) < 0);

  }
  inline bool less_eq(uint8_t const data_[]) const noexcept
  {
    return !(::memcmp(this->data(), data_, this->size()) > 0);
  }
  template<std::size_t ArrSize>
  inline bool less_eq(const binary<ArrSize>& arr_) const noexcept
  {
    return this->size() != arr_.size() ?
          (this->size() < arr_.size())  :
          !(::memcmp(this->data(), arr_.data(), this->size()) > 0);

  }

};

} } } // namespace

#endif
