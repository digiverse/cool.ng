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

#if !defined(cool_ng_d2de94ac_15ec_4A48_9d69_ced096d1b861)
#define      cool_ng_d2de94ac_15ec_4A48_9d69_ced096d1b861

#include <initializer_list>
#include <iostream>
#include "cool/ng/impl/platform.h"
#include "cool/ng/exception.h"

namespace cool { namespace ng { namespace util {

namespace detail {

namespace binary {

enum class style { decimal, hex, hex_no_prefix, octal, octal_no_prefix };


dlldecl void initialize(uint8_t* dst_, std::size_t size_, int8_t value_ = 0) noexcept;
dlldecl void initialize(uint8_t* dst_, std::size_t size_, std::initializer_list<uint8_t> args) noexcept;
dlldecl void copy(
    uint8_t* dst_
  , std::size_t dst_size_
  , const uint8_t* src_
  , std::size_t src_size_
  , bool zero = true) noexcept;
dlldecl void op_and(
    uint8_t* dst_
  , std::size_t dst_size_
  , const uint8_t* lhs_
  , std::size_t lhs_size_
  , const uint8_t* rhs_
  , std::size_t rhs_size_
) noexcept;
inline void op_and(uint8_t* lhs_, std::size_t lhs_size_, const uint8_t* rhs_, std::size_t rhs_size_) noexcept
{
  return op_and(lhs_, lhs_size_, lhs_, lhs_size_, rhs_, rhs_size_);
}
dlldecl void op_or(
    uint8_t* dst_
  , std::size_t dst_size_
  , const uint8_t* lhs_
  , std::size_t lhs_size_
  , const uint8_t* rhs_
  , std::size_t rhs_size_
) noexcept;
inline void op_or(uint8_t* lhs_, std::size_t lhs_size_, const uint8_t* rhs_, std::size_t rhs_size_) noexcept
{
  return op_or(lhs_, lhs_size_, lhs_, lhs_size_, rhs_, rhs_size_);
}
dlldecl void op_xor(
    uint8_t* dst_
  , std::size_t dst_size_
  , const uint8_t* lhs_
  , std::size_t lhs_size_
  , const uint8_t* rhs_
  , std::size_t rhs_size_
) noexcept;
inline void op_xor(uint8_t* lhs_, std::size_t lhs_size_, const uint8_t* rhs_, std::size_t rhs_size_) noexcept
{
  return op_xor(lhs_, lhs_size_, lhs_, lhs_size_, rhs_, rhs_size_);
}
dlldecl void op_not(uint8_t dst[], std::size_t dst_size_, uint8_t const src_[], std::size_t src_size_) noexcept;
dlldecl std::ostream& visualize(std::ostream& os_, style style_, const uint8_t* data_, std::size_t size_);

} } // namespace detail::binary

} } } // namespace cool::ng::util

#endif
