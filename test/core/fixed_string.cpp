//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <boost/ut.hpp>
#include <string_view>

template <boost::mp::fixed_string Str>
static constexpr auto str = Str;

int main() {
  using namespace boost::ut;
  using std::literals::operator""sv;

  "make fixed_string"_test = [] {
    expect(""sv == str<"">);
    expect("Foo"sv == str<"Foo">);
  };

  "make fixed_string with characters"_test = [] {
    expect(std::string_view{boost::mp::fixed_string{}}.empty());
    expect("abc"sv == boost::mp::fixed_string{'a', 'b', 'c'});
    expect("abc"sv == str<boost::mp::fixed_string{'a', 'b', 'c'}>);
  };

  "compare"_test = [] {
    constexpr auto foo = str<"Foo">;
    expect(foo == str<"Foo">);
    expect(foo != str<"Bar">);
  };
}
