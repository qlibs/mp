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

template <auto Fn>
auto sort = [](mp::concepts::meta auto types) {
  std::sort(std::begin(types), std::end(types), Fn);
  return types;
};

auto by_size = [](auto lhs, auto rhs) { return lhs.size < rhs.size; };

template <class T>
auto pack = mp::reflection::to_list<T> | sort<by_size> |
            []<class... Ts> { return std::tuple<Ts...>{}; };

struct not_packed {
  char c{};
  int i{};
  std::byte b{};
};

static_assert(sizeof(not_packed) == 12u);
static_assert(sizeof(pack<not_packed>) == 8u);

int main() {}
