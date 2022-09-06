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

template <auto Fn>
auto sort = []<class... Ts>(boost::mp::concepts::meta auto types) {
  std::sort(std::begin(types), std::end(types), Fn);
  return types;
};

auto by_size = [](auto lhs, auto rhs) { return lhs.size < rhs.size; };

auto unique = []<class... Ts>(boost::mp::concepts::meta auto types) {
  types.erase(
      std::unique(std::begin(types), std::end(types),
                  [ids = std::array{boost::mp::utility::type_id<Ts>...}](
                      auto lhs, auto rhs) { return ids[lhs] == ids[rhs]; }),
      std::end(types));
  return types;
};

template <auto v>
auto unique_sort = v | sort<by_size> | unique;

// clang-format off
static_assert(unique_sort<boost::mp::type_list<std::byte[1], std::byte[3], std::byte[2], std::byte[1], std::byte[3]>{}>
                       == boost::mp::type_list<std::byte[1], std::byte[2], std::byte[3]>{});
// clang-format on

int main() {}
