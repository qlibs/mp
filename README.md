<a href="http://www.boost.org/LICENSE_1_0.txt" target="_blank">![Boost Licence](http://img.shields.io/badge/license-boost-blue.svg)</a>
<a href="https://github.com/boost-ext/mp/releases" target="_blank">![Version](https://badge.fury.io/gh/boost-ext%2Fmp.svg)</a>
<a href="https://godbolt.org/z/on3qb6n9M">![build](https://img.shields.io/badge/build-blue.svg)</a>
<a href="https://godbolt.org/z/TY4EevTEM">![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)</a>

---------------------------------------

## MP - ~~Template~~ Meta-Programming

> https://en.wikipedia.org/wiki/Metaprogramming

### Features

- Single header (https://raw.githubusercontent.com/boost-ext/mp/main/mp)
    - Easy integration (see [FAQ](#faq))
- Minimal [API](#api)
- Minimal learning curve (reuses stl, ranges or any third-party library/algorithms operating on stl like containers)
- Easy debugging (meta-functions can be simply executed and debugged at run-time)
- Verifies itself upon include (aka run all tests via static_asserts (less than .1s overhead but it can be disabled - see [FAQ](#faq))
- Compiles cleanly with ([`-Wall -Wextra -Werror -pedantic -pedantic-errors | /W4 /WX`](https://godbolt.org/z/on3qb6n9M))
- Fast compilation-times (see [benchmarks](#benchmarks))
- Support for reflection and/or tuples (see [examples](#examples))

### Requirements

- C++17* ([clang++15+, g++11+, msvc-19.34+](https://godbolt.org/z/3nraKEoqr))

    - No STL headers required

> \* Limited compiler support and functionality (see [API](#api))

---

### Hello world (https://godbolt.org/z/TY4EevTEM)

> #### C++20

```cpp
template<class... Ts>
auto hello_world_20 = [] {
  std::vector v{mp::meta<Ts>...};
  std::vector<mp::meta_t> r;
  for (auto e : v) {
    if (is_const(e)) {
      r.push_back(add_pointer(e));
    }
  }
  return r;
};
```

```cpp
static_assert(
  typeid(mp::apply_t<std::variant,
    hello_world_20<int, double, const float, short>>),
  ==
  typeid(std::variant<const float*>)
);
```

> #### C++20 (ranges)

```cpp
template<class... Ts>
auto hello_world_20_ranges = [] {
  return std::vector{mp::meta<Ts>...}
       | std::views::filter([](auto m) { return is_const(m); })
       | std::views::transform([](auto m) { return add_pointer(m); })
       ;
};
```

```cpp
static_assert(
  typeid(mp::apply_t<std::variant,
    hello_world_20_ranges<int, double, const float, short>>),
  ==
  typeid(std::variant<const float*>)
);
```

> #### C++17 (stl)

```cpp
template<class... Ts>
auto hello_world_17_stl = [] {
  mp::vector v{mp::meta<Ts>...};
  v.erase(std::remove_if(v.begin(), v.end(),
          [](auto m) { return not is_const(m); }), v.end());
  mp::vector<mp::meta_t, sizeof...(Ts)> r;
  std::transform(v.begin(), v.end(), std::back_inserter(r),
          [](auto m) { return add_pointer(m); });
  return r;
};
```

```cpp
static_assert(
  typeid(mp::apply<std::variant>(
    hello_world_17_stl<int, double, const float, short>)),
  ==
  typeid(std::variant<const float*>)
);
```

> #### C++20 (`-DMP_MINIMAL` - optimized for compilation times)

```cpp
template<class... Ts>
auto hello_world_20_min = [] {
  constexpr mp::vector v{mp::meta<Ts>...};
  mp::vector<mp::meta_t, sizeof...(Ts)> r;
  mp::for_each<v>([&]<auto m> {
    if constexpr (using type = mp::type_of<m>; std::is_const_v<type>) {
      r.push_back(mp::meta<type*>);
    }
  });
  return r;
};
```

```cpp
static_assert(
  typeid(mp::apply_t<std::variant,
    hello_world_20_min<int, double, const float, short>>),
  ==
  typeid(std::variant<const float*>)
);
```

---

### Examples

-

---

### API

```cpp
/**
 * Meta type object representation
 */
using meta_t = /* unspecified */;
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
template<class T> inline constexpr meta_t meta = /* unspecified */;
```

```cpp
/**
 * Returns underlying type from meta type
 *
 * @code
 * static_assert(typeid(type_of<meta<void>>)
                 ==
                 typeid(void));
 * @endcode
 */
template<meta_t meta> using type_of = /* unspecified */;
```

```cpp
/**
 * Minimal (not standard compliant) inplace/static vector
 * implementation optimized for fast compilation-times with meta_t
 *
 * @code
 * vector v{meta<void>, meta<int>};
 * assert(2 == v.size());
 * assert(meta<void> == v[0]);
 * assert(meta<int>  == v[1]);
 * @endcode
 */
template<class T, size_t Size>
struct vector {
  using value_type = T;
  constexpr vector() = default;
  template<class... Ts> constexpr explicit vector(Ts&&...);
  template<class TRange> requires requires(TRange range) { range.begin(); range.end(); }
  constexpr vector(TRange range);
  constexpr void push_back(const T&);
  constexpr void emplace_back(T&&);
  [[nodiscard]] constexpr auto begin() const;
  [[nodiscard]] constexpr auto begin();
  [[nodiscard]] constexpr auto end() const;
  [[nodiscard]] constexpr auto end();
  [[nodiscard]] constexpr auto operator[](size_t);
  [[nodiscard]] constexpr auto& operator[](size_t);
  [[nodiscard]] constexpr auto size() const;
  [[nodiscard]] constexpr auto capacity() const;
  template<class TIt> constexpr auto erase(TIt, TIt);
  constexpr void clear();
};
```

```cpp
/**
 * Applies invocable `[] { return vector<meta_t>{...}; }` to
 *                   `T<type_of<meta_t>...>`
 *
 * @code
 * static_assert(typeid(variant<int>)
 *               ==
 *               typeid(apply<variant>([] { return vector{meta<int>}; })));
 * @endcode
 */
template<template<class...> class T, class Expr>
[[nodiscard]] constexpr auto apply(Expr expr);
```

```cpp
/**
 * Applies invocable `[] { return vector<meta_t>{...}; }` to
 *                   `[]<type_of<meta_t>...> { }`
 *
 * @code
 * static_assert(apply([] { return vector{meta<int>}; },
 *  []<class T> { static_assert(typeid(T) == typeid(int)); }));
 * @endcode
 */
#if defined(__cpp_nontype_template_args)
template<class Expr, class Fn>
[[nodiscard]]constexpr auto apply(Expr expr, Fn fn);
#endif
```

```cpp
/**
 * Applies `meta_t` and calls `fn.template operator()<meta_t>()`
 *
 * @code
 * apply(meta<int>, []<class T> {
 *   static_assert(typeid(T) == typeid(int)); });
 * @endcode
 */
#if !defined(MP_MINIMAL)
template<class R = meta_t, class Fn>
[[nodiscard]] constexpr auto apply(meta_t m, Fn fn);
#endif
```

```cpp
/**
 * Applies `Fn<type_of<meta_t>>`
 *
 * @code
 * static_assert(apply<std::is_const>(meta<const int>));
 * static_assert(mp::meta<int*> == apply<std::add_pointer>(meta<int>));
 * @endcode
 */
#if !defined(MP_MINIMAL)
template<template<class> class Fn>
[[nodiscard]] constexpr auto apply(mp::meta_t meta);
#endif
```

```cpp
/**
 * Alternative to write `decltype(apply<T>(Expr))`
 *
 * @code
 * static_assert(typeid(variant<int>)
 *               ==
 *               typeid(apply_t<variant, [] { return vector{meta<int>}; }>));
 * @endcode
 */
#if defined(__cpp_nontype_template_args)
template<template<class...> class T, auto Expr> using apply_t;
#endif
```

```cpp
/**
 * Iterates over all elements of constexpr continer
 *
 * @code
 * constexpr vector v{meta<int>};
 * for_each<v>([]<meta_t m> {
 *   static_assert(typeid(int) == typeid(type_of<m>));
 * });
 * @endcode
 */
#if (__cpp_generic_lambdas >= 201707L)
template<auto V, class Fn>
constexpr void for_each(Fn fn);
#endif
```

> Configuration

```cpp
#define MP 1'0'0 // Current library version (SemVer)
```

```cpp
#define MP_MINIMAL // If defined it limits the API for the fastest compilation times
```
---

### Benchmarks

---

### FAQ

- How `mp` works under the hood?

    > `mp` is leveraging a lot techniques such as stateful-metaprogramming (for meta types), constexpr evaluation and containers.
      See implementation of `meta` and `apply` for more details.

- What does it mean that `mp` tests itself upon include?

    > `mp` runs all tests (via static_asserts) upon include. If the include compiled it means all tests are passing and the library works correctly on given compiler, enviornment.

- Can I disable running tests at compile-time for faster compilation times?

    > When `DISABLE_STATIC_ASSERT_TESTS` is defined static_asserts tests won't be executed upon inclusion.
    Note: Use with caution as disabling tests means that there are no gurantees upon inclusion that given compiler/env combination works as expected.

- How to integrate with CMake/CPM?

    ```
    CPMAddPackage(
      Name mp
      GITHUB_REPOSITORY boost-ext/mp
      GIT_TAG v1.1.0
    )
    add_library(mp INTERFACE)
    target_include_directories(mp SYSTEM INTERFACE ${mp_SOURCE_DIR})
    add_library(mp::mp ALIAS mp)
    ```

    ```
    target_link_libraries(${PROJECT_NAME} mp::mp);
    ```

- Similar projects?
    > [boost.mp11](https://github.com/boostorg/mp11), [boost.hana](https://github.com/boostorg/hana), [boost.mpl](https://github.com/boostorg/mpl)

---

**Disclaimer** `mp` is not an official Boost library.
