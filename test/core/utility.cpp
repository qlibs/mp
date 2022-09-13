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

struct foo {};

int main() {
  using namespace boost::ut;

  "utility.nth_pack_element"_test = [] {
    expect(std::is_same_v<
           int, boost::mp::utility::nth_pack_element<0, int, void, double>>);
    expect(std::is_same_v<
           void, boost::mp::utility::nth_pack_element<1, int, void, double>>);
    expect(std::is_same_v<
           double, boost::mp::utility::nth_pack_element<2, int, void, double>>);
  };

  "utility.nth_pack_element_V"_test = [] {
    expect(constant<1 == boost::mp::utility::nth_pack_element_v<0, 1, 2, 3>>);
    expect(constant<2 == boost::mp::utility::nth_pack_element_v<1, 1, 2, 3>>);
    expect(constant<3 == boost::mp::utility::nth_pack_element_v<2, 1, 2, 3>>);
  };

  "utility.<char...>_c"_test = [] {
    using boost::mp::operator""_c;
    expect(constant<0_c == 0>);
    expect(constant<42_c == 42>);
    expect(constant<1000000_c == 1'000'000>);

    expect(type<boost::mp::utility::integral_constant<3>> ==
           boost::mp::const_t<1 + 2>{});
    expect(constant<1_c + 2_c == 3>);
    expect(constant<2_c - 1_c == 1>);
  };
}
