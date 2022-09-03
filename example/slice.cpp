//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <ranges>

template <auto Begin, auto End>
auto slice = []<class... Ts> {
  return boost::mp::list<Ts...>() | std::ranges::views::drop(Begin) |
         std::ranges::views::take(End);
};

using boost::mp::operator""_c;

static_assert((boost::mp::list<int, double, float>() | slice<0_c, 2_c>) ==
              boost::mp::list<int, double>());
static_assert((boost::mp::list<int, double, float>() | slice<1_c, 1_c>) ==
              boost::mp::list<double>());
static_assert((boost::mp::list<int, double, float>() | slice<1_c, 2_c>) ==
              boost::mp::list<double, float>());

int main() {}
