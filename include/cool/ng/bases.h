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

/**
 * @file bases.h
 *   Pulbic API for basic utility classes for @ref util.
*/

namespace cool { namespace ng {

namespace util {

namespace detail {

enum class id_policy { duplicate_on_copy, unique_on_copy };
dlldecl unsigned long get_next_id();

} // namespace detail

/**
 * Base class for identified instances.
 *
 * This class can be used as a base class for classes whose instances
 * are to have a program-wide unique numerical identification.
 * @tparam OnCopyPolicy a policy tag that determines whether copies of the objects aquire their
 * own unique identification or preserve the identification of originals
 * @note This class template is an empty shell and is not to be used. See class template specializations
 * for functional class templates.
 *
 * @see @ref identified<detail::id_policy::duplicate_on_copy> "identified" template specialization where
 * copies preserve the identification of the original object
 * @see @ref identified<detail::id_policy::unique_on_copy> "identified" template specialization where
 * copies gain their own original identification different from the identification of the original object
 */
template <detail::id_policy OnCopyPolicy>
class identified
{ };

/**
 * Base class for identified instances.
 *
 * This class can be used as a base class for classes whose instances are to have a program-wide unique
 * numerical identification. This template class specialziation preserves the identification
 * on copy or copy assignment - the copies will have the same identification as the originals. Multiple
 * instances of @ref identified objects may thus exist in the program's memory space.
 *
 * @note All classes that inherit from @ref identified share the same global counter. The numerical
 * identifications of objects of the same class may thus not be sequential.
 *
 * @see @ref identified<detail::id_policy::unique_on_copy> "identified" template specialization where
 * copies gain their own unique identification.
 */
template<>
class identified<detail::id_policy::duplicate_on_copy>
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
   * Constructs a new @ref identified object, moves the numerical identification of the other object to this
   * object, and resets the numerical identification of the original to 0.
   */
  identified(identified&& other_)
  {
    m_number = other_.m_number;
    other_.m_number = 0;
  }
 /**
  * Move assignment operator.
  *
  * Moves the numerical identification of the other object to this object, and resets the numerical
  * identification of the original to 0.
  *
  * @param other_ Object which contents to move
  * @return <tt>*this</tt>
  */
  identified& operator =(identified&& other_)
  {
    m_number = other_.m_number;
    other_.m_number = 0;
    return *this;
  }
  /**
   * Copy constructor
   *
   * Creates a copy of @ref identified object, with the same numerical identification as the original object.
   *
   * @param other_ Object to clone
   */
  identified(const identified& other_) = default;
  /**
   * Copy assignment operator.
   *
   * Copies the contents of the other @ref identified object to this object. This object will thus have the
   * same numerical identification os the original.
   *
   * @param other_ Object to copy
   * @return <tt>*this</tt>
   */
  identified& operator=(const identified& other_) = default;
  /**
   * Create a new @ref identified object.
   *
   * Creates a new @ref identified object with a unique numerical identification.
   */
  identified() : m_number(detail::get_next_id())
  { /* noop */ }

 private:
  unsigned long m_number;
};

/**
 * Base class for identified instances.
 *
 * This class can be used as a base class for classes whose instances are to have a program-wide unique
 * numerical identification. This template class specialziation @em will @em not preserve the identification
 * on copy or copy assignment - the copies will have their own unique identification. It thus guarantees that
 * @em at @em most one object with the same identification will exist in the program's memory at any time.
 *
 * @note All classes that inherit from @ref identified share the same global counter. The numerical
 * identifications of objects of the same class may thus not be sequential.
 *
 * @see @ref identified<detail::id_policy::duplicate_on_copy> "identified" class template specialization where
 * copies preserve the identification of the original object
 */
template<>
class identified<detail::id_policy::unique_on_copy>
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
   * Creates a new @ref identified object and moves the numerical identification of the original object
   * to the new object. The numerical identification of the original is reset to 0.
   *
   * Unlike the copy ctor, this move ctor will preserve the identification of the original object.
   *
   * @param other_ Object which contents to move
   */
  identified(identified&& other_)
  {
    m_number = other_.m_number;
    other_.m_number = 0;
  }
  /**
   * Move assignment operator.
   *
   * The move assignment operator move the numerical identification of the original object to this
   * object and resets the identification of the original to 0.
   *
   * Unlike the copy assignment, this move assignment operator will preserve the identification of the
   * original object.
   *
   * @param other_ Object which contents to move
   * @return <tt>*this</tt>
  */
  identified& operator =(identified&& other_)
  {
    m_number = other_.m_number;
    other_.m_number = 0;
    return *this;
  }
  /**
   * Copy constructor.
   *
   * Creates a new @ref identified object as a copy of the original object but with its own original numerical
   * identification.
   *
   * @param other_ Object to copy
   */
  identified(const identified& other_) : identified()
  { /* noop */ }
  /**
   * Copy assignment operator.
   *
   * Since the only contents of the @ref identified object is its numerical identificatior, this operator does
   * nothing.
   *
   * @param other_ Object to copy
   * @return <tt>*this</tt>
   */
  identified& operator=(const identified& other_)
  {
    return *this;
  }
  /**
   * Create a new @ref identified object.
   *
   * Creates a new @ref identified object with a unique numerical identification.
   */
  identified() : m_number(detail::get_next_id())
  { /* noop */ }

