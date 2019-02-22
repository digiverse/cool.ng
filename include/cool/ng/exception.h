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
#include <array>
#include <vector>
#include <exception>
#include <stdexcept>
#include <system_error>
#include <cstdint>

#include "cool/ng/error.h"
#include "cool/ng/impl/platform.h"

namespace cool { namespace ng {

/**
 * This namespace defines exception generic objects that help the applications
 * in building their own exception hierarcy.
 *
 * TODO:
 * used by the rest of Cool.NG library.
 * They are designed as a general purpose classes and the applications are
 * free to use them for own purposes. It is recommended that the semantics of
 * their use reflect the intended semantics as described in the associated
 * documentation.
 */

namespace exception {

const std::size_t default_bt_depth = 40;

using stack_address = void*;
const stack_address null_address = nullptr;

/**
 * Helper class that captures stack backtrace at its construction. It is
 * used by an @ref base "exception base" class to capture the stack
 * backtrace.
 */
class backtrace
{
 public:
  /**
   * Constructs an instance of backtrace.
   */
  dlldecl backtrace(bool capture_) NOEXCEPT_;
  /**
   * Returns the number of stack backtraces captured by this instance.
   */
  inline std::size_t size() const NOEXCEPT_ { return m_count; };
  /**
   * Returns the stack backtrace address at the specified index.
   *
   * @throw out_of_range thrown if index @c idx_ is out of range.
   */
  dlldecl stack_address operator [](std::size_t idx_) const;
  /**
   * Return the program symbols that correspond to the captured stack backtrace
   * addresses.
   */
  dlldecl std::vector<std::string> symbols() const NOEXCEPT_;

 private:
  std::array<stack_address, default_bt_depth> m_traces;
  int32_t                                     m_count;
};

/**
 * Common base class for all exceptions in this module.
 */
class base : public std::runtime_error
{
 public:
  /**
   * Construct a new exception object with optional user message that optionally
   * captures the call stack backtrace.
   *
   * @param msg_ optional user message
   * @param backtrace_ optional flag to set to @c false if the call stack backtrace is not
   *                   wanted; default value is @c true.
   */
  base(const std::string& msg_ = "", bool backtrace_ = true)
      : runtime_error(msg_), m_backtrace(backtrace_)
  { /* noop */ }
  /**
   * Return user message, if any, or an empty string if none.
   */
  dlldecl virtual std::string message() const;
  /**
   * Return @ref backtrace "captured call stack backtrace".
   */
  const backtrace& stack_backtrace() const             { return m_backtrace; }
  /**
   * Return the error code associated with this instance of exception object.
   */
  virtual std::error_code code() const NOEXCEPT_ = 0;
  /**
   * Return the name of this exception object type.
   */
  virtual const char* name() const NOEXCEPT_ = 0;

 private:
  backtrace m_backtrace;
};

/* ---------------------------------------------------------------------------
 * -----
 * ----- System errors
 * -----
 * ------------------------------------------------------------------------ */

class system_error_code
{
 public:
  system_error_code(int errno_) NOEXCEPT_    { m_errno = errno_; }
  dlldecl operator int() const NOEXCEPT_     { return m_errno;   }

 private:
  int m_errno;
};

/**
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
 * Exception class that captures the system error code of the networking subsystem
 * at the moment of its construction and present them in the C++ standard manner.
 * This and the derived exception classes contain an error code of the
 * @c std::system_error error category.
 *
 * @note This class is necessary because the network socket library on the
 * Microsoft Windows uses own error codes. Thus instead of calling @c GetLastError()
 * this class calls @c WSAGetLastError() to capture the error code. On other operating
 * system this type is an alias for @ref system_error.
 *
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
    return "socket_error";
  }
};
#else
using network_error = system_error;
#endif

/**
 * Exception class denoting condition where the weak pointer to the runner
 * object can no longer lock on this object.
 */
class runner_not_available : public base
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
 * Exception class denoting condition where the pointer to the runner
 * object cannot be cast into the pointer to the desired data type.
 */
class bad_runner_cast : public base
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
 * Exception class denoting condition where the value cannot be converted into
 * the value of the desired data type.
 */
class bad_conversion : public base
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
 * Exception class denoting concurrency problem condition.
 */
class concurrency_problem : public base
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
 * Exception class denoting that the expected operation context does not exist.
 */
class no_context : public base
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
 * Exception class denoting problem with the value out of the valid value range.
 */
class out_of_range : public base
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
 * Exception class denoting problem with one or more parameters being set to illegal value.
 */
class illegal_argument : public base
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
 * Exception class denoting problem with the unexpected internal state or the
 * internal state preventing the requested action.
 */
class invalid_state : public base
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
 * Exception class denoting problem with the requested resouse is already spoken for.
 */
class resource_busy : public base
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
 * Exception class denoting that the requested operation failed.
 *
 * @note The user message should provide more details on the cause of the failure.
 */
class operation_failed : public base
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
 * Exception class denoting that the object on which to perform the operation was found empty.
 */
class empty_object : public base
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
 * Exception class denoting that the similar item already exists.
 */
class already_exists : public base
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
 * Exception class denoting that the requested item was not found.
 */
class not_found : public base
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
class parsing_error : public base
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
