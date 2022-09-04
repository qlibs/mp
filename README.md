<a href="https://github.com/boost-ext/mp/actions/workflows/linux.yml" target="_blank">![Linux](https://github.com/boost-ext/mp/actions/workflows/linux.yml/badge.svg)</a>
<a href="https://godbolt.org/z/d5G9aE871">![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)</a>

# MP - ~~Template~~ Meta-Programming

| [Motivation](#motivation) | [Quick Start](#quick-start) | [Overview](#overview) | [Tutorial](#tutorial) | [Examples](#examples) | [User Guide](#user-guide) | [FAQ](#faq) | [Benchmarks](#benchmarks) |

<details open><summary>C++ <b>single header/single module</b> C++23 Meta-Programming Library</summary>
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

- Single C++23 header/module
- Small learning curve (reuses STL, ranges or any thrid-party solution)
- Easy debugging (meta-functions can be simply run at run-time too)
- Same interface for types/nttp/tuples
- Declarative by design (ranges)
- Fast compilation times (#benchmarks)

- Requirements
  - C++23 compliant compiler (clang-16+)
  - `constexpr` support inside STL (stdlibc++-16+)

</p>
</details>

<a name="tutorial"></a>
<details open><summary>Tutorial</summary>
<p>

> Firstly include or import boost.mp

```cpp
#include <boost/mp.hpp>
```
or
```cpp
import boost.mp
```

> Let's write a hello world, shall we?

```cpp
auto identity = [](boost::mp::concepts::meta types) {
  return types;
}
```

`meta` is the meta objects range which we can do operations on.
For example, sorting, changing the size, removing elements, etc...


Let's implement our meta-function.

```cpp
auto magic = boost::mp::list<int, double, float>() | identity;
```

```cpp
static_assert(magic == boost::mp::list<int, double, float>());
```

Yay, we have the first meta-function done. Notice the pipe (|)
operator. By using it multiple meta-functions can be combined.

For the next example including/importing ranges will be required

```cpp
#include <ranges>
```

Let's implement simple slice for types

```cpp
template<auto list, auto Start, auto End>
auto slice = list
   | std::ranges::views::drop(Start)
   | std::ranges::views::take(End);
```

```cpp
using boost::mp::operator""_c;
static_assert(slice<boost::mp::list<int, dobule, float>(), 1_c, 2_c> == boost::mp::list<double, float>());
```

> Notice that we have just use std::ranges at compile-time to munipulate type-list!

Let's add STL too

```
#incldue <tuple>
#incldue <algorithm>
```

This time we will sort and reverse a tuple.

> Note: All operations are supported for the following entities
  - boost::mp::type_list
  - boost::mp::value_list
  - boost::mp::fixed_string
  - std::tuple

Additionaly `type_list/value_list/fixed_string` will be deduced automatically
based on parmeters when `boost::mp::list<...>()` is used.

Okay, combing back to our sort...

````
template <auto Fn>
auto sort = [](boost::mp::concepts::meta auto types) {
  std::sort(std::begin(types), std::end(types), Fn);
  return types;
};

auto by_size = [](auto lhs, auto rhs) { return lhs.size < rhs.size; };
```

So far, nothing magical, same code as in run-time.

Let's apply it then.

```cpp
using boost::mp::operator|;
auto pack = [](auto t) {
  return boost::mp::to_tuple(t)
      | sort<by_size>; };
}
```

> Note: We used `to_tuple` which converst a struct into a tuple using reflection.
        There is also `to_list` available which produces `type_list`.

As usual, we use pipe (|) to compose functionality.

```cpp
struct not_packed {
  char c{};
  int i{};
  std::byte b{};
};
```

```cpp
static_assert(sizeof(not_packed) == 12uz);
static_assert(sizeof(pack(not_packed{})) == 8uz);
```

Okay, so far so good, but what about adding or removing from type_list?

Removing is simple, just remove from the types list.

For adding we need to use type/value space land to accomplish that.

```cpp
template <class... TRhs>
auto append = []<class... TLhs> { return boost::mp::type_list<TLhs..., TRhs...>{}; };
```

> Note: we propagtes `<class... Ts>` instead of meta types. Both options are valid.
  Aslo passing both `<class... Ts>(boost::mp::concepts::meta auto types)` is also
  correct and useful for cases when meta-types require manipulation based on types.

```cpp
template <auto v>
auto add = v | append<void>;
```

```cpp
static_assert(add<boost::mp::type_list<int, double>{}> ==
              boost::mp::type_list<int, double, void>{});
```

Okay, so what about the case when we need both?

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

> Notice that we created an array with functor values for each type, so that we
  they can be applied at meta-types manipulation level using STL.

```cpp
struct bar {};
struct foo {
  int value;
};
```

```cpp
template <auto v>
auto find_if_has_value = v | filter<[](auto t) { return requires { t.value; }; }>;
```

> Notice handy requries with lambda pattern to verify ad-hoc concepts.

```cpp
static_assert(boost::mp::type_list<foo>{} ==
              find_if_has_value<boost::mp::type_list<foo, bar>{}>);
```

That's it for now, for more let's take a look at more Examples in the following section.
</p>
</details>

<p>
</p>
</details>

<a name="examples"></a>
<details open><summary>Examples</summary>
<p>

- <a href="example/append.cpp">append</a>
- <a href="example/conditional.cpp">conditional</a>
- <a href="example/filter.cpp">filter</a>
- <a href="example/insert.cpp">insert</a>
- <a href="example/nth_pack_element.cpp">nth_pack_element</a>
- <a href="example/reverse.cpp">reverse</a>
- <a href="example/rotate.cpp">rotate</a>
- <a href="example/slice.cpp">slice</a>
- <a href="example/to_list.cpp">to_list</a>
- <a href="example/to_tuple.cpp">to_tuple</a>
- <a href="example/transform.cpp">transform</a>
- <a href="example/unique.cpp">unique</a>

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
