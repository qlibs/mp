//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <ranges>

auto reverse = []<class... Ts> {
  return boost::mp::list<Ts...>() | std::ranges::views::reverse;
};

static_assert((boost::mp::list<int, double, float>() | reverse) ==
              boost::mp::list<float, double, int>());

int main() {}
