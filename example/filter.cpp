//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <ranges>

// clang-format off
auto filter = boost::mp::adapt(
    std::ranges::views::filter,
    []<class... Ts>(auto type) {return std::array{requires(Ts t) { t.value; }...}[type];
  });
// clang-format on

struct bar {};
struct foo {
  int value;
};

// clang-format off
static_assert((boost::mp::list<foo, bar>() | filter) == boost::mp::list<foo>());
// clang-format on

int main() {}
