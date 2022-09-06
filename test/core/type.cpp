//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <boost/ut.hpp>
#include <type_traits>

int main() {
  using namespace boost::ut;

  "type.comparable"_test = [] {
    expect(constant<boost::mp::type<int> == boost::mp::type<int>>);
    expect(constant<boost::mp::type<void> != boost::mp::type<int>>);
    expect(constant<boost::mp::type<int> != boost::mp::type<void>>);
    expect(constant<boost::mp::type<int*> != boost::mp::type<int>>);
  };

  "type.*"_test = [] {
    expect(std::is_same_v<decltype(*boost::mp::type<int>), int>);
    expect(std::is_same_v<decltype(*boost::mp::type<float>), float>);
  };

  "type.size"_test = [] {
    expect(constant<boost::mp::type<void>.size == 1u>);
    expect(constant<boost::mp::type<int>.size == 1u>);
  };
}
