//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <boost/ut.hpp>
#include <ranges>

int main() {
  using namespace boost::ut;

  "adapt.filter.match"_test = [] {
    auto v = boost::mp::adapt(std::views::filter, []<class... Ts>(auto type) {
      return sizeof(type);
    });
    expect(2_u == (boost::mp::list<int, double>() | v).size);
  };

  "adapt.filter.doesn't match"_test = [] {
    auto v = boost::mp::adapt(std::views::filter, []<class... Ts>(auto type) {
      return not sizeof(type);
    });
    expect(0_u == (boost::mp::list<int, double>() | v).size);
  };
}
