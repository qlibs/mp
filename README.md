<a href="http://www.boost.org/LICENSE_1_0.txt" target="_blank">![Boost Licence](http://img.shields.io/badge/license-boost-blue.svg)</a>
<a href="https://github.com/boost-ext/mp/releases" target="_blank">![Version](https://badge.fury.io/gh/boost-ext%2Fmp.svg)</a>
<a href="https://godbolt.org/z/qarWdbK79">![build](https://img.shields.io/badge/build-blue.svg)</a>
<a href="https://godbolt.org/z/847qEqWzh">![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)</a>

---------------------------------------

## MP - ~~Template~~ Meta-Programming

> https://en.wikipedia.org/wiki/Metaprogramming

### Features

- Single header (https://raw.githubusercontent.com/boost-ext/mp/main/mp - for integration see [FAQ](#faq))
- Minimal [API](#api) and learning curve (supports STL, ranges, ...)
- Supports debugging (meta-functions can be executed and debugged at run-time - see [examples](#examples))
- Supports reflection (requires https://github.com/boost-ext/reflect - see [examples](#examples))
- Verifies itself upon include (can be disabled with `-DNTEST` - see [FAQ](#faq))
- Optimized compilation-times (see [benchmarks](#benchmarks))

### Requirements

- C++20 ([clang++13+, g++11+, msvc-19.34+ -Wall](https://godbolt.org/z/qarWdbK79))
    - `-Wextra -Werror -pedantic -pedantic-errors | /W4 /WX`

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
static_assert(typeid(type_list<int>) == typeid(mp::apply_t<type_list, std::array{meta}>>);

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

> Algorithms (https://godbolt.org/z/Y3rT58xMq)

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

> Reflection (https://github.com/boost-ext/reflect / https://godbolt.org/z/bPPjY3b3v)

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

> Run-time testing/debugging (https://godbolt.org/z/crbeMfo3r)

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
 * Invokes function with compile-time info based on run-time info
 *
 * @code
 * info i = meta<conts int>; // run-time
 * static_assert(invoke([]<info m> { return std::is_const_v<type_of<m>>; }, i));
 * @endcode
 */
constexpr auto invoke(auto fn, info meta);
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
constexpr auto for_each(auto fn);
```

> Configuration

```cpp
#define MP 2'0'3     // Current library version (SemVer)
#define MP_SIZE 1024 // Max number of meta types (compilation times optimization)
```

---

### Benchmarks

> https://boost-ext.github.io/mp

---

### FAQ

- What does it mean that `mp` tests itself upon include?

    > `mp` runs all tests (via static_asserts) upon include. If the include compiled it means all tests are passing and the library works correctly on given compiler, enviornment.

- How to disable running tests at compile-time?

    > When `-DNTEST` is defined static_asserts tests won't be executed upon include.
    Note: Use with caution as disabling tests means that there are no gurantees upon include that given compiler/env combination works as expected.

- How `mp` compares to Reflection for C++26 (https://wg21.link/P2996)?

    > `mp` meta-programming model is very simpilar to P2996 and it's based on type erased info object and meta-functions. `mp` also supports all C++ standard library and since verion 2.0.0+ `mp` type names have been adopted to closer reflect the reflection proposal.

    | P2996 (C++26*) | mp (C++20) |
    | ----- | -- |
    | `^T`  | `meta<T>` |
    | `typename [: T :]`  | `type_of<T>` |
    | ` template for` (https://wg21.link/p1306) | `for_each` |
    | `substitute`  | `apply` |
    | `value_of<R>(reflect_invoke(^fn, {substitute(^meta, {reflect_value(m)})}))` | `invoke(fn, m)` |

- How to integrate with CMake/CPM?

    ```
    CPMAddPackage(
      Name mp
      GITHUB_REPOSITORY boost-ext/mp
      GIT_TAG v2.0.3
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
