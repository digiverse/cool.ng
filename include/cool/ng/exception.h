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

#if !defined(cool_ng_f36a3cb0_dda1_4ce1_b25a_913f5951523a)
#define      cool_ng_f36a3cb0_dda1_4ce1_b25a_913f5951523a


#include <string>
#include <memory>
#include <array>
#include <vector>
#include <exception>
#include <stdexcept>
#include <system_error>
#include <cstdint>

#include "cool/ng/error.h"
#include "cool/ng/impl/platform.h"

namespace cool { namespace ng {

namespace exception {

/**
 * The compile time constant defining the maximal number ofcall stack traces captured by @ref backtrace.
 */
const std::size_t default_bt_depth = 40;
/**
 * The platform independent type used to store call stack addresses.
 */
using stack_address = void*;
/**
 * The platform independent null pointer value for call stack traces.
 */
const stack_address null_address = nullptr;

/**
 * Helper class that captures stack backtrace at its construction. It is
 * used by an @ref base "exception base" class to capture the stack
 * backtrace.
 */
class dlldecl backtrace
{
 public:
  /**
   * Constructs an instance of backtrace.
   */
  backtrace(bool capture_) NOEXCEPT_;
  /**
   * Returns the number of stack backtraces captured by this instance.
   */
  inline std::size_t size() const NOEXCEPT_ { return m_count; };
  /**
   * Returns the stack backtrace address at the specified index.
   *
   * @throw out_of_range thrown if index @c idx_ is out of range.
   */
  stack_address operator [](std::size_t idx_) const;
  /**
   * Return the program symbols that correspond to the captured stack backtrace
   * addresses.
   */
  std::vector<std::string> symbols() const NOEXCEPT_;

 private:
  std::array<stack_address, default_bt_depth> m_traces;
  int32_t                                     m_count;
};

/**
 * Helper class that captures the system error code. If listed as a base class before any
 * other base classes it can fetch the last system error code value before the failure in construction
 * of other bases may corrupt it.
 */
class dlldecl system_error_code
{
 public:
  /**
   * Ctor that stores the error code passed to it.
   * @param errno_ the error code to store
   */
  system_error_code(int errno_) NOEXCEPT_  { m_errno = errno_; }
  /**
   * Type conversion operator.
   * @return The stored error code.
   */
  EXPLICIT_ operator int() const NOEXCEPT_ { return m_errno;   }

 private:
  int m_errno;
};

/**
 * @ingroup excgen
 * Common base class for all exceptions in @ref errh.
 *
 * This class is a base class for all exceptions thrown by the Cool.NG library code. The class will, if so
 * instructed, and if possible, capture the call stack of the execution path that led to the creation
 * of this instance. Since in C++ the exception objects are typically created just before they are thrown,
 * this call stack represents the exection path that led to the exceptional situation which required throwing an
 * exception. The base class itself derives from the @c std::runtime_exception so that all
 * exceptions thrown by the Cool.NG library can be caught as usually, by catching @c std::exception
 * based objects.
 *
 * This base class is resuable and can be used by the application's code as a base for their application
 * level exceptions.
 */
class dlldecl base : public std::runtime_error
{
 public:
  /**
   * Construct a new exception object with optional user message that optionally
   * captures the call stack backtrace.
   *
   * @param msg_ optional user message, empty by default
   * @param backtrace_ optional flag to set to @c false if the call stack backtrace is not
   *                   wanted; default value is @c true.
   * @note The user message, as provided, can be obtained via @c what() method of the
   * @c std::exception base class. The @ref message() method will return more
   * elaborate description, including the exception @ref name() and the @ref error::library_category::message()
   * "standard error text", if available.
   */
  base(const std::string& msg_ = "", bool backtrace_ = true) NOEXCEPT_;
  /**
   * Return user message, if any, or an empty string if none.
   */
  virtual std::string message() const;
  /**
   * Return @ref backtrace "captured call stack backtrace".
   */
  const backtrace& stack_backtrace() const;
  /**
   * Return the error code associated with this instance of the exception object.
   *
   * @return the default implementation returns @ref error::errc::not_set
   *
   * @note The derived classes should override this method to return more specific error codes,  if
   * so desired by design.
   */
  virtual std::error_code code() const NOEXCEPT_;
  /**
   * Return the name of this exception object type.
   *
   * @return the default implementation returns name @em base.
   * @note The derived classes should override this method to return more specific names.
   */
  virtual const char* name() const NOEXCEPT_;

