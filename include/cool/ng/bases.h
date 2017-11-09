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

#if !defined(cool_ng_6c2a783d_18a0_4a18_abe5_908c1f127780)
#define      cool_ng_6c2a783d_18a0_4a18_abe5_908c1f127780

#include <functional>
#include <memory>
#include <string>
#include <atomic>
#include <utility>

#include "impl/platform.h"

namespace cool { namespace ng {

/**
 * This namespace contains several reusable base clases.
 */
namespace util {

/**
 * Base class for identified instances.
 *
 * This class can be used as a base class for classes whose instances
 * are to have a program-wide unique numerical identification.
 *
 * @note All classes that inherit from @ref identified share the same
 * global counter. The numerical identifications of objects of the same
 * class may thus not be sequential.
 *
 * @warning Copy assignment and copy construction will create an exact clone
 * of @ref identified object. Both original and the new copy will have the
 * same numerical identification.
 */
class identified
{
 public:
  /**
   * Return numerical identification of this object.
   *
   * @return Numerical identification.
   */
  unsigned long id() const { return m_number; }

 protected:
 /**
  * Move constructor.
  *
  * The move constructor resets the numerical identification of the original
  * to value 0.
  */
  dlldecl identified(identified&&);
 /**
  * Move assignment operator.
  *
  * The move assignment operator resets the identification of the original
  * to value 0.
  */
  dlldecl identified& operator =(identified&&);
 /**
  * Create a clone of @ref identified object.
  *
  * Creates a clone of @ref identified object with the same numerical
  * identification as the original object.
  *
  * @param original Object to clone
  */
  dlldecl identified(const identified& original) = default;
  /**
   * Create and return a clone of @ref identified object.
   *
   * Creates and returns a clone of @ref identified object with the same numerical
   * identification as the original object.
   *
   * @param original Object to clone
   * @return A clone of the original object
   */
  dlldecl identified& operator=(const identified& original) = default;
  /**
   * Create a new @ref identified object.
   *
   * Creates a new @ref identified object with a unique numerical identification.
   */
  identified() : m_number(++m_counter)
  { /* noop */ }

 private:
  unsigned long m_number;
  static std::atomic<unsigned long> m_counter;
};

/**
 * Base class for named instances.
 *
 * This class can be used as a base class for classes whose instances
 * are to have a program-wide unique textual name. The name has the following
 * format:
 *
 *   <i>prefix</i><tt>-</tt><i>numerical_id</i>
 *
 * where the <i>prefix</i> is a text provided at the @ref named object
 * construction and the <i>numerical_id</i> is a unique numerical identification
 * provided by @ref identified base class.
 *
 * @warning Copy assignment and copy construction will create an exact clone
 * of the @ref named object. Both original and the new copy will have the
 * same name.
 *
 * @see @ref identified
 */
class named : public identified
{
 public:
 /**
  * Return the object's name.
  *
  * @return The name of this object.
  */
  const std::string& name() const { return m_name; }
  /**
   * Return the object's name prefix.
   *
   * @return The prefix specified at the creation of this instance.
   *
   * @note The prefix is correct only when the name is auto-generated and
   *   not explicitly set.
   */
  dlldecl std::string prefix() const;
  /**
   * Set the name for this object
   */
   void name(const std::string& name) { m_name = name; }

 protected:
 /**
  * Create a new @ref named object.
  *
  * Creates a new @ref object using the specified prefix to construct the
  * name.
  *
  * @param The prefix to use to construct the name.
  */
  dlldecl named(const std::string& prefix);
  /**
   * Create a clone of @named object.
   *
   * Creates a @ref named object with the same name as the original.
   *
   * @param original Object to clone.
   */
  dlldecl named(const named& original) = default;
  /**
   * Create and return a clone of @named object.
   *
   * Creates and returns a @ref named object with the same name as the original.
   *
   * @param original Object to clone.
   * @return Clone of original object
   */
  dlldecl named& operator =(const named& original) = default;
  /**
   * Move constructor.
   *
   * Creates a new @ref named object and moves the name from original to the
   * new object. The name of the original object is set to <tt>moved-0</tt>.
   *
   * @param original Object whose name to move.
   */
  dlldecl named(named&& original);
  /**
   * Move assignment operator.
   *
   * Creates and returns a new @ref named object and moves the name from original to the
   * new object. The name of the original object is set to <tt>moved-0</tt>.
   *
   * @param original Object whose name to move.
   * @return A clone of the original object.
   */
  dlldecl named& operator =(named&&);

 private:
  std::string m_name;
  std::string m_prefix;
};

template <typename T>
class self_aware
{
 public:
   using ptr      = std::shared_ptr<T>;
   using weak_ptr = std::weak_ptr<T>;

 public:
  void self(const ptr& s_) { m_self = s_;   }
  void self(const weak_ptr& s_)   { m_self = s_;   }
  const weak_ptr& self() const    { return m_self; }

 private:
  weak_ptr m_self;
};

template <typename T, typename... Args>
std::shared_ptr<T> shared_new(Args&&... args_)
{
  std::shared_ptr<T> ret(new T(std::forward<Args>(args_)...));
  ret->self(ret);
  return ret;
}

#define befriend_shared_new \
    template <typename _C, typename... _Args> friend std::shared_ptr<_C> cool::ng::util::shared_new(_Args&&...)


} } } // namespace

#endif
