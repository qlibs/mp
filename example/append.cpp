//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <variant>

template <auto List, class... Ts>
auto add = List | boost::mp::list<Ts...>();

// clang-format off
static_assert(add<boost::mp::list<int, double>(), void> == boost::mp::list<int, double, void>());
// clang-format on

int main() {}
