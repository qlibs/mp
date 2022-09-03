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

  "type_list.<Ts...>"_test = [] {
    expect(constant<(boost::mp::type_list{} | []<class... Ts> {
                      return boost::mp::type_list<Ts...>{};
                    }) == boost::mp::type_list{}>);
    expect(constant<($(int, double) | []<class... Ts> {
                      return boost::mp::type_list<Ts...>{};
                    }) == $(int, double)>);
  };

  "type_list.(types)"_test = [] {
    expect(constant<(boost::mp::type_list{} |
                     [](boost::mp::concepts::meta auto types) {
                       return types;
                     }) == boost::mp::type_list{}>);
    expect(constant<($(int, double) | [](boost::mp::concepts::meta auto types) {
                      return types;
                    }) == $(int, double)>);
  };

  "type_list.<Ts...>(types)"_test = [] {
    expect(constant<(boost::mp::type_list{} |
                     []<class... Ts>(boost::mp::concepts::meta auto types) {
                       return types;
                     }) == boost::mp::type_list{}>);
    expect(constant<($(int, double) |
                     []<class... Ts>(boost::mp::concepts::meta auto types) {
                       return types;
                     }) == $(int, double)>);
  };

  "type_list.ranges"_test = [] {
    expect(constant<(boost::mp::type_list{} | std::ranges::views::reverse) ==
                    boost::mp::type_list{}>);
    expect(constant<($(int, double) | std::ranges::views::reverse) ==
                    $(double, int)>);
  };
}
