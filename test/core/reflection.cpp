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

  "reflection.type_id"_test = [] {
    expect(constant<boost::mp::reflection::type_id<const void> !=
                    boost::mp::reflection::type_id<void>>);
    expect(constant<boost::mp::reflection::type_id<void> !=
                    boost::mp::reflection::type_id<int>>);
    expect(constant<boost::mp::reflection::type_id<int> !=
                    boost::mp::reflection::type_id<int&>>);

    expect(constant<boost::mp::reflection::type_id<void> ==
                    boost::mp::reflection::type_id<void>>);
    expect(constant<boost::mp::reflection::type_id<const int&> ==
                    boost::mp::reflection::type_id<const int&>>);
  };

  "reflection.type_name"_test = [] {
    std::cout << boost::mp::reflection::type_name<void>() << std::endl;
    expect("void" == boost::mp::reflection::type_name<void>());
    expect("const int" == boost::mp::reflection::type_name<const int>());
    expect("foo" == boost::mp::reflection::type_name<foo>());
    expect("42" == boost::mp::reflection::type_name<42>());
  };
}
