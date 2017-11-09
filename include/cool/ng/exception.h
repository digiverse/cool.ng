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

#include "impl/platform.h"

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
 *
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

/**
 * Exception class capturing platform dependent error codes at the moment of
 * its construction and presenting them in the C++ standard manner.
 */
class system_error : public std::exception
                   , public backtrace
{
 public:
  dlldecl system_error(std::size_t depth_ = default_bt_depth);
  dlldecl virtual const char* what() const NOEXCEPT_ override;
};

/**
 * Base class for all exceptions thrown by the Cool.NG library.
 */
class runtime_fault : public std::exception
                    , public backtrace
{
 public:
  runtime_fault(const std::string& msg, std::size_t depth_ = default_bt_depth)
    : backtrace(depth_), m_what(msg)
  { /* noop */ }

  virtual const char* what() const NOEXCEPT_ override
  { return m_what.c_str(); }

 private:
  std::string m_what;
};


/**
 * Base class for all exceptions thrown by the Cool.NG library.
 */
class cool_base : public std::exception
                , public backtrace
{
 public:
  dlldecl cool_base(const std::string& msg, std::size_t depth_ = default_bt_depth);
  virtual const char* what() const NOEXCEPT_ override
  { return m_what.c_str(); }

 private:
  std::string m_what;
};


/**
 * An exception class representing a logical fault within the program
 *
 * This exception class, and exception classes derived from it, represent
 * an error condition that arises from the logical fault withing the
 * program. Examples of such faults are insertion of new elements into
 * containers that are already filled to their capacity, an array index that
 * is out of the array range, entering the method on the object that is in
 * a wrong state to perform it, and similar. In general these kind of
 * errors represent a fault in the code that uses the library.
 */
class cool_logic_fault : public cool_base
{
 public:
  cool_logic_fault(const std::string& msg) : cool_base(msg) { /* noop */ }
};

class runner_not_available : public cool_logic_fault
{
 public:
  runner_not_available()
    : cool_logic_fault("the destination runner not available")
  { /* noop */ }
};

class cool_internal_fault : public cool_base
{
 public:
  cool_internal_fault(const std::string& msg) : cool_base(msg) { /* noop */ }
};

class bad_runner_cast : public cool_internal_fault
{
 public:
  bad_runner_cast()
    : cool_internal_fault("dynamic_pointer_cast to RunnerT type unexpectedly failed")
  { /* noop */ }
};


class no_context : public cool_internal_fault
{
 public:
  no_context()
    : cool_internal_fault("the task context is not available")
  { /* noop */ }
};

class nothing_to_run : public cool_internal_fault
{
 public:
  nothing_to_run()
    : cool_internal_fault("task will not run owing to predicate resolution")
  { /* noop */ }
};







#if 0
/**
 * Denotes a condition in which an object, or a system, has been asked to change
 * state into another state, but such transition cannot be done.
 */
class illegal_transition : public logic_fault
{
 public:
  dlldecl illegal_transition(const std::string& msg) : logic_fault(msg) { /* noop */ }
};
/**
 * Denotes a condition in which an item searched for was not found.
 */
class not_found : public logic_fault
{
 public:
  dlldecl not_found(const std::string& msg) : logic_fault(msg) { /* noop */ }
};
/**
 * A container is already filled to its capacity.
 */
class beyond_capacity : public logic_fault
{
 public:
  dlldecl beyond_capacity(const std::string& msg) : logic_fault(msg) { /* noop */ }
};
/**
 * The specified time interval has expired, or the specified point in time has
 * been put behind.
 */
class timeout : public logic_fault
{
 public:
  dlldecl timeout(const std::string& msg) : logic_fault(msg) { /* noop */ }
};
/**
 * With the cool::basis::aim / cool::basis::vow pair, it is thrown by the aim half
 * of the pair if the vow was destroyed without setting the result of the
 * asynchronous operation.
 */
class broken_vow : public logic_fault
{
 public:
  dlldecl broken_vow(const std::string& msg) : logic_fault(msg) { /* noop */ }
};

class runtime_exception : public cool_base
{
 public:
  dlldecl runtime_exception(const std::string& msg) : cool_base(msg) { /* noop */ }
};

class create_failure : public runtime_exception
{
 public:
  dlldecl create_failure(const std::string& msg) : runtime_exception(msg) { /* noop */ }
};


class call_failed : public runtime_exception
{
 public:
  dlldecl call_failed(const std::string& msg) : runtime_exception(msg) { /* noop */ }
};
#endif


class operation_failed : public runtime_fault
{
 public:
  operation_failed(const std::string& msg) : runtime_fault(msg) { /* noop */ }
};

/**
 * Denotes a condition in which an object has, or has been asked to use, a
 * value that is out of its prescribed operational range.
 */
class out_of_range : public runtime_fault
{
 public:
  out_of_range(const std::string& msg) : runtime_fault(msg) { /* noop */ }
};

class illegal_argument : public runtime_fault
{
 public:
  dlldecl illegal_argument(const std::string& msg) : runtime_fault(msg) { /* noop */ }
};

/**
 * Denotes a condition in which an object, or a system, has been asked to perform
 * a task that cannot be performed in its current state.
 */
class illegal_state : public runtime_fault
{
 public:
  dlldecl illegal_state(const std::string& msg) : runtime_fault(msg) { /* noop */ }
};

class bad_conversion : public runtime_fault
{
 public:
  dlldecl bad_conversion(const std::string& msg) : runtime_fault(msg) { /* noop */ }
};

class unsupported_operation : public runtime_fault
{
 public:
  dlldecl unsupported_operation(const std::string& msg) : runtime_fault(msg) { /* noop */ }
};


} } } // namespace

#endif
