//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>

template <auto F>
auto filter = []<class... Ts>(boost::mp::concepts::meta auto types) {
  types.erase(std::remove_if(
                  std::begin(types), std::end(types),
                  [](auto type) { return std::array{not F(Ts{})...}[type]; }),
              std::end(types));
  return types;
};

struct bar {};
struct foo {
  int value;
};

template <auto v>
auto find_if_has_value =
    v | filter<[](auto t) { return requires { t.value; }; }>;

static_assert(boost::mp::type_list<foo>{} ==
              find_if_has_value<boost::mp::type_list<foo, bar>{}>);

int main() {}
