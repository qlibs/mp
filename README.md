<a href="https://github.com/boost-ext/mp/actions/workflows/linux.yml" target="_blank">![Linux](https://github.com/boost-ext/mp/actions/workflows/linux.yml/badge.svg)</a>
<a href="https://godbolt.org/z/8bcaxxYx4">![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)</a>

# MP - ~~Template~~ Meta-Programming

| [Motivation](#motivation) | [Quick Start](#quick-start) | [Overview](#overview) | [Tutorial](#tutorial) | [Examples](#examples) | [User Guide](#user-guide) | [Benchmarks](#benchmarks) | [FAQ](#faq) |

<details open><summary>C++ <b>single header/single module</b> C++20 Meta-Programming Library</summary>
<p>

<a name="motivation"></a>
<details open><summary>Motivation</summary>

<p>

> Make ~~Template~~ Meta-Programming easier by leveraging run-time approach at compile-time.
  If one knows how to use stl.algorithms/ranges one can consider themself a TMP expert now as well!

```cpp
#include <ranges>

// write once, use multiple times
auto slice = [](auto list, auto begin, auto end) {
  return list
    | std::views::drop(begin)      // use std.ranges
    | std::views::take(end - 1_c); // any library which can operate on containers is supported!
};
```

```cpp
// type_list
static_assert(slice(mp::list<int, double, float, short>(), 1_c, 3_c) ==
                    mp::list<double, float>());
```

```cpp
// variant.type
static_assert(std::is_same_v<
    mp::typeof<slice, std::variant<int, double, float, short>, mp::const_t<1>, mp::const_t<3>>,
                      std::variant<double, float>>
);
```

```cpp
// value_list
static_assert(slice(mp::list<1, 2, 3, 4>(), 1_c, 3_c) ==
                    mp::list<2, 3>());
```

// tuple of values
static_assert(slice(std::tuple{1, 2, 3, 4}, 1_c, 3_c) ==
                    std::tuple{2, 3});

```cpp
#include <cassert>

int main(int argc, const char**) {
  // run-time tuple of values
  assert((slice(std::tuple{1, argc, 3, 4}, 1_c, 3_c) ==
                std::tuple{argc, 3}));
}
```

---

```cpp
auto fn = [](auto&& tuple) {
  return tuple
    | std::views::filter<<([](auto i) -> bool { return i % 2; })
    | std::views::reverse
    | std::views::drop(1_c);
};

static_assert(std::tuple{5, 3, 1} == fn([] { return std::tuple{1, 2, 3, 4, 5, 6, 7}; }));
```

---

```cpp
#include <algorithm>

auto sort_by_size = [](mp::concepts::meta auto types) {
  std::sort(std::begin(types), std::end(types),
    [](auto lhs, auto rhs) { return lhs.size < rhs.size; });
  return types;
};

/**
 * Verify/debug at run-time
 */
int main () {
  "sort by size"_test = [] {
    // given
    const auto m1 = meta{.index = 0, .size = 2};
    const auto m2 = meta{.index = 1, .size = 1};
    const auto m3 = meta{.index = 2, .size = 3};

    // when
    const auto sorted = sort_by_size({m1, m2, m3});

    // then
    expect({m2, m1, m3} == sorted);
  };
}
```

```cpp
struct not_packed {
  char c{};
  int i{};
  std::byte b{};
};

/**
 * Check at compile-time
 */
static_assert(sizeof(not_packed) == 12u);
static_assert(sizeof(to_tuple(not_packed{}) | sort_by_size) == 8u);
```

</p>
</details>

<a name="quick-start"></a>
<details open><summary>Quick Start</summary>
<p>

> Try it out - https://godbolt.org/z/8bcaxxYx4

---

> Locally

```sh
docker build . -t dev # or docker pull krisjusiak/dev
docker run -it -v "$(pwd)":/mp --privileged dev:latest
```

```sh
mkdir build && cd build
CXX={clan}g++ cmake .. -DBOOST_MP_BUILD_TESTS=ON -DBOOST_MP_BUILD_EXAMPLES=ON
```

```sh
cmake --build . -j
```

```sh
ctest --output-on-failure
```

</p>
</details>

<a name="overview"></a>
<details open><summary>Overview</summary>
<p>

- Single C++20 header/module
- Minimal learning curve (reuses STL, ranges or any third-party algorithms for stl.container)
- Easy debugging (meta-functions can be simply run at run-time!)
- Same interface for `types/values/tuples`
- Declarative by design (composable using pipe operator, support for ranges)
- Fast compilation times (see [benchmarks](#benchmarks))

> Requirements ([Dockerfile](Dockerfile))

- C++20 compliant compiler (STL with support for `constexpr std::vector`)
  - clang++15+ [libc++-15+] (✔️)
  - g++12+ [libstdc++-12+] (✔️)

</p>
</details>

<a name="tutorial"></a>
<details open><summary>Tutorial</summary>
<p>

> Firstly include or import `boost.mp`

```cpp
#include <boost/mp.hpp>
```
or
```cpp
import boost.mp;
```

> Okay, let's write a hello world, shall we?

First step is to add our new meta-function.

```cpp
auto identity = [](mp::concepts::meta types) {
  return types;
};
```

`meta` is a meta objects range (like `vector<meta>`) which we can do operations on.
For example, sorting, changing the size, removing elements, etc...

Let's apply our first meta-function.

```cpp
auto magic = mp::list<int, double, float>() | identity;
```

```cpp
static_assert(magic == mp::list<int, double, float>());
```

Yay, we have the first meta-function done. Notice the pipe (|)
operator. By using it multiple meta-functions can be combined together.

For the next example including/importing ranges will be required

```cpp
#include <ranges>
```

Let's implement simple slice for types as an example

```cpp
template<auto list, auto Start, auto End>
auto slice = list
   | std::views::drop(Start)
   | std::views::take(End);
```

> Notice that we've just used std::ranges at compile-time to manipulate a type-list!

```cpp
using mp::operator""_c;
static_assert(slice<mp::list<int, double, float>(), 1_c, 2_c>
           == mp::list<double, float>());
```

`""_c` is an User Defined Literal which represents constant integral value which
is required for simulating passing constexpr parameters which aren't supported
in C++.

Let's add STL too, why not

```
#include <tuple>
#include <algorithm>
```

This time we will sort and reverse a tuple

> Note: All operations are supported for the following entities
  - `mp::type_list`
  - `mp::value_list`
  - `mp::fixed_string`
  - `std::tuple`

Additionally `type_list/value_list/fixed_string` will be deduced automatically
based on parameters when `mp::list<...>()` is used.

Okay, coming back to our sort...

```cpp
template <auto Fn>
auto sort = [](mp::concepts::meta auto types) {
  std::sort(std::begin(types), std::end(types), Fn);
  return types;
};

> Note With ranges that could be `actions::sort(types, Fn)`

auto by_size = [](auto lhs, auto rhs) { return lhs.size < rhs.size; };
```

So far, nothing magical, same code as in run-time!

Let's apply it then

```cpp
using mp::operator|;
auto pack = [](auto t) {
  return mp::to_tuple(t) | sort<by_size>;
}
```

> Note: We used `to_tuple` which converts a struct into a tuple using reflection.
        There is also `to_list` available which produces `type_list`.

As usual, we use pipe (|) to compose functionality.

```cpp
struct not_packed {
  char c{};       // 1b
  int i{};        // 4b
  std::byte b{};  // 1b
};
```

```cpp
static_assert(sizeof(not_packed) == 12u);
static_assert(sizeof(pack(not_packed{})) == 8u);
```

Okay, so far so good, but what about adding or removing from type_list?

Removing is simple as we can just erase elements from the meta types as before.

Dealing with new types/transforms it's a bit different but not difficult either.

```cpp
template <auto List, class... Ts>
auto add = List | mp::list<Ts...>();
```

```cpp
static_assert(add<mp::list<int, double>(), void> ==
                  mp::list<int, double, void>());
```

And what about transform? Let's add pointers to all our type in the list.

```cpp
template <auto List>
auto transform = List
  | mp::trait<std::add_pointer>()
  | mp::trait([]<class T> { return mp::type<T const>; });
```

It's that easy, we just applied a std::add_pointer trait to the list, followed by adding const.

```cpp
static_assert(transform<mp::list<int, double>()> ==
                        mp::list<int* const, double* const>());
```

Okay, so what about the case when we need meta-types and Ts...?


```cpp
auto filter = std::views::filter<<([]<class T> { return requires(T t) { t.value; }; });
```

> Notice handy `requires with lambda` pattern to verify ad-hoc concepts.


```cpp
struct bar {};
struct foo {
  int value;
};
```

```cpp
static_assert(mp::list<foo>() ==
             (mp::list<foo, bar>() | filter));
```

That's it for now, for more let's take a look at more
Examples in the following section and the User-Guide.
</p>
</details>

<p>
</p>
</details>

<a name="examples"></a>
<details open><summary>Examples</summary>
<p>

- <a href="https://github.com/boost-ext/mp/blob/main/example/append.cpp">append</a>
- <a href="https://github.com/boost-ext/mp/blob/main/example/conditional.cpp">conditional</a>
- <a href="https://github.com/boost-ext/mp/blob/main/example/filter.cpp">filter</a>
- <a href="https://github.com/boost-ext/mp/blob/main/example/insert.cpp">insert</a>
- <a href="https://github.com/boost-ext/mp/blob/main/example/nth_pack_element.cpp">nth_pack_element</a>
- <a href="https://github.com/boost-ext/mp/blob/main/example/reverse.cpp">reverse</a>
- <a href="https://github.com/boost-ext/mp/blob/main/example/rotate.cpp">rotate</a>
- <a href="https://github.com/boost-ext/mp/blob/main/example/slice.cpp">slice</a>
- <a href="https://github.com/boost-ext/mp/blob/main/example/to_list.cpp">to_list</a>
- <a href="https://github.com/boost-ext/mp/blob/main/example/to_tuple.cpp">to_tuple</a>
- <a href="https://github.com/boost-ext/mp/blob/main/example/transform.cpp">transform</a>
- <a href="https://github.com/boost-ext/mp/blob/main/example/unique.cpp">unique</a>
- <a href="https://github.com/boost-ext/mp/blob/main/example">...</a>

</p>
</details>

<a name="user-guide"></a>
<details open><summary>User-Guide</summary>
<p>

```cpp
/**
 * Library version for example 1'0'0
 */
#define BOOST_MP_VERSION
```

```cpp
/**
 * Forces using includes even if modules are supported
 */
#define BOOST_MP_DISABLE_MODULE
```

```cpp
/**
 * A meta type representation to be manipulated
 * vector<meta> is passed to pipe lambdas
 */
struct meta {
  std::size_t pos{};
  std::size_t size{};
  //...
}
```

```cpp
/**
 * Returns unique integral (std::size_t) representation of type
 * Should only be used for comparison
 * static_assert(type_id<void> ! = type_id<int>);
 */
template <class T> constexpr auto type_id;
```

```cpp
/**
 * Returns type/value name as string_view
 * static_assert(type_name<void>() == "void");
 * static_assert(type_name<42>() == "42");
 */
template <template auto T> [[nodiscard]] constexpr auto type_name()
```

```cpp
/**
 * A meta concept which verifies meta range
 * static_assert(concepts::meta<vector<meta>>);
 */
concept concepts::meta =
  requires(T t) {
    std::size_t{t.index};
    std::size_t{t.size};
  };
```

```cpp
/**
 * Variable template which represents single type
 * Useful for non-default constructible types
 */
template<class T> unspecified<T> type{};
```

```cpp
/**
 * Variable template which represents trait
 * static_assert(list<int, double> | trait<add::pointer>() == list<int*, double*>);
 * static_assert(list<int, double> | trait([]<class T> { return mp::type<T*>; })
 *            == list<int*, double*>);
 */
template<template auto T> unspecified<T> trait()
```

```cpp
/**
 * List of types
 */
template<class... Ts> struct type_list {
  constexpr auto size = sizeof...(Ts);
  constexpr operator==(type_list) = default;
  constexpr operator[](auto N); // returns N-th type
};
```

```cpp
/**
 * List of values
 */
template<class... Ts> struct value_list;
  constexpr auto size = sizeof...(Vs);
  constexpr operator==(value_list) = default;
  constexpr operator[](auto N); // returns N-th value
};
```

```cpp
/**
 * Compile-time string representation to be used
 * as <"Hello World">
 */
template<std::size_t N> struct fixed_string {
  static constexpr auto size = N;
  [[nodiscard]] constexpr auto operator<=>(const fixed_string&) const = default;
  [[nodiscard]] constexpr explicit(false) operator std::string_view() const;
  std::array<char, N + 1> data{};
};
```

```cpp
/**
 * Deduces correct list based on types
 *  type_list for Ts...
 *  value_list for auto...
 *  fixed_string for if { t.data; t.size; }
 */
template<template auto... Vs> [[nodiscard]] constexpr auto list();
```

```cpp
/**
 * Converts type into a type_list by reflecting fields
 * 0-10 number of reflected fields is supported
 * @tparam T type to be reflected
 */
template<class T> [[nodiscard]] constexpr auto to_list;
```

```cpp
/**
 * Converts type into a std::tuple by reflecting fields
 * 0-10 number of reflected fields is supported
 * @param obj object to be reflected
 */
constexpr auto to_tuple = []<class T>(T&& obj);
```

```cpp
/**
 * Composability pipe operator for types
 * @param fn functor to be applied
 * - [](concepts::meta auto types)
 * - []<class... Ts>
 * - []<class... Ts>(concepts::meta auto types)
 */
template <template <class...> class T, class... Ts>
[[nodiscard]] constexpr auto operator|(T<Ts...>, auto fn) {
```

```cpp
/**
 * Composability pipe operator for values
 * @param fn functor to be applied
 * - [](concepts::meta auto types)
 * - []<auto... Ts>
 * - []<auto... Ts>(concepts::meta auto types)
 */
template <template <auto...> class T, auto... Vs>
[[nodiscard]] constexpr auto operator|(T<Vs...>, auto fn) {
```

```cpp
/**
 * Composability pipe operator for std::tuple
 * @param fn functor to be applied
 * - [](concepts::meta auto types)
 * - [](auto&&... args)
 * - [](concepts::meta auto types, auto&&... args)
 */
template <template <class...> class T, class... Ts>
[[nodiscard]] constexpr auto operator|(std::tuple<Ts...>, auto fn) {
```

```cpp
/**
 * Adapts ranges to meta type space
 * std::views::filter<<([]<class T> { return ...; })
 */
constexpr auto operator<<(auto fn, auto... ts);
```

```cpp
/**
 * Compile time integral value representation (required to mimic constexpr parameters)
 * static_assert(42 == _c(1+41));
 * static_assert(42 == 42_c);
 */
template <auto N> constexpr auto _c;
template <char... Cs> [[nodiscard]] consteval auto operator""_c();
```

</p>
</details>

<a name="benchmarks"></a>
<details open><summary>Benchmarks</summary>
<p>

- <a href="https://boost-ext.github.io/mp/benchmark/sort_unique_reverse">sort_unique_reverse</a>
- <a href="https://boost-ext.github.io/mp/benchmark/transform_filter">transform_filter</a>
- <a href="https://boost-ext.github.io/mp/benchmark/conditional_drop_sum">conditional_drop_sum</a>
- <a href="https://boost-ext.github.io/mp/benchmark/first_or_last_size">first_or_last_size</a>
- <a href="https://boost-ext.github.io/mp/benchmark/first_middle_last">first_middle_last</a>

> To build/run benchmarks

```cpp
cd benchmark
mkdir build && cd build
CXX={clang}g++ cmake ..
make <<benchmark>>
```

</p>
</details>

<a name="faq"></a>
<details open><summary>FAQ</summary>
<p>

> [CONTRIBUTING](.github/CONTRIBUTING.md)

</p>
</details>

</p>
</details>

---

**Disclaimer** `MP` is not an official [Boost](https://www.boost.org/) library.
