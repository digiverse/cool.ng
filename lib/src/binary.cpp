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

#include <string.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "cool/ng/impl/binary.h"

namespace cool { namespace ng { namespace util {

namespace detail { namespace binary {

void initialize(uint8_t* dst_, std::size_t size_, int8_t value_) noexcept
{
  ::memset(dst_, value_, size_);
}

void copy(uint8_t* dst_, std::size_t dst_size_, const uint8_t* src_,  std::size_t src_size_, bool zero) noexcept
{
  if (dst_size_ <= src_size_)
  {
    ::memcpy(dst_, src_ + (src_size_ - dst_size_), dst_size_);
  }
  else
  {
    ::memcpy(dst_ + (dst_size_ - src_size_), src_, src_size_);
    if (zero)
      initialize(dst_, dst_size_ - src_size_);
  }
}

void initialize(uint8_t* dst_, std::size_t size_, std::initializer_list<uint8_t> args_) noexcept
{
  std::size_t index = size_ > args_.size() ? size_ - args_.size() : 0;
  auto it = args_.begin();
  if (size_ < args_.size())
  {
    for (std::size_t i = 0; i < args_.size() - size_; ++i, ++it)
    ;
  }
  for ( ; index < size_ && it != args_.end(); ++index, ++it)
    dst_[index] = *it;
  if (size_ > args_.size())
    initialize(dst_, size_ - args_.size());
}

void op_and(
      uint8_t* dst_
    , std::size_t dst_size_
    , const uint8_t* lhs_
    , std::size_t lhs_size_
    , const uint8_t* rhs_
    , std::size_t rhs_size_
  ) noexcept
{
  auto limit = std::min(dst_size_, std::min(lhs_size_, rhs_size_));
  dst_ += dst_size_;
  lhs_ += lhs_size_;
  rhs_ += rhs_size_;
  for ( ; limit > 0; --limit)
    *--dst_= *--lhs_ & *--rhs_;
}

void op_or(
      uint8_t* dst_
    , std::size_t dst_size_
    , const uint8_t* lhs_
    , std::size_t lhs_size_
    , const uint8_t* rhs_
    , std::size_t rhs_size_
  ) noexcept
{
  auto limit = std::min(dst_size_, std::min(lhs_size_, rhs_size_));
  dst_ += dst_size_;
  lhs_ += lhs_size_;
  rhs_ += rhs_size_;
  for ( ; limit > 0; --limit)
    *--dst_= *--lhs_ | *--rhs_;
}

void op_xor(
      uint8_t* dst_
    , std::size_t dst_size_
    , const uint8_t* lhs_
    , std::size_t lhs_size_
    , const uint8_t* rhs_
    , std::size_t rhs_size_
  ) noexcept
{
  auto limit = std::min(dst_size_, std::min(lhs_size_, rhs_size_));
  dst_ += dst_size_;
  lhs_ += lhs_size_;
  rhs_ += rhs_size_;
  for ( ; limit > 0; --limit)
    *--dst_= *--lhs_ ^ *--rhs_;
}

void op_not(uint8_t dst_[], std::size_t dst_size_, uint8_t const src_[], std::size_t src_size_) noexcept
{
  auto limit = std::min(dst_size_, src_size_);
  dst_ += dst_size_;
  src_ += src_size_;

  for ( ; limit > 0; --limit)
    *--dst_ = ~*--src_;
}

std::ostream& visualize(std::ostream& os_, style style_, const uint8_t* data_, std::size_t size_)
{
  auto flag = std::ios_base::dec;

  switch (style_)
  {
    case style::decimal:
      break;

    case style::hex_no_prefix:
    case style::hex:
      flag = std::ios_base::hex;
      break;

    case style::octal_no_prefix:
    case style::octal:
      flag = std::ios_base::oct;
      break;
  }
  os_.setf(flag, std::ios_base::basefield);
  os_ << "{ ";
  for (int i = 0; i < size_; ++i)
  {
    switch (style_)
    {
      case style::hex:
        os_ << "0x";
        /* !!! */
      case style::hex_no_prefix:
         os_ << std::setw(2) << std::setfill('0') << static_cast<int>(data_[i]);
      break;

      case style::decimal:
        os_ << static_cast<int>(data_[i]);
        break;

      case style::octal:
        if (data_[i] != 0)
          os_ << "0";
        /* !!! */
      case style::octal_no_prefix:
        os_ << static_cast<int>(data_[i]);
        break;
    }
    if (i < size_ - 1)
      os_ << ", ";
  }
  os_ << " }";
  os_.unsetf(flag);
  return os_;
}


} } // namespace detail::binary

} } } // namespace
