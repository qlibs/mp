//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <ranges>

namespace mp = boost::mp;
using mp::operator%;

// clang-format off
template <auto List>
auto filter = List
  | std::views::filter % []<class T> { return requires (T t) { t.value; }; };
// clang-format on

struct bar {};
struct foo {
  int value;
};

// clang-format off
static_assert(filter<mp::list<foo, bar>()> == mp::list<foo>());
// clang-format on

int main() {}
