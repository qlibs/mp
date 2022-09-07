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

template <auto List>
auto reverse = List | std::views::reverse;

// clang-format off
static_assert((reverse<mp::list<int, double, float>()>) == mp::list<float, double, int>());
// clang-format on

int main() {}
