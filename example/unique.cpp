//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>
#include <array>
#include <bit>
#include <boost/mp.hpp>

namespace mp = boost::mp;

template <auto Fn>
auto sort = []<class... Ts>(mp::concepts::meta auto types) {
  std::ranges::sort(types, Fn);
  return types;
};

auto by_size = [](auto lhs, auto rhs) { return lhs.size < rhs.size; };

auto unique = []<class... Ts>(mp::concepts::meta auto types) {
  auto [first, last] = std::ranges::unique(
      types, [ids = std::array{mp::utility::type_id<Ts>...}](
                 auto lhs, auto rhs) { return ids[lhs] == ids[rhs]; });
  types.erase(first, last);
  return types;
};

template <auto v>
auto unique_sort = v | sort<by_size> | unique;

// clang-format off
static_assert(unique_sort<mp::type_list<std::byte[1], std::byte[3], std::byte[2], std::byte[1], std::byte[3]>{}>
                       == mp::type_list<std::byte[1], std::byte[2], std::byte[3]>{});
// clang-format on

int main() {}
