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

template <auto List, auto Begin, auto End>
auto slice = List | std::views::drop(Begin) | std::views::take(End);

using mp::operator""_c;

// clang-format off
static_assert(slice<mp::list<int, double, float>(), 0_c, 2_c> == mp::list<int, double>());
static_assert(slice<mp::list<int, double, float>(), 1_c, 1_c> == mp::list<double>());
static_assert(slice<mp::list<int, double, float>(), 1_c, 2_c> == mp::list<double, float>());
// clang-format on

int main() {}
