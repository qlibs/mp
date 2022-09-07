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

template <auto List>
auto transform = List | mp::trait<std::add_pointer>;

// clang-format off
static_assert(transform<mp::list<int, double>()> == mp::list<int*, double*>());
// clang-format on

int main() {}
