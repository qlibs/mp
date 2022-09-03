//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>
#include <boost/mp.hpp>

auto rotate = []<class... Ts> {
  return boost::mp::list<Ts...>() | [](boost::mp::concepts::meta auto types) {
    std::rotate(std::begin(types), std::begin(types) + 1, std::end(types));
    return types;
  };
};

static_assert((boost::mp::list<int, double, float>() | rotate) ==
              boost::mp::list<double, float, int>());

int main() {}
