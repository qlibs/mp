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

// clang-format off
auto transform = [](auto list) {
  return list | std::views::transform([]<class T>() -> const T* { });
};

static_assert(transform(mp::list<int, double>()) == mp::list<const int*, const double*>());
// clang-format on

int main() {}
