//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <ranges>

namespace mp = boost::mp;

// clang-format off
auto hello_world = [](mp::concepts::meta auto list){
  using mp::operator""_c;
  return list | // int, foo, val, bar, double
    std::views::take(4_c) // int, foo, val, bar
  | std::views::drop(1_c) // foo, bar, val
  | std::views::transform([]<class T>()-> T const {}) // foo const, val const, bar const
  | std::views::filter([]<class T> { return requires(T t) { t.value; }; }) // foo const, val const
  | std::views::reverse; // val const, foo const
};

struct bar {};
struct foo {
  int value;
};
struct val {
  int value;
};

static_assert(mp::list<val const, foo const>() == hello_world(mp::list<int, foo, val, bar, double>()));
// clang-format on

int main() {}
