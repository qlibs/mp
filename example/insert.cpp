//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <ranges>
#include <variant>

namespace mp = boost::mp;

using mp::operator""_c;
using mp::operator|;

// clang-format off
template <auto List, auto N, class... Ns>
auto insert = List
  | std::views::take(N)
  | mp::list<Ns...>()
  | (List | std::views::drop(N));
// clang-format on


// clang-format off
static_assert(insert<mp::list<int, double, float>(), 1_c, short> == mp::list<int, short, double, float>());
static_assert(insert<mp::type<std::variant<int, double, float>>, 1_c, short> == mp::type<std::variant<int, short, double, float>>);
// clang-format on

int main() {}
