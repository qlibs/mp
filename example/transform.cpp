//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <type_traits>

namespace mp = boost::mp;

// clang-format off
template <auto List>
auto transform = List
  | []<class... Ts> { return mp::list<Ts* const...>(); };

static_assert(transform<mp::list<int, double>()> == mp::list<int* const, double* const>());
// clang-format on

int main() {}
