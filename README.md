<a href="http://www.boost.org/LICENSE_1_0.txt" target="_blank">![Boost Licence](http://img.shields.io/badge/license-boost-blue.svg)</a>
<a href="https://github.com/boost-ext/mp/releases" target="_blank">![Version](https://badge.fury.io/gh/boost-ext%2Fmp.svg)</a>
<a href="https://godbolt.org/z/K1Kr1Gz89">![build](https://img.shields.io/badge/build-blue.svg)</a>
<a href="https://godbolt.org/z/Y756ax4cq">![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)</a>

---------------------------------------

## MP - ~~Template~~ Meta-Programming

> https://en.wikipedia.org/wiki/Metaprogramming

### Features

- Single header (https://raw.githubusercontent.com/boost-ext/mp/main/mp)
    - Easy integration (see [FAQ](#faq))
- Minimal [API](#api)
- Minimal learning curve (reuses STL, ranges or any third-party algorithms for stl.container)
- Easy debugging (meta-functions can be simply run at run-time)
- Verifies itself upon include (aka run all tests via static_asserts but it can be disabled - see [FAQ](#faq))
- Compiles cleanly with ([`-fno-exceptions -fno-rtti -Wall -Wextra -Werror -pedantic -pedantic-errors | /W4 /WX`](https://godbolt.org/z/M747ocGfx))
- Fast compilation-times (see [compilation-times](#comp))

### Requirements

- C++17* ([clang++15+, g++11+, msvc-19.34+](https://godbolt.org/z/6Tzjx79vK))

---

### Hello world (https://godbolt.org/z/TEEcdonfs)

```cpp
template<class... Ts>
auto hello_world = [] {
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
  std::is_same_v<
    mp::apply_t<std::variant, hello_world<int, double, const float, short>>,
    std::variant<const float*>
  >);
```

--- C++20 with ranges

```cpp
template<class... Ts>
auto hello_world = [] {
  return std::vector{mp::meta<Ts>...}
       | std::views::filter([](auto m) { return is_const(m); })
       | std::views::transform([](auto m) { return add_pointer(m); })
       ;
};
```

```cpp
static_assert(
  std::is_same_v<
    mp::apply_t<std::variant, hello_world<int, double, const float, short>>,
    std::variant<const float*>
  >);
```
---

### Examples

- 

---

### API

> Configuration

```cpp
#define MP 1'0'0 // Current library version (SemVer)
```
---

### FAQ

- How `mp` works under the hood?

    > ....

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
    > [boost.mp11](https://github.com/boostorg/mp11)

---

**Disclaimer** `mp` is not an official Boost library.
