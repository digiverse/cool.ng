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

#if !defined(BINARY_H_HEADER_GUARD)
#define BINARY_H_HEADER_GUARD

#include <initializer_list>
#include <cstring>
#include <cstdint>

#include "impl/platform.h"
#include "cool/ng/exception.h"

namespace cool { namespace ng { namespace util {

/**
 * Binary buffer.
 *
 * Binary buffer is a fixed size array of bytes that supports bitwise operations
 * on entire array.
 *
 * @tparam Size Number of bytes in binary buffer.
 */
template <std::size_t Size> class binary
{
 public:
  /**
   * Constructs a binary array and zeros its contents.
   */
  binary()
  {
    ::memset(m_data, 0, Size);
  }
  /**
   * Constructs a binary buffer from an array. The byte array is assumed
   * to be at least as large as this binary buffer.
   */
  binary(uint8_t const data[])
  {
    ::memcpy(m_data, data, Size);
  }
  /**
   * Constructs a binary buffer and initializes its values from the initialization
   * least. If the list is larger than the binary buffer, excessive bytes are
   * ignored. If the list is shorter than the buffer, only first few bytes
   * of the buffer are initialized and the remaining elements are set to 0.
   */
  binary(std::initializer_list<uint8_t> args)
  {
    ::memset(m_data, 0, Size);
    std::size_t limit = args.size() < Size ? args.size() : Size;
    std::size_t index = 0;

    for (auto iter = args.begin(); index < limit ; ++iter)
      m_data[index++] = *iter;
  }
  /**
   * Assigns the contents of the byte array to the consecutive bytes in
   * the binary buffer. It is assumed that the size of the array matches or
   * exceeds the size of the binary buffer.
   */
  binary& operator =(uint8_t const val[])
  {
    ::memcpy(m_data, val, Size);
    return *this;
  }
  /**
   * Assigns the contents of another binary buffer to this binary buffer. If
   * another binary is larger than this, only the first part of it is used. When
   * the other binary is smaller, only the first part of this binary will be
   * set, and the remaining bytes will be zeroed.
   */
  template <std::size_t OtherSize>
  binary& operator =(binary<OtherSize>& other)
  {
    if (OtherSize > Size)
    {
      ::memcpy(m_data, other.m_data, Size);
    }
    else
    {
      ::memset(m_data, 0, Size);
      ::memcpy(m_data, other.m_data, OtherSize);
    }
    return *this;
  }
#if 0
  /**
   * Returns reference to the byte at the specified index.
   *
   * @exception cool::exception::out_of_range thrown if the index is out of range
   */
  uint8_t& operator[](std::size_t index)
  {
    if (index > Size)
      throw exception::out_of_range();
    return m_data[index];
  }
  /**
   * Returns reference to the byte at the specified index.
   *
   * @exception cool::exception::out_of_range thrown if the index is out of range
   */
  const uint8_t& operator[](std::size_t index) const
  {
    if (index > Size)
      throw exception::out_of_range();
    return m_data[index];
  }
#endif
  /**
   * Returns raw pointer to the array of bytes of this binary buffer.
   */
  operator uint8_t* ()
  {
    return m_data;
  }
  /**
   * Returns raw pointer to the array of bytes of this binary buffer.
   */
  operator const uint8_t* () const
  {
    return m_data;
  }
  /**
   * Compares the contents of this binary buffer with the contents of another
   * binary buffer.
   *
   * @param other other binary buffer to compare with
   * @return true if all bytes in both binary buffers are equal, false otherwise
   */
  bool operator ==(const binary& other) const
  {
    return ::memcmp(m_data, other.m_data, Size) == 0;
  }
  /**
   * Compares the contents of this binary buffer with the contents of the
   * byte array. It is assumed that the byte array is large enough to compare
   * all the bytes of the binary buffer with the respective bytes in the array.
   *
   * @param data byte array to compare with
   * @return true if all compared bytes bytes are equal, false otherwise
   */
  bool operator ==(uint8_t const data[]) const
  {
    return ::memcmp(m_data, data, Size) == 0;
  }
  /**
   * Compares the contents of this binary buffer with the contents of another
   * binary buffer.
   *
   * @param other other binary buffer to compare with
   * @return false if all bytes in both binary buffers are equal, true otherwise
   */
  bool operator !=(const binary& other) const
  {
    return ::memcmp(m_data, other.m_data, Size) != 0;
  }
  /**
   * Compares the contents of this binary buffer with the contents of the
   * byte array. It is assumed that the byte array is large enough to compare
   * all the bytes of the binary buffer with the respective bytes in the array.
   *
   * @param data byte array to compare with
   * @return false if all compared bytes bytes are equal, true otherwise
   */
  bool operator !=(uint8_t const data[]) const
  {
    return ::memcmp(m_data, data, Size) != 0;
  }
  /**
   * Perform bitwise AND operation on all bytes in the binary buffer
   * with the corresponding bytes from another binary buffer and returns
   * result in a new binary buffer.
   *
   * @param other right hand side operand
   * @return new binary buffer with results
   */
  binary operator &(const binary& other) const
  {
    binary res;
    for (std::size_t i = 0; i < Size; ++i)
      res[i] = m_data[i] & other.m_data[i];
    return res;
  }
  /**
   * Perform bitwise AND operation on all bytes in the binary buffer
   * with the corresponding bytes from the specified byte array and returns
   * result in a new binary buffer.
   *
   * @param data byte array, right hand side operand
   * @return new binary buffer with results
   * @note The operator assumes that the byte array is at least as large as the
   *   binary buffer.
   */
  binary operator &(uint8_t const data[]) const
  {
    binary res;
    for (std::size_t i = 0; i < Size; ++i)
      res[i] = m_data[i] & data[i];
    return res;
  }
  /**
   * Performs bitwise OR operation on all bytes in the binary buffer
   * with the corresponding bytes from another binary buffer and returns
   * result in a new binary buffer.
   *
   * @param other right hand side operand
   * @return new binary buffer with results
   */
  binary operator |(const binary& other) const
  {
    binary res;
    for (std::size_t i = 0; i < Size; ++i)
      res[i] = m_data[i] & other.m_data[i];
    return res;
  }
  /**
   * Performs bitwise OR operation on all bytes in the binary buffer
   * with the corresponding bytes from the specified byte array and returns
   * result in a new binary buffer.
   *
   * @param data byte array, right hand side operand
   * @return new binary buffer with results
   * @note The operator assumes that the byte array is at least as large as the
   *   binary buffer.
   */
  binary operator |(uint8_t const data[]) const
  {
    binary res;
    for (std::size_t i = 0; i < Size; ++i)
      res[i] = m_data[i] & data[i];
    return res;
  }
  /**
   * Perform bitwise XOR operation on all bytes in the binary buffer
   * with the corresponding bytes from another binary buffer and returns
   * result in a new binary buffer.
   *
   * @param other right hand side operand
   * @return new binary buffer with results
   */
  binary operator ^(const binary& other) const
  {
    binary res;
    for (std::size_t i = 0; i < Size; ++i)
      res[i] = m_data[i] & other.m_data[i];
    return res;
  }
  /**
   * Perform bitwise XOR operation on all bytes in the binary buffer
   * with the corresponding bytes from the specified byte array and returns
   * result and returns result in a new binary buffer.
   *
   * @param data byte array, right hand side operand
   * @return new binary buffer with results
   * @note The operator assumes that the byte array is at least as large as the
   *   binary buffer.
   */
  binary operator ^(uint8_t const data[]) const
  {
    binary res;
    for (std::size_t i = 0; i < Size; ++i)
      res[i] = m_data[i] & data[i];
    return res;
  }
  /**
   * Performs bitwise NOT operation on all bytes in the binary buffer and returns
   * result in a new binary buffer.
   *
   * @return new binary buffer with results
   */
  binary operator~() const
  {
    binary res;
    for (std::size_t i = 0; i < Size; ++i)
      res[i] = ~m_data[i];
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
    ::memcpy(&m_data[start], data, n);
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
    ::memcpy(&m_data[start], data, n);
  }
  /**
   * Returns the binary buffer size in bytes
   */
  static std::size_t size()
  {
    return Size;
  }

 private:
  uint8_t m_data[Size];
};

} } } // namespace

#endif
