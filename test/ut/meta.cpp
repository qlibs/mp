//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <array>
#include <boost/mp.hpp>
#include <boost/ut.hpp>
#include <vector>

int main() {
  using namespace boost::ut;

  "meta.comparable"_test = [] {
    expect(boost::mp::meta{} == boost::mp::meta{});
    expect(boost::mp::meta{.index = 42, .size = 4} ==
           boost::mp::meta{.index = 42, .size = 4});
    expect(boost::mp::meta{.index = 42, .size = 4} !=
           boost::mp::meta{.index = 42, .size = 10000});
    expect(boost::mp::meta{.index = 42, .size = 4} !=
           boost::mp::meta{.index = 1000, .size = 4});
  };

  "meta.concept"_test = [] {
    expect(not constant<boost::mp::concepts::meta<void>>);
    expect(not constant<boost::mp::concepts::meta<int>>);
    expect(not constant<boost::mp::concepts::meta<boost::mp::meta>>);
    expect(not constant<boost::mp::concepts::meta<std::vector<int>>>);
    expect(
        constant<boost::mp::concepts::meta<std::array<boost::mp::meta, 42>>>);
    expect(constant<boost::mp::concepts::meta<std::vector<boost::mp::meta>>>);
  };
}
