//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <ranges>

using boost::mp::operator""_c;

template <auto N>
auto nth_pack_element = []<class... Ts> {
  return boost::mp::list<Ts...>() | std::ranges::views::drop(N) |
             std::ranges::views::take(1_c) | []<class T> -> T { return {}; };
};

static_assert((boost::mp::list<int, double, float>() | nth_pack_element<0_c>) ==
              int{});
static_assert((boost::mp::list<int, double, float>() | nth_pack_element<1_c>) ==
              double{});
static_assert((boost::mp::list<int, double, float>() | nth_pack_element<2_c>) ==
              float{});

int main() {}
