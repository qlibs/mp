<a href="http://www.boost.org/LICENSE_1_0.txt" target="_blank">![Boost Licence](http://img.shields.io/badge/license-boost-blue.svg)</a>
<a href="https://github.com/boost-ext/mp/releases" target="_blank">![Version](https://badge.fury.io/gh/boost-ext%2Fmp.svg)</a>
<a href="https://godbolt.org/z/on3qb6n9M">![build](https://img.shields.io/badge/build-blue.svg)</a>
<a href="https://godbolt.org/z/W1fvsTc8o">![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)</a>

---------------------------------------

## MP - ~~Template~~ Meta-Programming

> https://en.wikipedia.org/wiki/Metaprogramming

### Features

- Single header (https://raw.githubusercontent.com/boost-ext/mp/main/mp)
    - Easy integration (see [FAQ](#faq))
- Minimal [API](#api)
- Minimal learning curve (reuses stl, ranges or any third-party library/algorithms operating on stl like containers)
- Easy debugging (meta-functions can be simply executed and debugged at run-time - see [examples](#examples))
- Verifies itself upon include (aka run all tests via static_asserts / can be disabled by `DISABLE_STATIC_ASSERT_TESTS`)
- Compiles cleanly with ([`-Wall -Wextra -Werror -pedantic -pedantic-errors | /W4 /WX`](https://godbolt.org/z/on3qb6n9M))
- Fast compilation-times (see [benchmarks](#benchmarks))
- Support for reflection and/or tuples (see [examples](#examples))

### Requirements

- C++17* ([clang++15+, g++11+, msvc-19.34+](https://godbolt.org/z/3nraKEoqr))

    - No STL headers required

> \* Limited compiler support and functionality (see [API](#api))

---

### Hello world (https://godbolt.org/z/W1fvsTc8o)

```cpp
```

---

### Examples

- Meta types - https://godbolt.org/z/Mnfjcj18P
- Run-time testing/debugging - https://godbolt.org/z/1oczvPE61
- Manipulating `std::tuple` - https://godbolt.org/z/7Eoz34z5q
- Integration with reflection (https://github.com/boost-ext/reflect) -

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
 * Returns value of meta type
 *
 * @code
 * static_assert(42 = value_of_v<mp::meta<std::integral_constant<int, 42>>>);
 * @endcode
 */
template<meta_t meta>
[[nodiscard]] constexpr auto value_of_v;
```

```cpp
/**
 * Returns value of meta type underlying object
 */
template<meta_t meta, class T>
[[nodiscard]] constexpr decltype(auto) value_of(T&& t);
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
template<class T, size_t Size> struct vector;
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
 * Applies expression expr to `R<type_of<meta_t>...>`
 *
 * @code
 * static_assert(typeid(variant<int>)
 *               ==
 *               typeid(apply<variant>([] { return vector{meta<int>}; })));
 * @endcode
 */
template<template<class...> class R, class Expr>
[[nodiscard]] constexpr auto apply(Expr expr);
```

```cpp
/**
 * Applies vector V to `R<type_of<meta_t>...>`
 *
 * @code
 * static_assert(typeid(variant<int>)
 *               ==
 *               typeid(apply<variant, vector{meta<int>}>));
 * @endcode
 */
#if (__cpp_nontype_template_args >= 201911L)
template<template<class...> class R, auto V>
inline constexpr auto apply_v = /* unspecified */;

```cpp
/**
 * Applies vector V with object t to `R{value_of<V>(t)...}
 */
#if (__cpp_nontype_template_args >= 201911L)
template<template<class...> class R, auto V, class T>
[[nodiscard]] constexpr auto apply(T&& t);
#endif
```

```cpp
/**
 * Alternative to write `decltype(apply_v<T, Expr>))`
 *
 * @code
 * static_assert(typeid(variant<int>)
 *               ==
 *               typeid(apply_t<variant, [] { return vector{meta<int>}; }>));
 * @endcode
 */
#if (__cpp_nontype_template_args >= 201911L)
template<template<class...> class T, auto V> using apply_t = /* unspecified */;
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
