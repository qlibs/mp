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
template <auto List, auto N, class... Ns>
auto insert = List
  | std::ranges::views::take(N)
  | boost::mp::list<Ns...>()
  | (List | std::ranges::views::drop(N));
// clang-format on

using boost::mp::operator""_c;

// clang-format off
static_assert(insert<boost::mp::list<int, double, float>(), 1_c, short> == boost::mp::list<int, short, double, float>());
// clang-format on

int main() {}
