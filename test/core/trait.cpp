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

  "trait"_test = [] {
    expect(
        std::is_same_v<typename decltype(boost::mp::trait<std::add_const>())::
                           template fn<int>::type,
                       const int>);
    expect(
        std::is_same_v<typename decltype(boost::mp::trait<std::add_pointer>())::
                           template fn<int>::type,
                       int*>);

    expect(std::is_same_v<typename decltype(boost::mp::trait([]<class T> {
                            return boost::mp::type<T*>;
                          }))::template fn<int>::type::value_type,
                          int*>);
  };
}
