//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>
#include <boost/mp.hpp>
#include <boost/ut.hpp>
#include <ranges>

int main() {
  using namespace boost::ut;
  using boost::mp::operator|;
  using boost::mp::operator""_c;

  "tuple.reverse"_test = [] {
    expect((std::tuple{1} | std::views::reverse) == std::tuple{1});
    expect((std::tuple{1, 2} | std::views::reverse) == std::tuple{2, 1});
    expect((std::tuple{1, 2., "3"} | std::views::reverse) ==
           std::tuple{"3", 2., 1});
  };

  "tuple.take"_test = [] {
    expect((std::tuple{1, 2, 3} | std::views::take(1_c)) == std::tuple{1});
    expect((std::tuple{1, 2, 3} | std::views::take(2_c)) == std::tuple{1, 2});
  };

  "tuple.drop"_test = [] {
    expect((std::tuple{1, 2, 3} | std::views::drop(1_c)) == std::tuple{2, 3});
    expect((std::tuple{1, 2, 3} | std::views::drop(2_c)) == std::tuple{3});
  };

  "tuple.take.drop"_test = [] {
    expect((std::tuple{1, 2, 3} | std::views::drop(1_c) |
            std::views::take(1_c)) == std::tuple{2});
  };
}
