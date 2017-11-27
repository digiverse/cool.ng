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

#if !defined(cool_ng_f36a3cb0_dda1_4de1_b25a_912f5951523a)
#define      cool_ng_f36a3cb0_dda1_4de1_b25a_912f5951523a



#if defined(WINDOWS_TARGET) && !defined(COOL_NG_STATIC_LIBRARY)
#  if defined(COOL_NG_BUILD)
#    define dlldecl __declspec( dllexport )
#  else
#    define dlldecl __declspec( dllimport )
#  endif
#else
#  define dlldecl
#endif

#if defined(WINDOWS_TARGET)

// MS compilers don't allow explicit virtual conversion operators!!!
#define EXPLICIT_

#define INETADDR_STORAGE_
// Visual Studio 2013 does not support c++11 specifiers noexcept
// and constexpr
# if _MSC_VER == 1800
#  define NOEXCEPT_
#  define CONSTEXPR_
# else
#  define NOEXCEPT_ noexcept
#  define CONSTEXPR_ constexpr
# endif

#else

#define EXPLICIT_  explicit
#define NOEXCEPT_  noexcept
#define CONSTEXPR_ constexpr

#endif


#endif
