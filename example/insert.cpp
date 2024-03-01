//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <mp>
#include <ranges>
#include <variant>

namespace mp = boost::mp;

// clang-format off
auto insert = [](mp::concepts::meta auto&& list, const auto pos, mp::concepts::meta auto&& elements) {
  using mp::operator|;
  return list
    | std::views::take(pos)
    | elements
    | (list | std::views::drop(pos));
};

using boost::mp::operator""_c;
static_assert(insert(mp::list<int, double>(), 1_c, boost::mp::list<short>()) == mp::list<int, short, double>());
static_assert(std::is_same_v<mp::typeof<insert, std::variant<int, double>, decltype(1_c), std::variant<short>>, std::variant<int, short, double>>);
static_assert(std::tuple{1, 2, 3, 4} == insert(std::tuple{1, 3, 4}, 1_c, std::tuple{2}));

#include <cassert>

int main(int argc, const char**) {
  assert((std::tuple{1, 3, argc, 4} == insert(std::tuple{1, 3, 4}, 2_c, std::tuple{argc})));
}
