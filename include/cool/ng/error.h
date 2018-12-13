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

#if !defined(cool_ng_f26a3cb0_dba1_4ce1_b25a_913f5951523a)
#define      cool_ng_f26a3cb0_dba1_4ce1_b25a_913f5951523a

#include <string>
#include <system_error>
#include "cool/ng/impl/platform.h"

namespace cool { namespace ng { namespace error {

enum class errc
{
  not_an_error = 0,
  no_runner = 1,
  bad_runner_cast = 2,
  no_task_context = 3,
  wrong_state = 4,
  out_of_range = 5,
  illegal_argument = 6,
  bad_conversion = 7,
  resource_busy = 8,
  parsing_error = 9,
  concurrency_problem = 10,
  not_available = 11,
  empty_object = 12,
  request_aborted = 13,
  request_rejected = 14,
  destination_unreachable = 15,
  request_failed = 16,
  not_found = 17,
  already_exists = 18
};

struct library_category : std::error_category
{
  dlldecl const char* name() const NOEXCEPT_ override;
  dlldecl std::string message(int ev) const override;
//  std::error_condition default_error_condition() const NOEXCEPT_ override;

};

dlldecl std::error_code make_error_code(errc);
dlldecl std::error_code no_error();

} } } // namespace

namespace std {

template <>
struct is_error_code_enum<cool::ng::error::errc>  : true_type { };

} // namespace

#endif
