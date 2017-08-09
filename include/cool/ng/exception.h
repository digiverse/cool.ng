/* Copyright (c) 2017 Digiverse d.o.o.
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
#include <exception>
#include <stdexcept>


#include "impl/platform.h"

namespace cool { namespace ng {

/**
 * This namespace defines exception objects used by the rest of Cool.NG library.
 * They are designed as a general purpose classes and the applications are
 * free to use them for own purposes. It is recommended that the semantics of
 * their use reflect the intended semantics as described in the associated
 * documentation.
 */

namespace exception {

/**
 * Base class for all exceptions thrown by the Cool.NG library objects.
 */
class base : public std::exception
{
 public:
  dlldecl base(const std::string& msg) : m_what(msg) { /* noop */ }
#if defined(COOL_NG_WINDOWS_TARGET)
  dlldecl virtual const char* what() const override { return m_what.c_str(); }
#else
  dlldecl virtual const char* what() const noexcept override { return m_what.c_str(); }
#endif
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
 * a wrong state to perform it, and similar.
 */
class logic_fault : public base
{
 public:
  dlldecl logic_fault(const std::string& msg) : base(msg) { /* noop */ }
};
/**
 * Denotes a condition in which an object, or a system, has been asked to perform
 * a task that cannot be performed in its current state.
 */
class illegal_state : public logic_fault
{
 public:
  dlldecl illegal_state(const std::string& msg) : logic_fault(msg) { /* noop */ }
};
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
 * Denotes a condition in which an object has, or has been asked to use, a
 * value that is out of its prescribed operational range.
 */
class out_of_range : public logic_fault
{
 public:
  dlldecl out_of_range(const std::string& msg) : logic_fault(msg) { /* noop */ }
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

class runtime_exception : public base
{
 public:
  dlldecl runtime_exception(const std::string& msg) : base(msg) { /* noop */ }
};

class create_failure : public runtime_exception
{
 public:
  dlldecl create_failure(const std::string& msg) : runtime_exception(msg) { /* noop */ }
};

class illegal_argument : public runtime_exception
{
 public:
  dlldecl illegal_argument(const std::string& msg) : runtime_exception(msg) { /* noop */ }
};

class bad_conversion : public runtime_exception
{
 public:
  dlldecl bad_conversion(const std::string& msg) : runtime_exception(msg) { /* noop */ }
};


class unsupported_operation : public runtime_exception
{
 public:
  dlldecl unsupported_operation(const std::string& msg) : runtime_exception(msg) { /* noop */ }
};

class call_failed : public runtime_exception
{
 public:
  dlldecl call_failed(const std::string& msg) : runtime_exception(msg) { /* noop */ }
};

class operation_failed : public runtime_exception
{
 public:
  dlldecl operation_failed(const std::string& msg) : runtime_exception(msg) { /* noop */ }
};

} } } // namespace

#endif
