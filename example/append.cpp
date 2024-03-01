//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <mp>
#include <variant>

namespace mp = boost::mp;

// clang-format off
auto append = [](mp::concepts::meta auto&& list, mp::concepts::meta auto&& elements) {
  using mp::operator|;
  return list | elements;
};

static_assert(append(mp::list<int, double>(), boost::mp::list<short>()) == mp::list<int, double, short>());
static_assert(std::is_same_v<mp::typeof<append, std::variant<int, double>, std::variant<short>>, std::variant<int, double, short>>);
static_assert(std::tuple{1, 2, 3} == append(std::tuple{1, 2}, std::tuple{3}));
// clang-format on

int main() {}
