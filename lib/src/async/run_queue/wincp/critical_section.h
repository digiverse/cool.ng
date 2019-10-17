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

#if !defined(cool_ng_d2ef94ad_15ec_4748_9d69_a7d096d1b861)
#define      cool_ng_d2ef94ad_15ec_4748_9d69_a7d096d1b861

#define WIN32_MEAN_AND_LEAN
#include <windows.h>

namespace cool { namespace ng { namespace async { namespace impl {


class critical_section
{
  critical_section(const critical_section&) = delete;
  critical_section(critical_section&&) = delete;
  critical_section& operator =(const critical_section&) = delete;
  critical_section& operator =(critical_section&&) = delete;

 public:
  inline critical_section(long spin_count = 1000000)
  {
    InitializeCriticalSectionAndSpinCount(&m_cs, spin_count);
  }
  inline ~critical_section()
  {
    DeleteCriticalSection(&m_cs);
  }

  // BasicLockable requirements
  void lock()
  {
    EnterCriticalSection(&m_cs);
  }
  void unlock()
  {
    LeaveCriticalSection(&m_cs);
  }

  // Lockable requirements
  bool try_lock()
  {
#pragma warning( suppress: 4800 )
    return static_cast<bool>(TryEnterCriticalSection(&m_cs));
  }

private:
  CRITICAL_SECTION m_cs;
};

} } } }// namespace

#endif

