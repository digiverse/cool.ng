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

#if !defined(cool_ng_41352bf7_f2d7_4732_8200_be1475dc84b2)
#define      cool_ng_41352bf7_f2d7_4732_8200_be1475dc84b2

#include <cstddef>
#include <type_traits>
#include <tuple>

namespace cool { namespace ng {  namespace traits {

template <typename T> struct naked_type
{
  using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};

// -------
// functional<Callable> determines the following information about
// the Callable type:
//   - result_type the return type of the callable, including void
//   - aritiy::value number of parameters
//   - ::arg<n>::type type of n-th paramter; will static_assert if n >= arity::value
//
// Note: it works for function pointers, functors, member function pointers,
//       member pointers. lambdas and std::function objects. It does not work
//       for std::bind<> produced types as they have multiple operator()
//       overloads. If you need this anyway, assign it to std::function.
template <typename T> struct arg_info
{
  using arg_type = T;
  using naked_type = typename naked_type<T>::type;
  using is_lref = std::is_lvalue_reference<T>;
  using is_const = std::is_const<T>;
};
template <typename T> struct arg_info<const T>
{
  using arg_type = T;
  using naked_type = typename naked_type<T>::type;
  using is_lref = std::true_type;
  using is_const = std::true_type;
};
template <typename T> struct arg_info<const T&>
{
  using arg_type = T;
  using naked_type = typename naked_type<T>::type;
  using is_lref = std::true_type;
  using is_const = std::true_type;
};


template<typename F> struct functional;

template<class R, class... Args>
struct functional<R(Args...)>
{
  using result_type = R;

  using arity = std::integral_constant<std::size_t, sizeof...(Args) >;
//  using arguments = std::tuple<Args...>;
  using arguments = std::tuple<arg_info<Args>...>;
  template <std::size_t N>
  struct arg
  {
    static_assert(N < arity::value, "error: invalid parameter index.");
    using info = typename std::tuple_element<N, arguments>::type;
    using type = typename std::tuple_element<N, arguments>::type::arg_type;
  };
};

// function pointer
template<typename R, typename... Args>
struct functional<R(*)(Args...)> : public functional<R(Args...)>
{};

// member function pointer
template<typename C, typename R, typename... Args>
struct functional<R(C::*)(Args...)> : public functional<R(C*,Args...)>
{};

// const member function pointer
template<typename C, typename R, typename... Args>
struct functional<R(C::*)(Args...) const> : public functional<R(const C*,Args...)>
{};

// member object pointer
template<typename C, typename R>
struct functional<R(C::*)> : public functional<R(C*)>
{};


// functor
template<typename F>
struct functional
{
private:
  using call_type = functional<decltype(&F::operator())>;
public:
  using result_type = typename call_type::result_type;

  using arity = std::integral_constant<std::size_t, call_type::arity::value - 1>;

  template <std::size_t N>
  struct arg
  {
    static_assert(N < arity::value, "error: invalid parameter index.");
    using type = typename call_type::template arg<N+1>::type;
    using info = typename call_type::template arg<N+1>::info;
  };
};

template<typename F>
struct functional<F&> : public functional<F>
{};

template<typename F>
struct functional<F&&> : public functional<F>
{};

// -------
// arg_type<n, CallableT>::type
//     returns type of the n-th parameter if exist, void otherwise

template <std::size_t arg_num, typename CallableT> class arg_type
{
 private:
  template <bool is_valid, std::size_t arg_num_, typename CallableT_>
  struct helper
  {
    using arg_type = typename functional<CallableT_>::template arg<arg_num_>::type;
  };

  template <std::size_t arg_num_, typename CallableT_>
  struct helper<false, arg_num_, CallableT_>
  {
    using arg_type = void;
  };

 public:
  using type = typename helper<
        arg_num < functional<CallableT>::arity::value
      , arg_num
      , CallableT
    >::arg_type;
};

} } } // namespace

#endif
