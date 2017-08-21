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

#if !defined(cool_ng_41352af7_f2d7_4732_8200_be1475dc84b2)
#define      cool_ng_41352af7_f2d7_4732_8200_be1475dc84b2

#include <cstddef>
#include <type_traits>
#include <tuple>

namespace cool { namespace ng {  namespace async {

namespace detail { namespace traits {

struct void_type { };
  
// ---------------------
// Get first type in the parameter pack, assuming there's more than one
template <typename... Args>
class get_first
{
  template <typename T, typename... Ts>
  struct helper
  {
    using type = T;
  };

 public:
  static_assert(sizeof...(Args) > 0, "error: non-empty template parameter pack expected");
  using type = typename std::decay<typename helper<Args...>::type>::type;
};

// ---------------------
// Get last type in the parameter pack, assuming there's more than one
template <typename... Args>
class get_last
{

  using sequence = std::tuple<
    typename std::conditional<
        std::is_same<typename std::decay<Args>::type, void>::value
      , void_type
      , Args>::type...
    >;

 public:
  static_assert(sizeof...(Args) > 0, "error: non-empty template parameter pack expected");
  using type = typename std::conditional<
      std::is_same<
          void_type
        , typename std::tuple_element<std::tuple_size<sequence>::value - 1,  sequence>::type
        >::value
    , void
    , typename std::tuple_element<std::tuple_size<sequence>::value - 1,  sequence>::type
  >::type;
};

// --------
// calculate result type of parallel tasks as
//     std::tuple<result_type1, result_type2, ...>
// Note: for tasks not returning value it has to replace void with placeholder
// void_result

template <typename... TaskTs>
struct get_parallel_result_type
{
  using type = std::tuple<
      typename std::conditional<
          std::is_same<typename std::decay<typename std::decay<TaskTs>::type::result_type>::type, void>::value
        , void_type
        , typename std::remove_reference<TaskTs>::type::result_type>::type...
      >;
};
  
// --------
// result type  of sequential tasks is a result of the last task in the sequence
template <typename... TaskTs>
struct get_sequence_result_type
{
  using type = typename get_last<TaskTs...>::type::result_type;
};


// --------
// all_same::value is true if all types in paramter pack are the same
// type (after std::decay) and false if not

template<typename... Ts>
struct is_same : std::false_type
{ };

template<>
struct is_same<> : std::true_type
{ };

template<typename T>
struct is_same<T> : std::true_type
{ };


template<typename T, typename... Ts>
struct is_same<T, T, Ts...> : is_same<T, Ts...>
{ };

// --------
// all_chained::value is true if for all task types in the parameter pack
// the parameter type of the next task is the same as the result type of the
// preceding task

template <typename T, typename Y, typename... Ts>
struct is_chain
{
  using result = std::integral_constant<bool, is_chain<T, Y>::result::value && is_chain<Y, Ts...>::result::value>;
};

template <typename T, typename Y>
struct is_chain<T, Y>
{
  using result = std::integral_constant<bool, std::is_same<typename std::decay<typename T::result_type>::type, typename std::decay<typename Y::input_type>::type>::value>;
};

// ---------
// Misc utility traits:
// - unbound_type: calculates function signature depending on whether it has input param or not
// - result_reporter: calculates result reported signature depending on whether user lamda returns value or not
template <typename RunT, typename InpT, typename RetT>
struct run_signature
{
  using type = std::function<RetT(const std::shared_ptr<RunT>&, const InpT&)>;
};

template <typename RunT, typename RetT>
struct run_signature<RunT, void, RetT>
{
  using type = std::function<RetT(const std::shared_ptr<RunT>&)>;
};
// ---
template <typename RetT>
struct result_reporter
{
  using type = std::function<void(const RetT&)>;
};
template <>
struct result_reporter<void>
{
  using type = std::function<void()>;
};

} } } } }// namespace

#endif
