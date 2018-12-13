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
 * Helper class that captures stack backtrace at the construction
 */
class backtrace
{
 public:
  dlldecl const std::vector<stack_address>& traces() const NOEXCEPT_;
  dlldecl std::vector<std::string> symbols() const NOEXCEPT_;

 protected:
  dlldecl backtrace(std::size_t depth_) NOEXCEPT_;

 private:
  std::vector<stack_address> m_traces;
};

class base : public std::exception
           , public backtrace
{
 public:
  base(const std::error_code& ec_, std::size_t depth_ = default_bt_depth) NOEXCEPT_
      : backtrace(depth_), m_errc(ec_)
  { /* noop */ }

  base(const cool::ng::error::errc ec_, std::size_t depth_ = default_bt_depth) NOEXCEPT_
      : backtrace(depth_), m_errc(make_error_code(ec_))
  { /* noop */ }

  const char* what() const NOEXCEPT_ override
  {
    return "Cool.NG library exception. Use message() method for more details.";
  }

  const std::error_code code() const NOEXCEPT_
  {
    return m_errc;
  }
 private:
  const std::error_code m_errc;
};
/**
 * Helper class that captures platform dependent error codes at the moment of
 * its construction and presenting them in the C++ standard manner.
 */
class system_error : public base
{
 public:
  dlldecl system_error(std::size_t depth_ = default_bt_depth) NOEXCEPT_;
};

class runtime_fault : public base
{
 public:
  runtime_fault(const cool::ng::error::errc ec_, std::size_t depth_ = default_bt_depth) NOEXCEPT_
      : base(ec_, depth_)
  { /* noop */ }
};

class logic_fault : public base
{
 public:
  logic_fault(const cool::ng::error::errc ec_, std::size_t depth_ = default_bt_depth) NOEXCEPT_
     : base(ec_, depth_)
  { /* noop */ }
  virtual const char* what() const NOEXCEPT_ override
  {
    return m_msg.c_str();
  }

 private:
  std::string m_msg;
};

class connection_failure : public system_error
{
 public:
  connection_failure(std::size_t depth_ = default_bt_depth) : system_error(depth_)
  { /* noop */ }
};

#if defined(WINDOWS_TARGET)

class socket_failure : public base
{
 public:
  dlldecl socket_failure(std::size_t depth_ = default_bt_depth)  NOEXCEPT_;
};

#else

class socket_failure : public system_error
{
 public:
  socket_failure(std::size_t depth_ = default_bt_depth)  NOEXCEPT_
      : system_error(depth_)
  { /* noop */ }
};

#endif


class threadpool_failure : public system_error
{
 public:
  threadpool_failure(std::size_t depth_ = default_bt_depth)  NOEXCEPT_
      : system_error(depth_)
  { /* noop */ }
};

class cp_failure : public system_error
{
 public:
  cp_failure(std::size_t depth_ = default_bt_depth)  NOEXCEPT_
      : system_error(depth_)
  { /* noop */ }
};

class internal_fault : public base
{
 public:
  internal_fault(const cool::ng::error::errc ec_, std::size_t depth_ = default_bt_depth) NOEXCEPT_
    : base(ec_, depth_)
  { /* noop */ }
};

class bad_runner_cast : public internal_fault
{
 public:
  bad_runner_cast(std::size_t depth_ = default_bt_depth) NOEXCEPT_
      : internal_fault(cool::ng::error::errc::bad_runner_cast, depth_)
  { /* noop */ }
};

class invalid_state : public logic_fault
{
 public:
  invalid_state(std::size_t depth_ = default_bt_depth) NOEXCEPT_
    : logic_fault(cool::ng::error::errc::wrong_state, depth_)
  { /* noop */ }
};

class runner_not_available : public logic_fault
{
 public:
  runner_not_available(std::size_t depth_ = default_bt_depth) NOEXCEPT_
    : logic_fault(cool::ng::error::errc::no_runner, depth_)
  { /* noop */ }
};

class no_context : public internal_fault
{
 public:
  no_context(std::size_t depth_ = default_bt_depth) NOEXCEPT_
      : internal_fault(cool::ng::error::errc::no_task_context, depth_)
  { /* noop */ }
};

class nothing_to_run : public internal_fault
{
 public:
  nothing_to_run(std::size_t depth_ = default_bt_depth) NOEXCEPT_
    : internal_fault(cool::ng::error::errc::not_an_error, depth_)
  { /* noop */ }
};

class out_of_range : public logic_fault
{
 public:
  out_of_range(std::size_t depth_ = default_bt_depth) NOEXCEPT_
      : logic_fault(cool::ng::error::errc::out_of_range, depth_)
  { /* noop */ }
};

class illegal_argument : public logic_fault
{
 public:
  illegal_argument(std::size_t depth_ = default_bt_depth) NOEXCEPT_
      : logic_fault(cool::ng::error::errc::illegal_argument, depth_)
  { /* noop */ }
};

class bad_conversion : public logic_fault
{
 public:
  dlldecl bad_conversion(std::size_t depth_ = default_bt_depth) NOEXCEPT_
      : logic_fault(cool::ng::error::errc::bad_conversion, depth_)
  { /* noop */ }
};

class parsing_error : public logic_fault
{
 public:
  dlldecl parsing_error(std::size_t depth_ = default_bt_depth) NOEXCEPT_
      : logic_fault(cool::ng::error::errc::parsing_error, depth_)
  { /* noop */ }
};

class empty_object : public logic_fault
{
 public:
  dlldecl empty_object(std::size_t depth_ = default_bt_depth) NOEXCEPT_
      : logic_fault(cool::ng::error::errc::empty_object, depth_)
  { /* noop */ }
};

class not_found : public runtime_fault
{
 public:
  not_found(std::size_t depth_ = default_bt_depth) NOEXCEPT_
    : runtime_fault(cool::ng::error::errc::not_found, depth_)
  { /* noop */ }
};

class already_exists : public runtime_fault
{
 public:
  already_exists(std::size_t depth_ = default_bt_depth) NOEXCEPT_
    : runtime_fault(cool::ng::error::errc::already_exists, depth_)
  { /* noop */ }
};

class operation_failed : public runtime_fault
{
 public:
  operation_failed(cool::ng::error::errc ec_, std::size_t depth_ = default_bt_depth) NOEXCEPT_
    : runtime_fault(ec_, depth_)
  { /* noop */ }
};

} } } // namespace

#endif
