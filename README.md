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

</p>
</details>

<a name="tutorial"></a>
<details open><summary>Tutorial</summary>
<p>
</p>
</details>

<p>
</p>
</details>

<a name="examples"></a>
<details open><summary>Examples</summary>
<p>

- <a href="example/append.cpp">append</a>
- <a href="conditional.cpp">conditional</a>
- <a href="filter.cpp">filter</a>
- <a href="insert.cpp">insert</a>
- <a href="nth_pack_element.cpp">nth_pack_element</a>
- <a href="reverse.cpp">reverse</a>
- <a href="rotate.cpp">rotate</a>
- <a href="slice.cpp">slice</a>
- <a href="to_list.cpp">to_list</a>
- <a href="to_tuple.cpp">to_tuple</a>
- <a href="transform.cpp">transform</a>
- <a href="unique.cpp">unique</a>

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
