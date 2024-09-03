// <!--
// The MIT License (MIT)
//
// Copyright (c) 2024 Kris Jusiak <kris@jusiak.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#if 0
// -->
[Overview](#Overview) / [Examples](#Examples) / [API](#API) / [FAQ](#FAQ)

## MP: ~~Template~~ Meta-Programming library

[![MIT Licence](http://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/license/mit)
[![Version](https://img.shields.io/github/v/release/qlibs/mp)](https://github.com/qlibs/mp/releases)
[![Build](https://img.shields.io/badge/build-green.svg)](https://godbolt.org/z/6asb6K4EK)
[![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)](https://godbolt.org/z/3TqPx5WEG)

  > https://en.wikipedia.org/wiki/Template_metaprogramming

### Features

- Single header (https://raw.githubusercontent.com/qlibs/mp/main/mp - for integration see [FAQ](#faq))
- Minimal [API](#api) and learning curve (supports STL, ranges, ...)
- Supports debugging (meta-functions can be executed and debugged at run-time - see [examples](#examples))
- Supports reflection (requires https://github.com/qlibs/reflect - see [examples](#examples))
- Verifies itself upon include (can be disabled with `-DNTEST` - see [FAQ](#faq))
- Optimized compilation-times (see [benchmarks](https://qlibs.github.io/mp))

### Requirements

- C++20 ([clang++13+, g++11+, msvc-19.34+](https://en.cppreference.com/w/cpp/compiler_support))

---

### Overview

> API (https://godbolt.org/z/zTdYGvKKW)

```cpp
// mp::meta
static_assert(mp::meta<int> == mp::meta<int>);
static_assert(mp::meta<int> != mp::meta<void>);
static_assert(typeid(mp::meta<int>) == typeid(mp::meta<void>));

// mp::type_of
constexpr mp::info meta = mp::meta<int>;
mp::type_of<meta> i{}; // same as int i{};
mp::type_of<mp::meta<bool>> b = true; // same as bool b = true;

// mp::apply
template<class...> struct type_list{ };
static_assert(std::is_same_v<type_list<int>, mp::apply_t<type_list, std::array{meta}>>);

// mp::invoke
static_assert(not mp::invoke<std::is_const>(meta));
static_assert(std::is_same_v<const int, mp::type_of<mp::invoke<std::add_const>(meta)>>);

int main() {
  // mp::for_each
  constexpr auto v = mp::vector{meta};
  mp::for_each<v>([&]<mp::info meta>{ /* ... */ });
}

// and more (see API)...
```

---

### Examples

> Hello World (https://godbolt.org/z/69jGzqPs1)

```cpp
template<size_t N, class... Ts>
using at_c = mp::type_of<std::array{mp::meta<Ts>...}[N]>;

static_assert(std::is_same_v<int, at_c<0, int, bool, float>>);
static_assert(std::is_same_v<bool, at_c<1, int, bool, float>>);
static_assert(std::is_same_v<float, at_c<2, int, bool, float>>);
```

---

> Algorithms (https://godbolt.org/z/GvzjvdPq8)

```cpp
template<class... Ts>
struct example {
  mp::apply_t<std::variant,
      std::array{mp::meta<Ts>...}
    | std::views::drop(1)
    | std::views::reverse
    | std::views::filter([](auto m) { return mp::invoke<std::is_integral>(m); })
    | std::views::transform([](auto m) { return mp::invoke<std::add_const>(m); })
    | std::views::take(2)
    | std::ranges::to<mp::vector<mp::info>>()
  > v;
};

static_assert(
  typeid(std::variant<const int, const short>)
  ==
  typeid(example<double, void, const short, int>::v)
);
```

---

> Reflection - https://github.com/qlibs/reflect (https://godbolt.org/z/qb37G79Ya)

```cpp
struct foo {
  int a;
  bool b;
  float c;
};

constexpr foo f{.a = 42, .b = true, .c = 3.2f};

constexpr mp::vector<mp::info> v =
    members(f)
  | std::views::filter([&](auto meta) { return member_name(meta, f) != "b"; })
  ;

static_assert(std::tuple{42, 3.2f} == to<std::tuple, v>(f));
```

---

> Run-time testing/debugging (https://godbolt.org/z/vTfGGToa4)

```cpp
constexpr auto reverse(std::ranges::range auto v) {
  std::reverse(v.begin(), v.end());
  return v;
}

int main() {
  static_assert(
    std::array{mp::meta<float>, mp::meta<double>, mp::meta<int>}
    ==
    reverse(std::array{mp::meta<int>, mp::meta<double>, mp::meta<float>})
  );

  assert((
    std::array{mp::meta<float>, mp::meta<double>, mp::meta<int>}
    ==
    reverse(std::array{mp::meta<int>, mp::meta<double>, mp::meta<float>})
  ));
}
```

---

### API

```cpp
/**
 * Meta info type
 */
enum class info : size_t { };
```

```cpp
/**
 * Creates meta type
 *
 * @code
 * static_assert(meta<void> == meta<void>);
 * static_assert(meta<void> != meta<int>);
 * @endcode
 */
template<class T> inline constexpr info meta = /* unspecified */;
```

```cpp
/**
 * Returns underlying type from meta type
 *
 * @code
 * static_assert(typeid(type_of<meta<void>>) == typeid(void));
 * @endcode
 */
template<info meta> using type_of = /* unspecified */;
```

```cpp
/**
 * Applies invocable `[] { return vector<info>{...}; }` to
 *                   `T<type_of<info>...>`
 *
 * @code
 * static_assert(typeid(variant<int>) ==
 *               typeid(apply<variant>([] { return vector{meta<int>}; })));
 * @endcode
 */
template<template<class...> class T>
[[nodiscard]] constexpr auto apply(concepts::invocable auto expr);
```

```cpp
/**
 * Applies range to `T<type_of<info>...>`
 *
 * @code
 * static_assert(typeid(variant<int>) ==
 *               typeid(apply<variant, vector{meta<int>}>));
 * @endcode
 */
template<template<class...> class T, concepts::range auto range>
inline constexpr auto apply_v = decltype(apply<T, [] { return range; }>);
```

```cpp
/**
 * Applies range to `T<type_of<info>...>`
 *
 * @code
 * static_assert(typeid(variant<int>) ==
 *               typeid(apply_t<variant, [] { return vector{meta<int>}; }>));
 * @endcode
 */
template<template<class...> class T, concepts::range auto range>
using apply_t = decltype(apply_v<T, range>);
```

```cpp
/**
 * Invokes function with compile-time info based on meta-info
 *
 * @code
 * static_assert(invoke(
 *   []<info m> { return std::is_const_v<type_of<m>>;
 * }, meta<conts int>));
 * @endcode
 */
[[nodiscard]] constexpr auto invoke(auto&& fn, info meta);
```

```cpp
/**
 * Invokes type_trait with meta-info
 *
 * @code
 * static_assert(not invoke<std::is_const>(meta<int>));
 * static_assert(invoke<std::is_const>(meta<const int>));
 * @endcode
 */
template<template<class...> class T, class... Ts>
[[nodiscard]] constexpr auto invoke(info meta) {
```

```cpp
/**
 * Iterates over all elements of a range
 *
 * @code
 * constexpr vector v{meta<int>};
 * for_each<v>([]<info m> {
 *   static_assert(typeid(int) == typeid(type_of<m>));
 * });
 * @endcode
 */
template<concepts::range auto range>
constexpr auto for_each(auto&& fn);
```

```cpp
/**
 * Asserts that condition is satisfied
 *  - constant evaluation: calls undefined function `failed()`
 *  - run-time evaluation: #ifndef NDEBUG calls `breakpoint()` otherwise empty call
 */
constexpr void expect(auto cond) requires requires { bool(cond); };
```

---

### FAQ

- What does it mean that `mp` tests itself upon include?

    > `mp` runs all tests (via static_asserts) upon include. If the include compiled it means all tests are passing and the library works correctly on given compiler, enviornment.

- How to disable running tests at compile-time?

    > When `-DNTEST` is defined static_asserts tests wont be executed upon include.
    Note: Use with caution as disabling tests means that there are no gurantees upon include that given compiler/env combination works as expected.

- How does it work?

    > [Template-less Metaprogramming](https://krzysztof-jusiak.github.io/talks/cppcon-2024)

    > C++17 / 100 LOC / gcc, clang, msvc / no dependencies - https://godbolt.org/z/efexGo676

- How `mp` compares to Reflection for C++26 (https://wg21.link/P2996)?

    > `mp` meta-programming model is very simpilar to P2996 and its based on type erased info object and meta-functions. `mp` also supports all C++ standard library and since verion 2.0.0+ `mp` type names have been adopted to closer reflect the reflection proposal.

    | mp (C++20) | P2996 (C++26*) |
    | - | - |
    | `meta<T>` | `^T` |
    | `using info = decltype(meta<void>)` | `using info = decltype(^::)` |
    | `type_of<T>` | `typename [: T :]` |
    | `for_each` | `template for` (https://wg21.link/p1306) |
    | `apply_t` | `substitute` |
    | `invoke<Fn>` | `reflect_invoke` |
    | `invoke<Trait>` | `test_trait` |
    | `invoke<T>` | `extract` |

- How to integrate with [CMake.FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html)?

    ```
    include(FetchContent)

    FetchContent_Declare(
      qlibs.mp
      GIT_REPOSITORY https://github.com/qlibs/mp
      GIT_TAG v2.0.10
    )

    FetchContent_MakeAvailable(qlibs.mp)
    ```

    ```
    target_link_libraries(${PROJECT_NAME} PUBLIC qlibs.mp);
    ```

- Similar projects?
    > [boost.mp11](https://github.com/boostorg/mp11), [boost.hana](https://github.com/boostorg/hana), [boost.fusion](https://github.com/boostorg/fusion), [boost.mpl](https://github.com/boostorg/mpl)
<!--
#endif

#pragma once
#ifdef __GNUC__
#pragma GCC system_header
#endif

#ifndef MP_SIZE
#define MP_SIZE 1024u
#endif

namespace mp::inline v2_0_10 {
using size_t = decltype(sizeof(int));

namespace utility {
template<class T, T...> struct integer_sequence { };
template<size_t... Ns> using index_sequence = integer_sequence<size_t, Ns...>;
template<size_t N> using make_index_sequence =
#if defined(__clang__) || defined(_MSC_VER)
  __make_integer_seq<integer_sequence, size_t, N>;
#else
   index_sequence<__integer_pack(N)...>;
#endif
template<class T> auto declval() -> T&&;
} // namespace utility

namespace concepts {
template<class TRange>
concept range = requires(TRange range) { range.begin(); range.end(); };
template<class Fn, class... TArgs>
concept invocable = requires(Fn fn, TArgs... args) { fn(args...); };
} // namespace concepts

/**
 * Minimal (non-conforming) array
 *
 * @code
 * array v{meta<void>, meta<int>};
 * assert(2 == v.size());
 * assert(meta<void> == v[0]);
 * assert(meta<int> == v[1]);
 * @endcode
 */
template<class T, size_t N>
struct array {
  using value_type = T;
  [[nodiscard]] constexpr auto begin() const { return &data[0]; }
  [[nodiscard]] constexpr auto begin() { return &data[0]; }
  [[nodiscard]] constexpr auto end() const { return &data[0] + N; }
  [[nodiscard]] constexpr auto end() { return &data[0] + N; }
  [[nodiscard]] constexpr auto operator[](size_t I) const { return data[I]; }
  [[nodiscard]] constexpr auto& operator[](size_t I) { return data[I]; }
  [[nodiscard]] constexpr auto size() const { return N; }
  [[nodiscard]] constexpr auto operator==(const array& other) const -> bool {
    for (auto i = 0u; i < N; ++i) { if (data[i] != other.data[i]) { return false; } }
    return true;
  }
  [[nodiscard]] constexpr auto operator!=(const array& other) const -> bool {
    return not operator==(other);
  }
  T data[N];
};
template<class T, class... Ts> array(T, Ts...) -> array<T, 1u + sizeof...(Ts)>;

/**
 * Minimal (non-conforming) vector
 *
 * @code
 * vector v{meta<void>, meta<int>};
 * assert(2 == v.size());
 * assert(meta<void> == v[0]);
 * assert(meta<int> == v[1]);
 * @endcode
 */
template<class T>
struct vector {
  using value_type = T;
  constexpr vector() = default;
  constexpr vector(size_t size) : size_{size} { }
  constexpr explicit vector(const auto&... ts) requires (requires { T(ts); } and ...) : data{ts...}, size_{sizeof...(ts)} { }
  constexpr vector(concepts::range auto range) { for (const auto& t : range) { data[size_++] = t; } }
  constexpr void push_back(const T& t) { data[size_++] = t; }
  constexpr void emplace_back(T&& t) { data[size_++] = static_cast<T&&>(t); }
  [[nodiscard]] constexpr auto begin() const { return &data[0]; }
  [[nodiscard]] constexpr auto begin() { return &data[0]; }
  [[nodiscard]] constexpr auto end() const { return &data[0] + size_; }
  [[nodiscard]] constexpr auto end() { return &data[0] + size_; }
  [[nodiscard]] constexpr auto operator[](size_t i) const { return data[i]; }
  [[nodiscard]] constexpr auto& operator[](size_t i) { return data[i]; }
  [[nodiscard]] constexpr auto size() const { return size_; }
  [[nodiscard]] constexpr auto resize(size_t size) { size_ = size; }
  [[nodiscard]] constexpr auto capacity() const { return MP_SIZE; }
  [[nodiscard]] constexpr auto operator==(const vector<T>& other) const -> bool {
    if (size_ != other.size_) { return false; }
    for (auto i = 0u; i < size_; ++i) { if (data[i] != other.data[i]) { return false; } }
    return true;
  }
  template<size_t size> [[nodiscard]] constexpr auto operator!=(const vector<T>& other) const -> bool {
    return not operator==(other);
  }
  constexpr void clear() { size_ = {}; }

  union { T data[MP_SIZE]{}; }; // __cpp_trivial_union
  size_t size_{};
};
template<class T, class... Ts> vector(T, Ts...) -> vector<T>;

/**
 * Meta info type
 */
enum class info : size_t { };

namespace detail {
template<info> struct info { constexpr auto friend get(info); };
template<class T> struct meta {
  using value_type = T;
  template<size_t left = 0u, size_t right = MP_SIZE - 1u>
  static constexpr auto gen() -> size_t {
    if constexpr (left >= right) {
      return left + requires { get(info<mp::info{left}>{}); };
    } else if constexpr (constexpr auto mid = left + (right - left) / 2u;
      requires { get(info<mp::info{mid}>{}); }) {
      return gen<mid + 1u, right>();
    } else {
      return gen<left, mid - 1u>();
    }
  }
  static constexpr auto id = mp::info{gen()};
  constexpr auto friend get(info<id>) { return meta{}; }
};
void failed();
} // namespace detail

/**
 * Creates meta type
 *
 * @code
 * static_assert(meta<void> == meta<void>);
 * static_assert(meta<void> != meta<int>);
 * @endcode
 */
template<class T>
inline constexpr info meta = detail::meta<T>::id;

/**
 * Returns underlying type from meta type
 *
 * @code
 * static_assert(typeid(type_of<meta<void>>) == typeid(void));
 * @endcode
 */
template<info meta>
using type_of = typename decltype(get(detail::info<meta>{}))::value_type;

/**
 * Applies expression expr to `T<type_of<info>...>`
 *
 * @code
 * static_assert(typeid(variant<int>) == typeid(apply<variant>([] { return vector{meta<int>}; })));
 * @endcode
 */
template<template<class...> class T>
[[nodiscard]] constexpr auto apply(concepts::invocable auto expr) {
  constexpr concepts::range auto range = expr();
  return [range]<size_t... Ns>(utility::index_sequence<Ns...>) {
    return T<type_of<range[Ns]>...>{};
  }(utility::make_index_sequence<range.size()>{});
}

/**
 * Applies range to `T<type_of<info>...>`
 *
 * @code
 * static_assert(typeid(variant<int>) == typeid(apply<variant, vector{meta<int>}>));
 * @endcode
 */
template<template<class...> class T, concepts::range auto range>
inline constexpr auto apply_v = []<size_t... Ns>(utility::index_sequence<Ns...>) {
  return T<type_of<range[Ns]>...>{};
}(utility::make_index_sequence<range.size()>{});

/**
 * Applies range to `T<type_of<info>...>`
 *
 * @code
 * static_assert(typeid(variant<int>) == typeid(apply_t<variant, [] { return vector{meta<int>}; }>));
 * @endcode
 */
template<template<class...> class T, concepts::range auto range>
using apply_t = decltype(
  []<size_t... Ns>(utility::index_sequence<Ns...>) {
    return utility::declval<T<type_of<range[Ns]>...>>();
  }(utility::make_index_sequence<range.size()>{})
);

/**
 * Invokes function with compile-time info based on run-time info
 *
 * @code
 * static_assert(invoke(
 *   []<info m> { return std::is_const_v<type_of<m>>;
 * }, meta<conts int>));
 * @endcode
 */
template<class Fn, class T = decltype([]{})>
[[nodiscard]] constexpr auto invoke(Fn&& fn, info meta) {
  constexpr auto dispatch = [&]<size_t... Ns>(utility::index_sequence<Ns...>) {
    return array{
      []<info N> {
        return +[](Fn fn) {
          if constexpr (requires { fn.template operator()<N>(); }) {
            return fn.template operator()<N>();
          }
        };
      }.template operator()<info{Ns}>()...
    };
  }(utility::make_index_sequence<size_t(mp::meta<T>)>{});
  return dispatch[size_t(meta)](fn);
}

/**
 * Invokes type_trait with meta-info
 *
 * @code
 * static_assert(not invoke<std::is_const>(meta<int>));
 * static_assert(invoke<std::is_const>(meta<const int>));
 * @endcode
 */
template<template<class...> class T, class... Ts, auto = []{}>
[[nodiscard]] constexpr auto invoke(info meta) {
  return invoke([]<info meta> {
    using type = type_of<meta>;
    if constexpr (requires { T<Ts..., type>::value; }) {
      return T<Ts..., type>::value;
    } else {
      return mp::meta<typename T<Ts..., type>::type>;
    }
  }, meta);
}

/**
 * Iterates over all elements of over range
 *
 * @code
 * constexpr vector v{meta<int>};
 * for_each<v>([]<auto m> {
 *   static_assert(typeid(int) == typeid(type_of<m>));
 * });
 * @endcode
 */
template<concepts::range auto range>
constexpr auto for_each(auto&& fn) {
  [fn]<size_t... Ns>(utility::index_sequence<Ns...>) {
    (fn.template operator()<range[Ns]>(), ...);
  }(utility::make_index_sequence<range.size()>{});
}

/**
 * Asserts that condition is satisfied
 *  - constant evaluation: calls undefined function `failed()`
 *  - run-time evaluation: #ifndef NDEBUG calls `breakpoint()` otherwise empty call
 */
constexpr void expect(auto cond) requires requires { bool(cond); } {
  if (__builtin_is_constant_evaluated()) {
    if (not cond) {
      detail::failed();
    }
  } else {
    #ifndef NDEBUG
      if (not cond) {
        #if defined (_MSC_VER)
          __debugbreak();
        #else
          __builtin_trap();
        #endif
      }
    #endif
  }
}
} // namespace mp

#ifndef NTEST
namespace mp::test {
  template<class...> struct type_list { };
  template<class...> inline constexpr auto is_same_v = false;
  template<class T> inline constexpr auto is_same_v<T, T> = true;
  template<class T> struct is_const { static constexpr auto value = false; };
  template<class T> struct is_const<const T> { static constexpr auto value = true; };
  template<class T> struct add_const { using type = const T; };
} // namespace mp::test

static_assert(([] {
  using namespace mp::test;
  using mp::expect;

  // mp::test
  {
    static_assert(not is_same_v<int, void>);
    static_assert(not is_same_v<void, int>);
    static_assert(not is_same_v<void*, int>);
    static_assert(not is_same_v<int, const int>);
    static_assert(is_same_v<void, void>);
    static_assert(is_same_v<int, int>);
  }

  // mp::utility::index_sequence
  {
    static_assert([](mp::utility::integer_sequence<mp::size_t>){ return true; }(mp::utility::make_index_sequence<0>{}));
    static_assert([](mp::utility::integer_sequence<mp::size_t, 0>){ return true; }(mp::utility::make_index_sequence<1>{}));
    static_assert([](mp::utility::integer_sequence<mp::size_t, 0, 1>){ return true; }(mp::utility::make_index_sequence<2>{}));
    static_assert([](mp::utility::index_sequence<>){ return true; }(mp::utility::make_index_sequence<0>{}));
    static_assert([](mp::utility::index_sequence<0>){ return true; }(mp::utility::make_index_sequence<1>{}));
    static_assert([](mp::utility::index_sequence<0, 1>){ return true; }(mp::utility::make_index_sequence<2>{}));
  }

  // mp::array
  {
    {
      mp::array<mp::size_t, 1u> a{};
      expect(1u == a.size());
    }

    {
      mp::array a{1, 2};
      expect(2u == a.size());
      expect(1 == a[0]);
      expect(2 == a[1]);
    }

    {
      expect(mp::array{1, 2} == mp::array{1, 2});
      expect(mp::array{1, 2} != mp::array{1, 3});
    }

    {
      mp::array a{1};
      a[0] = 2;
      expect(2 == a[0]);
    }

    {
      mp::array a{1, 2, 3};
      expect(3u == a.size());
      expect(a.begin() != a.end());
      expect(a.size() == mp::size_t(a.end() - a.begin()));
      expect(a.end() == a.begin() + a.size());
    }
  }

  // mp::vector
  {
    {
      mp::vector<mp::size_t> v{};
      expect(0u == v.size());
      expect(1u <= v.capacity());
    }

    {
      mp::vector<int> v{1, 2, 3};
      expect(3u == v.size());
      expect(3u <= v.capacity());
    }

    {
      mp::vector<int> v{1, 2, 3};
      expect(3u == v.size());
      v.resize(2);
      expect(2u == v.size());
    }

    {
      constexpr mp::vector<int> v1{1};
      static_assert(1u == v1.size());
      static_assert(1 == v1[0]);

      constexpr mp::vector<int> v2{v1};
      static_assert(1u == v2.size());
      static_assert(1 == v2[0]);
    }

    {
      mp::vector<int> v1{1, 2};
      expect(2u == v1.size());
      expect(4u <= v1.capacity());
      expect(1 == v1[0]);
      expect(2 == v1[1]);

      mp::vector<int> v2{v1};
      expect(2u == v2.size());
      expect(2u <= v2.capacity());
      expect(1 == v2[0]);
      expect(2 == v2[1]);
    }

    {
      mp::vector<int> v;
      expect(0u == v.size());
      expect(2u <= v.capacity());

      v.push_back(1);
      expect(1u == v.size());
      expect(1 == v[0]);

      v.push_back(2);
      expect(2u == v.size());
      expect(2 == v[1]);
    }

    {
      mp::vector<int> v{42};
      expect(42 == v[0]);
      v[0]++;
      expect(43 == v[0]);
      v[0] = 0;
      expect(0 == v[0]);
    }

    {
      mp::vector<int> v{1, 2};
      expect(2u == v.size());
      expect(2u <= v.capacity());

      v.clear();
      expect(0u == v.size());
      expect(2u <= v.capacity());
    }

    {
      mp::vector<int> v{};
      expect(0u == v.size() && 0u == mp::size_t(v.end() - v.begin()));
      expect(v.begin() == v.end());

      v.emplace_back(1);
      expect(1u == v.size() && 1 == mp::size_t(v.end() - v.begin()));
      expect(v.begin() != v.end());

      v.emplace_back(2);
      expect(2u == v.size() && 2u == mp::size_t(v.end() - v.begin()));
      expect(v.begin() != v.end());
    }

    {
      expect(mp::vector<int>{1} == mp::vector<int>{1});
      expect(mp::vector<int>{1} != mp::vector<int>{2});
    }
  }

  // mp::detail
  {
    static_assert(mp::detail::meta<int>::id == mp::detail::meta<int>::id);
    static_assert(mp::detail::meta<int>::id != mp::detail::meta<void>::id);
    static_assert(mp::detail::meta<void>::id == mp::detail::meta<void>::id);
  }

  // mp::meta
  {
    static_assert(mp::meta<int> == mp::meta<int>);
    static_assert(mp::meta<int> != mp::meta<double>);
    static_assert(mp::meta<double> == mp::meta<double>);
    static_assert(mp::meta<int> == mp::meta<int>);
    static_assert(mp::meta<void> == mp::meta<void>);
  }

  // mp::type_of
  {
    enum class e : int { };
    static_assert(is_same_v<void, mp::type_of<mp::meta<void>>>);
    static_assert(is_same_v<const int, mp::type_of<mp::meta<const int>>>);
    static_assert(is_same_v<const void*, mp::type_of<mp::meta<const void*>>>);
    static_assert(is_same_v<e, mp::type_of<mp::meta<e>>>);

    constexpr mp::vector v{mp::meta<int>, mp::meta<e>};
    static_assert(is_same_v<int, mp::type_of<v[0]>>);
    static_assert(is_same_v<e, mp::type_of<v[1]>>);
  }

  // mp::apply
  {
    static_assert([](type_list<>){ return true; }(mp::apply<type_list>([] { return mp::vector<mp::info>{}; })));
    static_assert([](type_list<int>){ return true; }(mp::apply<type_list>([] { return mp::vector{mp::meta<int>}; })));
    static_assert([](type_list<void, void>){ return true; }(mp::apply<type_list>([] { return mp::vector{mp::meta<void>, mp::meta<void>}; })));
  }

  // mp::apply_t
  {
    static_assert(is_same_v<
      type_list<>,
      mp::apply_t<type_list, mp::vector<mp::info>{}>
    >);

    static_assert(is_same_v<
      type_list<int, const short>,
      mp::apply_t<type_list, mp::array{mp::meta<int>, mp::meta<const short>}>
    >);
  }

  // mp::invoke
  {
    static_assert(mp::invoke([]<mp::info meta> { return meta == mp::meta<int>; }, mp::meta<int>));
    static_assert(not mp::invoke([]<mp::info meta> { return meta == mp::meta<void>; }, mp::meta<int>));

    static_assert(not mp::invoke<is_const>(mp::meta<int>));
    static_assert(mp::invoke<is_const>(mp::meta<const int>));

    static_assert(is_same_v<const int, mp::type_of<mp::invoke<add_const>(mp::meta<int>)>>);
    static_assert(is_same_v<const int, mp::type_of<mp::invoke<add_const>(mp::meta<const int>)>>);
  }

  // mp::for_each
  {
    {
      constexpr mp::vector<mp::info> m{};
      mp::vector<mp::info> v{};
      mp::for_each<m>([&]<auto meta> { v.push_back(meta); });
      expect(0u == v.size());
    }

    {
      mp::for_each<mp::vector{mp::meta<int>}>([&]<auto meta> {
        expect(meta == mp::meta<int>);
      });
    }

    {
      constexpr mp::vector m{mp::meta<int>, mp::meta<void>};
      mp::vector<mp::info> v{};
      mp::for_each<m>([&]<mp::info meta> { v.push_back(meta); });
      expect(2u == v.size());
      expect(m[0] == v[0]);
      expect(m[1] == v[1]);
    }
  }
}(), true));
#endif // NTEST
