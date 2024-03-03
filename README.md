<a href="http://www.boost.org/LICENSE_1_0.txt" target="_blank">![Boost Licence](http://img.shields.io/badge/license-boost-blue.svg)</a>
<a href="https://github.com/boost-ext/mp/releases" target="_blank">![Version](https://badge.fury.io/gh/boost-ext%2Fmp.svg)</a>
<a href="https://godbolt.org/z/53Gej5cax">![build](https://img.shields.io/badge/build-blue.svg)</a>
<a href="https://godbolt.org/z/YEx8c3nG6">![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)</a>

# MP - ~~Template~~ Meta-Programming

> https://en.wikipedia.org/wiki/Metaprogramming

---

| [Motivation](#motivation) | [Examples](#examples) | [API](#api) | [Benchmarks](#benchmarks) | [FAQ](#faq) |

### Features

- Single header (https://raw.githubusercontent.com/boost-ext/mp/main/mp)
- Minimal learning curve (reuses STL, ranges or any third-party algorithms for stl.container)
- Easy debugging (meta-functions can be simply run at run-time)
- Same interface for `types/values/tuples`
- Declarative by design (composable using pipe operator, support for ranges)
- Verifies itself upon include (aka runs all tests via static_asserts)
- Fast compilation times (see [benchmarks](#benchmarks))
- Plays well with reflection

### Requirements

- C++20 ([clang++16+, g++12+, msvc-19.34+](https://godbolt.org/z/6Tzjx79vK))

---

<p>

<a name="motivation"></a>
<details open><summary>Motivation</summary>

<p>

> Simplify Template Meta-Programming by leveraging run-time approach at compile-time with std.ranges.

```cpp
#include <ranges>

auto hello_world = [](auto list, auto add_const, auto has_value){
  return list                          // int, foo, val, bar
   | std::views::drop(1_c)             // foo, val, bar
   | std::views::reverse               // bar, val, foo
   | std::views::take(2_c)             // bar, val
   | std::views::transform(add_const)  // bar const, val const
   | std::views::filter(has_value)     // val const
   ;
};
```

```cpp
auto add_const = []<class T> -> T const {};
auto has_value = []<class T> { return requires(T t) { t.value; }; };
```

---

```cpp
struct bar {};
struct foo { int value; };
struct val { int value; };
```

```cpp
static_assert(mp::list<val const>() ==
  hello_world(mp::list<int, foo, val, bar>(), add_const, has_value)
);
```

```cpp
int main () {
  struct stub_type{
    std::size_t type{};
    bool has_value{};
    bool add_const{};
    constexpr auto operator<=>(const stub_type&) const = default;
  };

  "hello world"_test = [] {
    // given
    std::vector list{stub_type{.type = 0},
                     stub_type{.type = 1, .has_value = true}};

    auto add_const = [](auto& t) { t.add_const = true; return t; };
    auto has_value = [](auto t)  { return t.has_value; };

    // when
    auto out = hello_world(list, add_const, has_value);
    const std::vector<stub_type> result{std::begin(out), std::end(out)};

    // then
    expect(1_u == std::size(result));
    expect(result[0] == stub_type{.type = 1,
                                  .has_value = true,
                                  .add_const = true});
  };
}
```

> https://godbolt.org/z/3zrrcddY7

---

```cpp
// write once, use multiple times
auto slice = [](auto list, auto begin, auto end) {
  return list
    | std::views::drop(begin)
    | std::views::take(end - 1_c)
    ;
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
    mp::typeof<slice, std::variant<int, double, float, short>, 1_c, 3_c>,
                      std::variant<double, float>>
);
```

```cpp
// value_list
static_assert(slice(mp::list<1, 2, 3, 4>(), 1_c, 3_c) ==
                    mp::list<2, 3>());
```

```cpp
// fixed_string
static_assert(slice(mp::list<"foobar">(), 1_c, 3_c) ==
                    mp::list<"oo">());
```

```cpp
// tuple of values
static_assert(slice(std::tuple{1, 2, 3, 4}, 1_c, 3_c) ==
                    std::tuple{2, 3});
```

```cpp
#include <cassert>

int main(int argc, const char**) {
  // run-time tuple of values
  assert((slice(std::tuple{1, argc, 3, 4}, 1_c, 3_c) ==
                std::tuple{argc, 3}));
}
```

> https://godbolt.org/z/cz3vrqY5v

---

```cpp
auto fun_with_tuple = [](auto tuple) {
  return tuple
    | std::views::filter([](auto i) -> bool { return i % 2; })
    | std::views::reverse
    | std::views::drop(1_c)
    ;
};

static_assert(std::tuple{5, 3, 1} == fn([] { return std::tuple{1, 2, 3, 4, 5, 6, 7}; }));
```

---

```cpp
#include <algorithm>

auto sort_by_size = [](std::ranges::range auto types) {
  std::ranges::sort(types, [](auto lhs, auto rhs) { return lhs.size < rhs.size; });
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

---

```cpp
#include <ranges>
#include <algorithm>

auto rotate = [](std::ranges::range auto types) {
    std::ranges::rotate(types, std::begin(types) + 1);
    return types;
};

static_assert((mp::list<int, double, float>() | rotate) ==
               mp::list<double, float, int>());
```

</p>
</details>

<a name="examples"></a>
<details open><summary>Examples</summary>
<p>

- <a href="https://godbolt.org/z/14Wjb7hWe">append</a>
- <a href="https://godbolt.org/z/oWfWMGKsz">conditional</a>
- <a href="https://godbolt.org/z/jEsW46KWz">count_if</a>
- <a href="https://godbolt.org/z/TsnTs4Koe">filter</a>
- <a href="https://godbolt.org/z/sqW5Y3v91">insert</a>
- <a href="https://godbolt.org/z/KWM6s9shx">nth_pack_element</a>
- <a href="https://godbolt.org/z/c8o11qdx4">reverse</a>
- <a href="https://godbolt.org/z/5nfncnja7">rotate</a>
- <a href="https://godbolt.org/z/YWbEbrq9W">slice</a>
- <a href="https://godbolt.org/z/645avsWfd">sort</a>
- <a href="https://godbolt.org/z/jssqYa343">split</a>
- <a href="https://godbolt.org/z/9hxds4ro6">pack</a>
- <a href="https://godbolt.org/z/bP9a1Kd66">transform</a>
- <a href="https://godbolt.org/z/jbx87ccKe">unique</a>
- <a href="https://godbolt.org/z/eKzj1hbW8">state_machine</a>

</p>
</details>

<a name="api"></a>
<details open><summary>API</summary>
<p>

```cpp
/**
 * Current library version
 */
#define MP 1'0'0
```

```cpp
/**
 * If defined static asserts tests arent executed upon include (enabled by default)
 */
#define MP_DISABLE_STATIC_ASSERT_TESTS
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
 * A meta concept which verifies meta list
 * static_assert(concepts::meta<list<>>>);
 * static_assert(concepts::meta<list<int, double>>>);
 * static_assert(concepts::meta<std::tuple<int, double>>>);
 */
concept concepts::meta;
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
 * Composability pipe operator for types
 * @param fn functor to be applied
 * - [](concepts::meta auto types)
 * - []<class... Ts>
 * - []<class... Ts>(concepts::meta auto types)
 */
template <template <class...> class T, class... Ts>
[[nodiscard]] constexpr auto operator|(T<Ts...>, auto fn);
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
[[nodiscard]] constexpr auto operator|(T<Vs...>, auto fn);
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
[[nodiscard]] constexpr auto operator|(std::tuple<Ts...>, auto fn);
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


> To build/run benchmarks

```cpp
git clone https://github.com/boost-ext/mp
git co benchmark
cd benchmark
mkdir build && cd build
cmake ..
make
```

</p>
</details>

<a name="faq"></a>
<details open><summary>FAQ</summary>
<p>

</p>
</details>

</p>
</details>

---

**Disclaimer** `MP` is not an official [Boost](https://www.boost.org/) library.
