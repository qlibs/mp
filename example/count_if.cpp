//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#if defined(__clang__)
#include <ranges>

namespace mp = boost::mp;

using mp::operator|;

// clang-format off
template <auto List>
constexpr auto count_if = List
  | std::ranges::count_if([]<class T> { return std::is_pointer_v<T>; });

static_assert(count_if<mp::list<int*, double>()> == 1);
// clang-format on

#endif

int main() {}
