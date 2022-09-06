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

template <auto List, auto N>
auto nth_pack_element =
    List | std::ranges::views::drop(N) | std::ranges::views::take(1_c);

// clang-format off
static_assert(nth_pack_element<boost::mp::list<int, double, float>(), 0_c>[0_c] == int{});
static_assert(nth_pack_element<boost::mp::list<int, double, float>(), 1_c>[0_c] == double{});
static_assert(nth_pack_element<boost::mp::list<int, double, float>(), 2_c>[0_c] == float{});
// clang-format on

int main() {}
