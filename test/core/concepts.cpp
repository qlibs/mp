//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <boost/ut.hpp>
#include <variant>

template <class...>
struct types;

template <auto...>
struct values;

int main() {
  using namespace boost::ut;

  "concepts.meta"_test = [] {
    expect(constant<not boost::mp::concepts::meta<void>>);
    expect(constant<not boost::mp::concepts::meta<int>>);
    expect(constant<boost::mp::concepts::meta<types<>>>);
    expect(constant<boost::mp::concepts::meta<values<>>>);
    expect(constant<boost::mp::concepts::meta<std::tuple<>>>);
    expect(constant<boost::mp::concepts::meta<std::variant<>>>);
  };
}
