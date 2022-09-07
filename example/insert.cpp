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

// clang-format off
template <auto N, class... Ns>
auto insert = [](auto list) {
  using mp::operator|;

  return list
    | std::views::take(N)
    | mp::list<Ns...>()
    | (list | std::views::drop(N));
};

using mp::operator""_c;
static_assert(insert<1_c, short>(mp::list<int, double, float>()) == mp::list<int, short, double, float>());
static_assert(insert<1_c, short>(mp::type<std::variant<int, double, float>>) == mp::type<std::variant<int, short, double, float>>);
// clang-format on

int main() {}
