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

// !!!! NOTE: Do not reorder this enumeration without adjusting messages[]
// !!!!       string array in the implementaion file. New codes are to
// !!!!       appended at the end of the list and their corresponding text
// !!!!       messages to the end of the said array.
/**
 * @ingroup coolerrc
 * Error codes enumeration for Cool.NG error codes
 */
enum class errc
{
         not_an_error = 0,    //!< Not an error
         not_set      = 1,    //!< Potential error condition detected but no specific error code was set
         no_runner,           //!< The requested @ref async::runner "runner" instance no longer exists
         bad_runner_cast,     //!< The target type of the @c dynamic_cast is not a @ref async::runner "runner"
         operation_failed,    //!< Requested operation failed for the unspecified reason
         invalid_state,       //!<  The current state was either unexpected or prohibits the operation to proceed
         out_of_range,        //!< The provided value was out of the valid value range for the operational context.
         illegal_argument,    //!< The value of one or more parameters was not valid
         bad_conversion,      //!< The value cannot be converted into the requested data type
         resource_busy,       //!< The requested resource is already spoken for
/* 10 */ parsing_error,       //!< The input text failed to parse correctly
         concurrency_problem, //!< Object that is not thread safe detected concurrent use
         empty_object,        //!< The object w as found empty and non-functional
         request_aborted,     //!< The pending request was aborted
         request_failed,      //!< The asynchronous request has failed
         not_found,           //!< The item was not found
         already_exists,      //!< The item already exists
         no_context,          //!< The expected context for the asynchronous or delayed  operation does not exist.
};

/**
 * The error category for Cool.NG library error codes.
 */
struct dlldecl cool_ng_category : std::error_category
{
  /**
   * Returns the error category name.
   *
   * This method overrides  @c std::error_category::name() method.
   */
  const char* name() const NOEXCEPT_ override;
  /**
   * Returns the error message associated with the error value.
   *
   * This method overrides  @c std::error_category::message() method.
   */
  std::string message(int ev) const override;
  /**
   * Returns the error condition for the given error code.
   *
   * This methods overrides @c std::error_category::default_error_condition() method.
   */
  std::error_condition default_error_condition(int code_) const NOEXCEPT_ override;

};

/**
 * Creates a @c std::error_code from the @ref errc "Cool.NG library error code enumerator".
 */
dlldecl std::error_code make_error_code(errc);
/**
 * Returns an @em no @em error error code.
 */
dlldecl std::error_code no_error();

} } } // namespace

namespace std {

/**
 * Standard template specialization for @ref cool::ng::error::errc "errc" enumeration.
 */
template <>
struct is_error_code_enum<cool::ng::error::errc>  : true_type { };

/**
 * Standard template specialization for @ref cool::ng::error::errc "errc" enumeration.
 */
template <>
struct is_error_condition_enum<cool::ng::error::errc>  : true_type { };

} // namespace

#endif
