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

#include "cool/ng/exception.h"

#if defined(WINDOWS_TARGET)

# include <Windows.h>
# include <DbgHelp.h>
# include <sstream>
# include <iomanip>

#else

# include <cstdint>
# include <execinfo.h>
# include <errno.h>

#endif

namespace cool { namespace ng { namespace exception {

backtrace::backtrace(std::size_t depth_) NOEXCEPT_ : m_traces(depth_, null_address)
{
  try
  {
    if (depth_ == 0)
      return;

#if defined(WINDOWS_TARGET)
    auto count = CaptureStackBackTrace(1, static_cast<DWORD>(depth_), m_traces.data(), nullptr);
#else
    auto count = ::backtrace(m_traces.data(), depth_);
#endif
    if (count < 0)
      m_traces.clear();
    else if (count < depth_)
      m_traces.resize(count);
  }
  catch (...)
  { /* noop */ }
}

const std::vector<stack_address>& backtrace::traces() const NOEXCEPT_
{
  return m_traces;
}

std::vector<std::string> backtrace::symbols() const NOEXCEPT_
{
  std::vector<std::string> ret;
  try
  {
    if (m_traces.size() > 0)
    {
#if defined(WINDOWS_TARGET)

      HANDLE process = GetCurrentProcess();
      if (!SymInitialize(process, NULL, TRUE))
        return ret;

      unsigned char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME  * sizeof(TCHAR)];
      PSYMBOL_INFO symbol = reinterpret_cast<PSYMBOL_INFO>(buffer);
      symbol->MaxNameLen = MAX_SYM_NAME;
      symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

      DWORD64 displacement;
      for (std::size_t i = 0; i < m_traces.size(); ++i)
      {
        displacement = 0;
        if (SymFromAddr(process, reinterpret_cast<int64_t>(m_traces[i]), &displacement, symbol))
        {
          ret.push_back(symbol->Name);
        }
        else
        {
          std::stringstream ss;
          ss << "At address 0x" << std::hex << std::setw(8)
             << std::setfill('0') << std::right << m_traces[i];
          ret.push_back(ss.str());
        }
      }

#else

      auto syms = ::backtrace_symbols(m_traces.data(), m_traces.size());
      if (syms != nullptr)
      {
        for (std::size_t i = 0; i < m_traces.size(); ++i)
          ret.push_back(syms[i]);
        std::free(syms);
      }

#endif
    }
  }
  catch (...)
  { /* noop */ }
  return ret;
}

// -------
system_error::system_error(std::size_t depth_) NOEXCEPT_
#if defined(WINDOWS_TARGET)
    : base(std::error_code(GetLastError(), std::system_category()), depth_)
#else
    : base(std::error_code(errno, std::system_category()), depth_)
#endif
{ /* noop */ }

#if defined(WINDOWS_TARGET)
socket_failure::socket_failure(std::size_t depth_) NOEXCEPT_
    : base(std::error_code(GetLastError(), std::system_category()), depth_)
{ /* noop */ }
#endif

} } } // namespace
