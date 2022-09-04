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

  "utility.type_id"_test = [] {
    expect(constant<boost::mp::utility::type_id<const void> !=
                    boost::mp::utility::type_id<void>>);
    expect(constant<boost::mp::utility::type_id<void> !=
                    boost::mp::utility::type_id<int>>);
    expect(constant<boost::mp::utility::type_id<int> !=
                    boost::mp::utility::type_id<int&>>);

    expect(constant<boost::mp::utility::type_id<void> ==
                    boost::mp::utility::type_id<void>>);
    expect(constant<boost::mp::utility::type_id<const int&> ==
                    boost::mp::utility::type_id<const int&>>);
  };

  "utility.type_name"_test = [] {
    expect(constant<"void" == boost::mp::utility::type_name<void>()>);
    expect(
        constant<"const int *" == boost::mp::utility::type_name<const int*>()>);
    struct foo {};
    expect(constant<"foo" == boost::mp::utility::type_name<foo>()>);
    expect(constant<"42" == boost::mp::utility::type_name<42>()>);
  };

  "utility.<char...>_c"_test = [] {
    using boost::mp::operator""_c;
    expect(std::is_same_v<std::integral_constant<std::size_t, 42>,
                          decltype(42_c)>);
    expect(constant<0_c == 0>);
    expect(constant<42_c == 42>);
    expect(constant<1000000_c == 1'000'000>);

    expect(type<std::integral_constant<int, 3>> == boost::mp::_c<1 + 2>);
    expect(constant<1_c + 2_c == 3>);
    expect(constant<1_c + 2 == 3>);
    expect(constant<1_c - 2 == -1>);
  };
}
