<a href="http://www.boost.org/LICENSE_1_0.txt" target="_blank">![Boost Licence](http://img.shields.io/badge/license-boost-blue.svg)</a>
<a href="https://github.com/boost-ext/mp/releases" target="_blank">![Version](https://badge.fury.io/gh/boost-ext%2Fmp.svg)</a>
<a href="https://godbolt.org/z/Wch96eT53">![build](https://img.shields.io/badge/build-blue.svg)</a>
<a href="https://godbolt.org/z/Wch96eT53">![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)</a>

# MP - ~~Template~~ Meta-Programming

> https://en.wikipedia.org/wiki/Metaprogramming

---

| [Motivation](#motivation) | [Tutorial](#tutorial) | [Examples](#examples) | [API](#api) | [Benchmarks](#benchmarks) | [FAQ](#faq) |

### Features

- Single header (https://raw.githubusercontent.com/boost-ext/mp/main/mp)
- Minimal learning curve (reuses STL, ranges or any third-party algorithms for stl.container)
- Easy debugging (meta-functions can be simply run at run-time!)
- Same interface for `types/values/tuples`
- Declarative by design (composable using pipe operator, support for ranges)
- Verifies itself upon include (aka runs all tests via static_asserts)
- Fast compilation times (see [benchmarks](#benchmarks))

### Requirements

- C++20 ([clang++16+, g++12+](https://godbolt.org/z/73fv5vfMM))

---

<p>

<a name="motivation"></a>
<details open><summary>Motivation</summary>

<p>

> Make ~~Template~~ Meta-Programming easier by leveraging run-time approach at compile-time.
  If one knows how to use stl.algorithms/ranges one can consider themself a TMP expert now as well!

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

<a name="tutorial"></a>
<details open><summary>Tutorial</summary>
<p>

> Firstly include `mp`

```cpp
#include <mp>
```

> Okay, let's write a hello world, shall we?

First step is to add our new meta-function.

```cpp
auto identity = [](std::ranges::range types) {
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

For the next example including ranges will be required

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
auto sort = [](std::ranges::range auto types) {
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
  return reflect::to<std::tuple>(t) | sort<by_size>;
}
```

> Note: We used https://github.com/boost-ext/reflect reflection library to convert a struct into a tuple.

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

Dealing with new types it's a bit different but not difficult either.

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
auto transform = [](auto list){
  return list | std::views::transform([]<class T> -> Ts* const {})
};
```

It's that easy, we just apply addition of `const pointer` to all `Ts...`.

```cpp
static_assert(transform(mp::list<int, double>) ==
                        mp::list<int* const, double* const>);
```

Okay, so what about the case when we need meta-types and Ts...?

```cpp
auto filter = std::views::filterclass T> { return requires(T t) { t.value; }; });
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
Examples in the following section and the API.
</p>
</details>

<p>
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
- <a href="https://godbolt.org/z/hs79W63P9">state_machine</a>

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
