/*
 *  Copyright (c) 2017 Digiverse d.o.o.
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

#if !defined(cool_ng_f36a3cb0_dda1_4ce1_b25a_912f5951523a)
#define      cool_ng_f36a3cb0_dda1_4ce1_b25a_912f5951523a

#if !defined(COOL_NG_WINDOWS_TARGET)
#  if defined(_MSC_VER)
#    define COOL_NG_WINDOWS_TARGET
#  endif
#endif

#if !defined(COOL_NG_OSX_TARGET)
#  if defined(__APPLE__)
#    define COOL_NG_OSX_TARGET
#  endif
#endif

#if !defined(COOL_NG_LINUX_TARGET)
#  if defined(__linux)
#    define COOL_NG_LINUX_TARGET
#  endif
#endif

#if defined(COOL_NG_WINDOWS_TARGET) && !defined(COOL_NG_STATIC_LIBRARY)
#  if defined(COOL_NG_BUILD)
#    define dlldecl __declspec( dllexport )
#  else
#    define dlldecl __declspec( dllimport )
#  endif
#else
#  define dlldecl
#endif

#endif
