//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <utility>

template <class T>
concept has_value = requires(T t) { t.value; };

template <auto List>
auto conditional = List | []<class... Ts> {
  if constexpr (constexpr auto any_has_value = (has_value<Ts> or ...);
                any_has_value) {
    return std::integral_constant<int, (has_value<Ts> + ... + 0u)>{};
  } else {
    return boost::mp::type_list{};
  }
};

struct foo {
  int value{};
};

struct bar {};

// clang-format off
static_assert(conditional<boost::mp::type_list<foo, bar>{}>.value == 1);
static_assert(conditional<boost::mp::type_list<bar, bar>{}> == boost::mp::type_list{});
// clang-format on

int main() {}