 private:
  std::shared_ptr<backtrace> m_backtrace;
};

/* ---------------------------------------------------------------------------
 * -----
 * ----- System errors
 * -----
 * ------------------------------------------------------------------------ */

/**
 * @ingroup excgen
 * Exception class that captures the system error code at the moment of
 * its construction and present them in the C++ standard manner. This and the
 * derived exception classes contain an error code of the @c std::system_error
 * error category.
 */
class system_error : public system_error_code, public base
{
 public:
  /**
   * Construct a new exception object that captures the system error code.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  dlldecl system_error(const std::string& msg_ = "", bool backtrace_ = true);
  /**
   * Construct a new exception object with already captured system error code.
   *
   * @param code_ system error code
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  dlldecl system_error(int code_, const std::string& msg_ = "", bool backtrace_ = true);
  dlldecl std::string message() const override;
  std::error_code code() const NOEXCEPT_ override
  {
    return m_errc;
  }
  const char* name() const NOEXCEPT_ override
  {
    return "system_error";
  }

 private:
  std::error_code m_errc;
};

/**
 * @ingroup excgen
 * Exception class that captures the system error code of the networking subsystem
 * at the moment of its construction and present them in the C++ standard manner.
 * This and the derived exception classes contain an error code of the
 * @c std::system_error error category.
 *
 * @note This class is necessary because the network socket library on the
 * Microsoft Windows uses own error codes. Thus instead of calling @c GetLastError()
 * this class calls @c WSAGetLastError() to capture the error code. On other platforms
 * this type is an alias for @ref system_error.
 * @note The multi-platform application code should use this type as a base type to report all failures
 * related to the socket operations.
 * @note The error handling code should not catch this type of exception. It should
 * always catch @ref system_error instead.
 */
#if defined(WINDOWS_TARGET)
class network_error : public system_error
{
 public:
  /**
   * Construct a new exception object that captures the error code of the networking
   * subsystem.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  dlldecl network_error(const std::string& msg_ = "", bool backtrace_ = true);
  /**
   * Construct a new exception object with already captured networkign subsystem error code.
   *
   * @param code_ networking subsystem error code
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  dlldecl network_error(int code_, const std::string& msg_ = "", bool backtrace_ = true);
  const char* name() const NOEXCEPT_ override
  {
    return "network_error";
  }
};
#else
using network_error = system_error;
#endif

/**
 * @ingroup excool
 * Exception class denoting condition where the weak pointer to the @ref async::runner "runner"
 * object can no longer lock on this object.
 *
 * @note The @ref code() method will return @ref error::errc::no_runner "no_runner".
 */
class runner_not_available final : public base
{
 public:
  /**
   * Construct a new exception object.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  runner_not_available(const std::string& msg_ = "", bool backtrace_ = true)
    : base(msg_, backtrace_)
  { /* noop */ }
  std::error_code code() const NOEXCEPT_ override
  {
    return make_error_code(cool::ng::error::errc::no_runner);
  }
  const char* name() const NOEXCEPT_ override
  {
    return "runner_not_available";
  }
};

/**
 * @ingroup excool
 * Exception class denoting condition where the pointer to the @ref async::runner "runner"
 * object cannot be cast into the pointer to the desired data type.
 *
 * @note The @ref code() method will return @ref error::errc::bad_runner_cast "bad_runner_cast".
 * @note While the code path to throw this exception exists, it should never be thrown. Compiler's type
 *   safety mechnisms should prevent the use of this code path.
 */
class bad_runner_cast final : public base
{
 public:
  /**
   * Construct a new exception object.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  bad_runner_cast(const std::string& msg_ = "", bool backtrace_ = true)
    : base(msg_, backtrace_)
  { /* noop */ }
  std::error_code code() const NOEXCEPT_ override
  {
    return make_error_code(cool::ng::error::errc::bad_runner_cast);
  }
  const char* name() const NOEXCEPT_ override
  {
    return "bad_runner_cast";
  }
};

/**
 * @ingroup excool
 * Exception class denoting condition where the value cannot be converted into
 * the value of the desired data type.
 *
 * @note The @ref code() method will return @ref error::errc::bad_conversion "bad_conversion".
 */
class bad_conversion final : public base
{
 public:
  /**
   * Construct a new exception object.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  bad_conversion(const std::string& msg_ = "", bool backtrace_ = true)
    : base(msg_, backtrace_)
  { /* noop */ }
  std::error_code code() const NOEXCEPT_ override
  {
    return make_error_code(cool::ng::error::errc::bad_conversion);
  }
  const char* name() const NOEXCEPT_ override
  {
    return "bad_conversion";
  }
};

/**
 * @ingroup excool
 * Exception class denoting concurrency problem condition.
 *
 * This exception is typically thrown by objects that aren't thread-safe when they detect possible concurrent
 * use which prevents them to successfully complete the task at hand.
 *
 * @note The @ref code() method will return @ref error::errc::concurrency_problem "concurrency_problem".
 */
class concurrency_problem final : public base
{
 public:
  /**
   * Construct a new exception object.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  concurrency_problem(const std::string& msg_ = "", bool backtrace_ = true)
    : base(msg_, backtrace_)
  { /* noop */ }
  std::error_code code() const NOEXCEPT_ override
  {
    return make_error_code(cool::ng::error::errc::concurrency_problem);
  }
  const char* name() const NOEXCEPT_ override
  {
    return "concurrency_problem";
  }
};

/**
 * @ingroup excool
 * Exception class denoting that the expected context for the asynchronous or delayed  operation does not exist.
 *
 * @note The @ref code() method will return @ref error::errc::no_context "no_context".
 */
class no_context final : public base
{
 public:
  /**
   * Construct a new exception object.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  no_context(const std::string& msg_ = "", bool backtrace_ = true)
    : base(msg_, backtrace_)
  { /* noop */ }
  std::error_code code() const NOEXCEPT_ override
  {
    return make_error_code(cool::ng::error::errc::no_context);
  }
  const char* name() const NOEXCEPT_ override
  {
    return "no_context";
  }
};


