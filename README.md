<a href="http://www.boost.org/LICENSE_1_0.txt" target="_blank">![Boost Licence](http://img.shields.io/badge/license-boost-blue.svg)</a>
<a href="https://github.com/boost-ext/mp/releases" target="_blank">![Version](https://badge.fury.io/gh/boost-ext%2Fmp.svg)</a>
<a href="https://godbolt.org/z/nhrqWWr3W">![build](https://img.shields.io/badge/build-blue.svg)</a>
<a href="https://godbolt.org/z/zKK981h8j">![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)</a>

---------------------------------------

## MP - ~~Template~~ Meta-Programming (Back to the future)

> https://en.wikipedia.org/wiki/Metaprogramming

### Features

- Single header (https://raw.githubusercontent.com/boost-ext/mp/main/mp)
    - Easy integration (see [FAQ](#faq))
- Minimal [API](#api)
- Minimal learning curve (reuses stl, ranges or any third-party library/algorithms operating on stl like containers)
- Easy debugging (meta-functions can simply be executed and debugged at run-time - see [examples](#examples))
- Self verfication upon include (<.05s overhead - can be disabled by `NTEST`)
- Compiles cleanly with ([`-Wall -Wextra -Werror -pedantic -pedantic-errors | /W4 /WX`](https://godbolt.org/z/on3qb6n9M))
- Fast compilation-times (see [benchmarks](#benchmarks))
- Support for reflection (see [examples](#examples) / https://github.com/boost-ext/reflect)

### Requirements

- C++17* ([clang++13+, g++12+, msvc-19.34+](https://godbolt.org/z/5vjba7ezY))

> \* Limited compiler support and functionality (see [API](#api))

---

### Overview

```cpp
static_assert(mp::meta<int> != mp::meta<void>);
static_assert(typeid(mp::meta<int>) == typeid(mp::meta<void>));
```

```cpp
constexpr mp::meta_t meta = mp::meta<int>;
mp::type_of<meta> i{}; // same as int i{};
mp::type_of<mp::meta<bool>> b = true; // same as bool b = true;
```

> https://godbolt.org/z/xYb3hsoYq

---

### Examples

> [C++17] Hello world

```cpp
template<auto N, class... Ts>
using at_c = mp::type_of<std::array{mp::meta<Ts>...}[N]>;

static_assert(std::is_same_v<int, at_c<0, int, bool, float>>);
static_assert(std::is_same_v<bool, at_c<1, int, bool, float>>);
static_assert(std::is_same_v<float, at_c<2, int, bool, float>>);
```

> https://godbolt.org/z/44q1jEsea

---

> [C++17] Algorithms

```cpp
template<class... Ts>
auto drop_1_reverse = [] {
  std::array v{mp::meta<Ts>...}; // or mp::array{mp::meta<Ts>...};
  std::array<mp::meta_t, sizeof...(Ts)-1> r{};
  // fuse operations for faster compilation times (can use STL)
  for (auto i = 1u; i < v.size(); ++i) { r[i-1] = v[v.size()-i]; }
  return r;
};

static_assert(std::is_same_v<std::variant<int, double>,
              decltype(mp::apply<std::variant>(drop_1_reverse<float, double, int>))>);
```

> https://godbolt.org/z/fhahKPqK1

---

> [C++17/C++20] Reduce

```cpp
template<class... Ts>
constexpr auto reduce() {
  constexpr auto v = drop_1_reverse<Ts...>();
  mp::meta_t result = mp::meta<void>;
  mp::for_each<v>([&]<auto m> {
    if (using type = mp::type_of<m>; std::is_same_v<double, type>) {
      result = mp::meta<type*>;
    }
  });
  return result;
}

static_assert(std::is_same_v<double*,
              mp::type_of<reduce<float, double, int>()>>);
```

> https://godbolt.org/z/KEEPoxKK5

---

> [C++20] Ranges

```cpp
template<class... Ts>
constexpr mp::vector drop_1_reverse =
    std::array{mp::meta<Ts>...}
  | std::views::drop(1)
  | std::views::reverse
  ;

static_assert(std::is_same_v<std::variant<int, double>,
              mp::apply_t<std::variant, drop_1_reverse<float, double, int>>>);
```

> https://godbolt.org/z/93GTe7xGx

---

> [C++20] Reflection (https://github.com/boost-ext/reflect)

```cpp
struct foo {
  int a;
  bool b;
  float c;
};

foo f{.a = 42, .b = true, .c = 3.2f};

constexpr mp::vector v =
    reflect::reflect(f)
  | std::views::filter([](auto meta) { return meta->name() == "b" ; })
  | std::views::reverse
  ;

mp::for_each<v>([&]<auto meta>{
  std::cout << reflect::type_name<mp::type_of<meta>>() << '\n'; // prints bool
  std::cout << mp::value_of<meta>(f) << '\n';                   // prints true
});

auto&& t = mp::apply<std::tuple, v>(f);

std::apply([](auto... args) {
  ((std::cout << args << '\n'), ...); // prints true
}, t);
```

> https://godbolt.org/z/s7ce6bh5d

---

> [C++20] Simple Domain Specific Language (DSL)

```cpp
int main() {
  using namespace dsl;
  constexpr auto v =
        type_list<int, const double, float>
      | filter([]<class T> { return not std::is_const_v<T>; })
      | transform([]<class T>() -> T* { })
      | reverse
      | take<1>
      ;

  static_assert(type_list<float*> == v);
}
```

> https://godbolt.org/z/r936cErdd

---

> [C++17] Run-time testing/debugging

```cpp
template<class... Ts>
constexpr auto reverse() {
  std::array v{mp::meta<Ts>...};
  std::array<mp::meta_t, sizeof...(Ts)> r;
  for (auto i = 0u; i < v.size(); ++i) { r[i] = v[v.size()-i-1]; }
  return r;
}

int main() {
  static_assert(
    std::array{mp::meta<float>, mp::meta<double>, mp::meta<int>}
    ==
    reverse<int, double, float>()
  );

  assert((
    std::array{mp::meta<float>, mp::meta<double>, mp::meta<int>}
    ==
    reverse<int, double, float>()
  ));
}
```

> https://godbolt.org/z/h9K3bnaea

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
 * Minimal (not standard compliant) array
 * implementation optimized for fast compilation-times with meta_t
 *
 * @code
 * array v{meta<void>, meta<int>};
 * assert(2 == v.size());
 * assert(meta<void> == v[0]);
 * assert(meta<int> == v[1]);
 * @endcode
 */
template<class T, size_t Size>
struct array;
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
```

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

> https://boost-ext.github.io/mp

---

### FAQ

- What does it mean that `mp` tests itself upon include?

    > `mp` runs all tests (via static_asserts) upon include. If the include compiled it means all tests are passing and the library works correctly on given compiler, enviornment.

- Can I disable running tests at compile-time for faster compilation times?

    > When `NTEST` is defined static_asserts tests won't be executed upon inclusion.
    Note: Use with caution as disabling tests means that there are no gurantees upon inclusion that given compiler/env combination works as expected.

- How to integrate with CMake/CPM?

    ```
    CPMAddPackage(
      Name mp
      GITHUB_REPOSITORY boost-ext/mp
      GIT_TAG v2.0.0
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
