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

#if defined(WINDOWS_TARGET)
# include <Windows.h>
#endif

#include "cool/ng/bases.h"

namespace cool { namespace ng {

namespace util
{

namespace detail {

unsigned long get_next_id()
{
  static std::atomic<unsigned long> id_(0);

  return ++id_;
}

}  // identified


named::named(const std::string& prefix)
{
  m_name = prefix + "-" + std::to_string(id());
}

named::named(named&& original)
{
  m_name = std::move(original.m_name);
  original.m_name = "moved-0";
}

named& named::operator =(const named& original)
{
  m_name = original.prefix() + "-" + std::to_string(id());
  return *this;
}

named& named::operator =(named&& original)
{
  m_name = std::move(original.m_name);
  original.m_name = "moved-0";
  return *this;
}

std::string named::prefix() const
{
  return m_name.substr(0, m_name.find_last_of('-'));
}


} } } // namespace