/**
 * @ingroup excool
 * Exception class denoting problem with the index of the array exceeding the array size, or the value was
 * detected to be out of the valid value range for the operation.
 *
 * @note The @ref code() method will return @ref error::errc::out_of_range "out_of_range".
 */
class out_of_range final : public base
{
 public:
  /**
   * Construct a new exception object.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  out_of_range(const std::string& msg_ = "", bool backtrace_ = true)
    : base(msg_, backtrace_)
  { /* noop */ }
  std::error_code code() const NOEXCEPT_ override
  {
    return make_error_code(cool::ng::error::errc::out_of_range);
  }
  const char* name() const NOEXCEPT_ override
  {
    return "out_of_range";
  }
};

/**
 * @ingroup excool
 * Exception class denoting problem with one or more parameters being set to illegal value.
 *
 * @note The @ref code() method will return @ref error::errc::illegal_argument "illegal_argument".
 */
class illegal_argument final : public base
{
 public:
  /**
   * Construct a new exception object.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  illegal_argument(const std::string& msg_ = "", bool backtrace_ = true)
    : base(msg_, backtrace_)
  { /* noop */ }
  std::error_code code() const NOEXCEPT_ override
  {
    return make_error_code(cool::ng::error::errc::illegal_argument);
  }
  const char* name() const NOEXCEPT_ override
  {
    return "illegal_argument";
  }
};

/**
 * @ingroup excool
 * Exception class denoting problem with the unexpected internal state,  or with the
 * internal state preventing the requested action.
 *
 * @note The @ref code() method will return @ref error::errc::invalid_state "invalid_state".
 */
class invalid_state final : public base
{
 public:
  /**
   * Construct a new exception object.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  invalid_state(const std::string& msg_ = "", bool backtrace_ = true)
    : base(msg_, backtrace_)
  { /* noop */ }
  std::error_code code() const NOEXCEPT_ override
  {
    return make_error_code(cool::ng::error::errc::invalid_state);
  }
  const char* name() const NOEXCEPT_ override
  {
    return "invalid_state";
  }
};

