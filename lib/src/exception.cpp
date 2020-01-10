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

#include <sstream>
#include <iomanip>
#include "cool/ng/exception.h"

#if defined(WINDOWS_TARGET)

# include <Windows.h>
# include <DbgHelp.h>
# include <sstream>
# include <iomanip>

#else

# include <cstdlib>
# include <execinfo.h>
# include <errno.h>

#endif

namespace cool { namespace ng { namespace exception {

// ---
// --- backtrace class ---------------------------------------------------------
// ---
backtrace::backtrace(bool capture_) NOEXCEPT_
  : m_count(0)
{
  if (!capture_)
    return;

  try
  {
#if defined(WINDOWS_TARGET)
    m_count = CaptureStackBackTrace(1, static_cast<DWORD>(m_traces.size()), m_traces.data(), nullptr);
#else
    m_count = ::backtrace(m_traces.data(), m_traces.size());
#endif
    if (m_count < 0)
      m_count = 0;
  }
  catch (...)
  { /* noop */ }
}

stack_address backtrace::operator [](std::size_t idx_) const
{
  if (idx_ >= size())
    throw out_of_range();
  return m_traces[idx_];
}

std::vector<std::string> backtrace::symbols() const NOEXCEPT_
{
  std::vector<std::string> ret;
  try
  {
    if (size() > 0)
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
      for (std::size_t i = 0; i < size(); ++i)
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

      auto syms = ::backtrace_symbols(m_traces.data(), size());
      if (syms != nullptr)
      {
        for (std::size_t i = 0; i < size(); ++i)
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

// ---
// --- base class --------------------------------------------------------------
// ---

base::base(const std::string& msg_, bool backtrace_) NOEXCEPT_
    : runtime_error(msg_)
{
  try
  {
    if (backtrace_)
      m_backtrace = std::make_shared<backtrace>(true);
  }
  catch (...)
  { /* noop */ }
}

std::string base::message() const
{
  return std::string(name()) + ": " + what() + "\n  " + code().message() + "\n";
}

std::error_code base::code() const NOEXCEPT_
{
  static const std::error_code errc_ = error::make_error_code(error::errc::not_set);

  return errc_;
}

const char* base::name() const NOEXCEPT_
{
  return "base";
}

const backtrace& base::stack_backtrace() const
{
  static const backtrace empty_(false);

  return m_backtrace ? *m_backtrace : empty_;
}

// -------

std::string system_error::message() const
{
  return std::string(name()) + ": " + what() + "\n  [errno = " +
     std::to_string(code().value()) + "] " + code().message() + "\n";
}

// -------
system_error::system_error(const std::string& msg_, bool backtrace_)
#if defined(WINDOWS_TARGET)
    : system_error_code(GetLastError())
    , base(msg_, backtrace_)
#else
    : system_error_code(errno)
    , base(msg_, backtrace_)
#endif
{
  m_errc = std::error_code(static_cast<int>(*this), std::system_category());
}

system_error::system_error(int code_, const std::string& msg_, bool backtrace_)
#if defined(WINDOWS_TARGET)
    : system_error_code(code_)
    , base(msg_, backtrace_)
#else
    : system_error_code(errno)
    , base(msg_, backtrace_)
#endif
{
  m_errc = std::error_code(static_cast<int>(*this), std::system_category());
}

#if defined(WINDOWS_TARGET)
network_error::network_error(const std::string& msg_, bool backtrace_)
    : system_error(WSAGetLastError(), msg_, backtrace_)
{ /* noop */ }

network_error::network_error(int code_, const std::string& msg_, bool backtrace_)
    : system_error(code_, msg_, backtrace_)
{ /* noop */ }
#endif


std::string to_string(const base& ex)
{
  std::stringstream os;
  os << ex.message();
  if (ex.stack_backtrace().size() > 0)
  {
    auto syms = ex.stack_backtrace().symbols();
    os << "  **** Call stack backtrace:\n";
    for (int i = 0; i < syms.size(); ++i)
      os << "\t" << syms[i] << "\n";
  }
  else
  {
    os << "  **** No call stack backtrace\n";
  }
  return os.str();
}


} } } // namespace
