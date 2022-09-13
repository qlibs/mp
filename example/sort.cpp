//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>
#include <bit>
#include <boost/mp.hpp>

namespace mp = boost::mp;

// clang-format off
auto sort = [](auto list) {
  return list | []<class... Ts>(auto types) {
    std::ranges::sort(types,
      [names = std::array{mp::utility::type_name<Ts>()...}]
        (auto lhs, auto rhs) { return names[lhs] < names[rhs]; });
    return types;
  };
};

static_assert(sort(mp::list<int, short, double, float>())
                == mp::list<double, float, int, short>());
// clang-format on

int main() {}
