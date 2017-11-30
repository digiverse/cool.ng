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

#include "cool/ng/error.h"

namespace cool { namespace ng { namespace error {

namespace {

const library_category the_cool_category;

} // namespace

std::error_code make_error_code(errc e)
{

  return std::error_code(static_cast<int>(e), the_cool_category);
}

std::error_code no_error()
{
  return std::error_code(0, the_cool_category);
}

const char* library_category::name() const NOEXCEPT_
{
  return "cool.ng";
}

std::string library_category::message(int ev) const
{
  static const char * const messages[] = {
    "not an error",
    "the runner requested through the weak_ptr no longer exists",
    "internal: dynamic cast from async::runner to user runner type unexplicably failed",
    "internal: no task context",
    "the object was in the wrong state to perform the requested operation",
    "the provided value was out of the valid value range",
    "the parameter value was not valid",
    "failed to convert provided value into the requested value"
    "the requested resource is busy and not available",
    "parsing of textual input failed"
  };
  static const char* const unknown = "unrecognized error";

  if (ev >= 0 && ev < sizeof(messages)/sizeof(const char*))
    return messages[ev];
  return unknown;
}



} } } // namespace