 private:
  unsigned long m_number;
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
 * @see @ref identified<detail::id_policy::unique_on_copy>
 */
class named : public identified<detail::id_policy::unique_on_copy>
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
   */
  dlldecl std::string prefix() const;

 protected:
 /**
  * Create a new @ref named object.
  *
  * Creates a new @ref named object using the specified prefix to construct the
  * name.
  *
  * @param prefix_ The prefix to use to construct the name.
  */
  dlldecl named(const std::string& prefix_);
  /**
   * Create a copy of @ref named object.
   *
   * @param other_ Object to copy.
   * @note New object will have a ref prefix() of <tt>other</tt> an a unique numerical
   * designation.
   */
  named(const named& other_) : named(other_.prefix())
  { /* noop */ }
  /**
   * Copy assignment operator.
   *
   * Assigns the contents of the other object to this object.
   *
   * @param other_ Object to clone.
   * @return <tt>*this</tt>
   */
  dlldecl named& operator =(const named& other_);
  /**
   * Move constructor.
   *
   * Creates a new @ref named object and moves the name from original to the
   * new object. The @ref prefix() of the original oblject is set to string @em moved and its
   * numberic identification to 0..
   *
   * @param other_ other object
   */
  dlldecl named(named&& other_);
  /**
   * Move assignment operator.
   *
   * Assigns the contents of the other object to this object and destroys the contents of the
   * other object.The @ref prefix() of the original oblject is set to string @em moved and its
   * numberic identification to 0..
   *
   * @param other_ other (original) object
   * @return <tt>*this</tt>
   */
  dlldecl named& operator =(named&& other_);

 private:
  std::string m_name;
};

/**
 * Macro definition that makes private ctors accessible to shared_new() construction method.
 * @see self_aware
 * @see shared_new()
 */
#define befriend_shared_new  \
  template <typename _C, typename... _Args> friend std::shared_ptr<_C> cool::ng::util::shared_new(_Args&&...)

/**
 * Creata a new instance of a class @em T with arguments @em Args..., and return a shared pointer to it.
 * @tparam T type  to construct
 * @tparam Args... types of arguments passed to the T's constructor, auto-deduced
 *
 * @see @ref self_aware
 * @see befriend_shared_new macro
 */
template <typename T, typename... Args>
std::shared_ptr<T> shared_new(Args&&... args_)
{
  std::shared_ptr<T> ret(new T(std::forward<Args>(args_)...));
  ret->self(ret);
  return ret;
}
/**
 * Template base class for classes storing a weak pointer to themselves.
 *
 * This base class template, in conjuction with @ref shared_new, attempts to address the main deficiency
 * of <tt>std::enable_shared_from_this</tt> and its <tt>shared_from_this()</tt> method, that a shared
 * pointer to the object must already exist before a call to <tt>shared_from_this()</tt> functions properly.
 * <tt>enable_shared_from_this</tt> also hides its weak pointer, making it inaccessible from the derived
 * classes.
 *
 * The following coding pattern:
 *
 * <pre>
 *   class A {
 *    ...
 *    private:
 *     befriend_shared_new;
 *     A() { .... }
 *   };
 *
 *   auto ptr_A = cool::ng::util::shared_new<A>();
 * </pre>
 *
 * will ensure that a <tt>shared_ptr</tt> will exist immediatelly after the construction of the instance of A, and
 * that any other method of A but the ctor is free to use @ref self() or @ref shared_from_this() at any time. The
 * <tt>befriend_shared_new</tt>, in a combination with the private ctors, also assure that the instance of
 * A can only be constructed though a call to @ref shared_new.
 *
 * @see @ref shared_new()
 * @see befriend_shared_new macro

 */
template <typename T>
class self_aware
{
 public:
   /**
    * Smart strong pointer-to-this-class type
    */
   using ptr      = std::shared_ptr<T>;
   /**
    * Smart weak pointer-to-this-class type
    */
   using weak_ptr = std::weak_ptr<T>;

 public:
 /**
  * Return a weak pointer to itself.
  */
  const weak_ptr& self() const    { return m_self; }

 protected:
 /**
  * Return a shared pointer to itself.
  */
  ptr shared_from_this() const    { return self().lock(); }

 private:
  befriend_shared_new;
  void self(const ptr& s_)        { m_self = s_; }
  void self(const weak_ptr& s_)   { m_self = s_; }

 private:
  weak_ptr m_self;
};


} } } // namespace


#endif
