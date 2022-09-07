//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <variant>

namespace mp = boost::mp;

template <auto List, class... Ts>
auto append = List | mp::list<Ts...>();

// clang-format off
static_assert(append<mp::list<int, double>(), void> == mp::list<int, double, void>());
static_assert(append<mp::type<std::variant<int, double>>, float> == mp::type<std::variant<int, double, float>>);
static_assert(std::is_same_v<decltype(*append<mp::type<std::variant<int, double>>, float>), std::variant<int, double, float>>);
// clang-format on

int main() {}