/**
 * @ingroup excool
 * Exception class denoting that the requested resource is already spoken for.
 *
 * @note The @ref code() method will return @ref error::errc::resource_busy "resource_busy".
 */
class resource_busy final : public base
{
 public:
  /**
   * Construct a new exception object.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  resource_busy(const std::string& msg_ = "", bool backtrace_ = true)
    : base(msg_, backtrace_)
  { /* noop */ }
  std::error_code code() const NOEXCEPT_ override
  {
    return make_error_code(cool::ng::error::errc::resource_busy);
  }
  const char* name() const NOEXCEPT_ override
  {
    return "resource_busy";
  }
};

/**
 * @ingroup excool
 * Exception class denoting that the requested operation failed for the unspecified reason.
 *
 * @note The user message should provide more details on the cause of the failure.
 * @note The @ref code() method will return @ref error::errc::operation_failed "operation_failed".
 */
class operation_failed final : public base
{
 public:
  /**
   * Construct a new exception object.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  operation_failed(const std::string& msg_ = "", bool backtrace_ = true)
    : base(msg_, backtrace_)
  { /* noop */ }
  std::error_code code() const NOEXCEPT_ override
  {
    return make_error_code(cool::ng::error::errc::operation_failed);
  }
  const char* name() const NOEXCEPT_ override
  {
    return "operation_failed";
  }
};

/**
 * @ingroup excool
 * Exception class denoting that the object on which to perform the operation was found empty.
 *
 * @note The @ref code() method will return @ref error::errc::empty_object "empty_object".
 */
class empty_object final : public base
{
 public:
  /**
   * Construct a new exception object.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  empty_object(const std::string& msg_ = "", bool backtrace_ = true)
    : base(msg_, backtrace_)
  { /* noop */ }
  std::error_code code() const NOEXCEPT_ override
  {
    return make_error_code(cool::ng::error::errc::empty_object);
  }
  const char* name() const NOEXCEPT_ override
  {
    return "empty_object";
  }
};

/**
 * @ingroup excool
 * Exception class denoting that the item already exists.
 *
 * @note The @ref code() method will return @ref error::errc::already_exists "already_exists".
 */
class already_exists final : public base
{
 public:
  /**
   * Construct a new exception object.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  already_exists(const std::string& msg_ = "", bool backtrace_ = true)
    : base(msg_, backtrace_)
  { /* noop */ }
  std::error_code code() const NOEXCEPT_ override
  {
    return make_error_code(cool::ng::error::errc::already_exists);
  }
  const char* name() const NOEXCEPT_ override
  {
    return "already_exists";
  }
};

/**
 * @ingroup excool
 * Exception class denoting that the requested item was not found.
 *
 * @note The @ref code() method will return @ref error::errc::not_found "not_found".
 */
class not_found final : public base
{
 public:
  /**
   * Construct a new exception object.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  not_found(const std::string& msg_ = "", bool backtrace_ = true)
    : base(msg_, backtrace_)
  { /* noop */ }
  std::error_code code() const NOEXCEPT_ override
  {
    return make_error_code(cool::ng::error::errc::not_found);
  }
  const char* name() const NOEXCEPT_ override
  {
    return "not_found";
  }
};

/**
 * Exception class denoting that the problem occured during parsing of the input.
 */
class parsing_error final : public base
{
 public:
  /**
   * Construct a new exception object.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the stack backtrace is not
   *                   wanted; default value is @c true.
   */
  parsing_error(const std::string& msg_ = "", bool backtrace_ = true)
    : base(msg_, backtrace_)
  { /* noop */ }
  std::error_code code() const NOEXCEPT_ override
  {
    return make_error_code(cool::ng::error::errc::parsing_error);
  }
  const char* name() const NOEXCEPT_ override
  {
    return "parsing_error";
  }
};

/**
 * Return textual presentation of this exception with information presented
 * in human readable way.
 */
dlldecl std::string to_string(const base& ex);

} } } // namespace

#endif
