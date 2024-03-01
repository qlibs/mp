//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <mp>
#if defined(__clang__)
#include <ranges>

namespace mp = boost::mp;

// clang-format off
static_assert(mp::invoke(
  std::ranges::count_if,
  mp::list<int*, double>(),
  []<class T> { return std::is_pointer_v<T>; }
) == 1);
// clang-format on

#endif

int main() {}
