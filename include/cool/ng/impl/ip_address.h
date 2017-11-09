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

#if !defined(cool_ng_d2aa94ac_15ec_4A48_9d69_a7d096d1b861)
#define      cool_ng_d2aa94ac_15ec_4A48_9d69_a7d096d1b861

#include <initializer_list>
#include "cool/ng/exception.h"
#include "cool/ng/binary.h"
#include <cstring>

namespace cool { namespace ng { namespace net {

enum class style;

namespace detail {


template <std::size_t Size>
cool::ng::util::binary<Size> calculate_mask(std::size_t length)
{
  cool::ng::util::binary<Size> result;

  if (length > Size * 8)
    throw exception::illegal_argument("Network mask length exceeds data size");

  std::size_t limit = length >> 3;
  for (std::size_t i = 0; i < limit; ++i)
    result[i] = 0xff;

  std::size_t limit2 = length & 0x07;
  if (limit2 > 0)
  {
    uint8_t aux = 0x80;
    for (int i = 1; i < limit2; ++i)
    {
      aux >>= 1;
      aux |= 0x80;
    }
    result[limit] = aux;
  }
  return result;
}

} // namespace details

} } } // namespace

#endif
