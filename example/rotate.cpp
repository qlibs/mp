//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>
#include <boost/mp.hpp>

auto rotate = [](boost::mp::concepts::meta auto types) {
  std::rotate(std::begin(types), std::begin(types) + 1, std::end(types));
  return types;
};

// clang-format off
static_assert((boost::mp::list<int, double, float>() | rotate) == boost::mp::list<double, float, int>());
// clang-format on

int main() {}
