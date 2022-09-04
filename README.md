<a href="https://github.com/boost-ext/mp/actions/workflows/linux.yml" target="_blank">![Linux](https://github.com/boost-ext/mp/actions/workflows/linux.yml/badge.svg)</a>
<a href="https://godbolt.org/z/d5G9aE871">![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)</a>

# MP - ~~Template~~ Meta-Programming

| [Motivation](#motivation) | [Quick Start](#quick-start) | [Overview](#overview) | [Tutorial](#tutorial) | [Examples](#examples) | [User Guide](#user-guide) | [Benchmarks](#benchmarks) | [FAQ](#faq) |

<details open><summary>C++ <b>single header/single module</b> C++20 Meta-Programming Library</summary>
<p>

<a name="motivation"></a>
<details open><summary>Motivation</summary>

<p>

```cpp
#include <ranges>

template <auto Begin, auto End, auto List>
auto slice = List
           | std::ranges::views::drop(Begin)
           | std::ranges::views::take(End);

static_assert(slice<1_c, 2_c, list<int, double, float>> == list<double, float>);
```

</p>

<p>

```cpp
#include <algorithm>

auto sort_by_size = [](boost::mp::concepts::meta auto types) {
  std::sort(std::begin(types), std::end(types),
    [](auto lhs, auto rhs) { return lhs.size < rhs.size; });
  return types;
};

struct not_packed {
  char c{};
  int i{};
  std::byte b{};
};

static_assert(sizeof(not_packed) == 12uz);
static_assert(sizeof(to_tuple(not_packed{}) | sort_by_size) == 8uz);
```

</p>

</details>

<a name="quick-start"></a>
<details open><summary>Quick Start</summary>
<p>

> Just play on https://godbolt.org/z/d5G9aE871

</p>
</details>

<a name="overview"></a>
<details open><summary>Overview</summary>
<p>

- Single C++20 header/module
- Small learning curve (reuses STL, ranges or any thrid-party solution)
- Easy debugging (meta-functions can be simply run at run-time too)
- Same interface for types/nttp/tuples
- Declarative by design (ranges)
- Fast compilation times (see benchmarks)

- Requirements
  - C++20 compliant compiler (clang-16+)
  - `constexpr` support inside STL (stdlibc++-16+)

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
import boost.mp
```

> Okay, let's write a hello world, shall we?

First step is to add our new meta-function.

```cpp
auto identity = [](boost::mp::concepts::meta types) {
  return types;
}
```

`meta` is a meta objects range (like `vector<meta>`) which we can do operations on.
For example, sorting, changing the size, removing elements, etc...

Let's apply our firt meta-function.

```cpp
auto magic = boost::mp::list<int, double, float>() | identity;
```

```cpp
static_assert(magic == boost::mp::list<int, double, float>());
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
   | std::ranges::views::drop(Start)
   | std::ranges::views::take(End);
```

> Notice that we have just use std::ranges at compile-time to munipulate type-list!

```cpp
using boost::mp::operator""_c;
static_assert(slice<boost::mp::list<int, dobule, float>(), 1_c, 2_c>
           == boost::mp::list<double, float>());
```

`""_c` is an User Defined Literal which represents constant integral value which
is required for simulating passing constexpr parameters which aren't supported
in C++.

Let's add STL too, why not

```
#incldue <tuple>
#incldue <algorithm>
```

This time we will sort and reverse a tuple

> Note: All operations are supported for the following entities
  - `boost::mp::type_list`
  - `boost::mp::value_list`
  - `boost::mp::fixed_string`
  - `std::tuple`

Additionaly `type_list/value_list/fixed_string` will be deduced automatically
based on parameters when `boost::mp::list<...>()` is used.

Okay, combing back to our sort...

```cpp
template <auto Fn>
auto sort = [](boost::mp::concepts::meta auto types) {
  std::sort(std::begin(types), std::end(types), Fn);
  return types;
};

> Note With ranges that could `actions::sort(types, Fn)`

auto by_size = [](auto lhs, auto rhs) { return lhs.size < rhs.size; };
```

So far, nothing magical, same code as in run-time!

Let's apply it then

```cpp
using boost::mp::operator|;
auto pack = [](auto t) {
  return boost::mp::to_tuple(t) | sort<by_size>;
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
static_assert(sizeof(not_packed) == 12uz);
static_assert(sizeof(pack(not_packed{})) == 8uz);
```

Okay, so far so good, but what about adding or removing from type_list?

Removing is simple as we can just erase elements from the meta types as before.

For adding we need to use type/value space land to accomplish that, though.

```cpp
template <class... TRhs>
auto append = []<class... TLhs> {
  return boost::mp::type_list<TLhs..., TRhs...>{};
};
```

> Note: we propagtes `<class... Ts>` instead of meta types. Both options are valid.
  Aslo passing both `<class... Ts>(boost::mp::concepts::meta auto types)` is also
  correct and useful for cases when meta-types require manipulation based on types.

> Note: With C++ and Universal Tempalte Parameters we will be able to unify it

```cpp
template <template auto... TRhs>
auto append = []<tempalte auto... TLhs> {
  return boost::mp::list<TLhs..., TRhs...>;
};
```

But, let's back to reality for now and apply our meta-function

```cpp
template <auto List>
auto add = list | append<void>; // adds void type
```

```cpp
static_assert(add<boost::mp::type_list<int, double>{}> ==
              boost::mp::type_list<int, double, void>{});
```

Okay, so what about the case when we need meta-types and Ts...?

```cpp
template <auto F>
auto filter = []<class... Ts>(boost::mp::concepts::meta auto types) {
  types.erase(std::remove_if(
                  std::begin(types), std::end(types),
                  [](auto type) { return std::array{not F(Ts{})...}[type]; }),
              std::end(types));
  return types;
};
```

> Notice that we created an array with functor values for each type, so that
  they can be applied at meta-types manipulation level using STL.

```cpp
struct bar {};
struct foo {
  int value;
};
```

```cpp
template <auto List>
auto find_if_has_value =
  List
| filter<[](auto t) { return requires { t.value; }; }>;
```

> Notice handy `requries with lambda` pattern to verify ad-hoc concepts.

```cpp
static_assert(boost::mp::type_list<foo>{} ==
              find_if_has_value<boost::mp::type_list<foo, bar>{}>);
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

</p>
</details>

<a name="user-guide"></a>
<details open><summary>User-Guide</summary>
<p>

</p>
</details>

<a name="benchmarks"></a>
<details open><summary>Benchmarks</summary>
<p>

- <a href="https://boost-ext.github.io/mp/benchmark/sort_unique_reverse">sort_unique_reverse</a>
- <a href="https://boost-ext.github.io/mp/benchmark/transform_filter">transform_filter</a>
- <a href="https://boost-ext.github.io/mp/benchmark/conditional_drop_sum">conditional_drop_sum</a>

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

**Disclaimer** `MP` is not an official Boost library.
