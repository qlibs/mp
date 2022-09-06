//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <ranges>

template <class T>
concept HasValue = requires(T t) { t.value; };

const auto has_value = []<class... Ts>(auto type) {
  return std::array{HasValue<Ts>...}[type];
};

using boost::mp::operator%;

template <auto List>
auto filter = List | std::views::filter % has_value;

struct bar {};
struct foo {
  int value;
};

// clang-format off
static_assert(filter<boost::mp::list<foo, bar>()> == boost::mp::list<foo>());
// clang-format on

int main() {}
