//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <boost/ut.hpp>

int main() {
  using namespace boost::ut;

  "type_id"_test = [] {
    expect(
        constant<boost::mp::type_id<const void> != boost::mp::type_id<void>>);
    expect(constant<boost::mp::type_id<void> != boost::mp::type_id<int>>);
    expect(constant<boost::mp::type_id<int> != boost::mp::type_id<int&>>);

    expect(constant<boost::mp::type_id<void> == boost::mp::type_id<void>>);
    expect(constant<boost::mp::type_id<const int&> ==
                    boost::mp::type_id<const int&>>);
  };
}
