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
auto filter = [](auto list, auto fn){
  using namespace boost::mp;
  return list | std::views::filter<<(fn);
};
// clang-format on

struct bar {};
struct foo {
  int value;
};

// clang-format off
static_assert(mp::list<foo>() == filter(mp::list<foo, bar>(), []<class T> { return requires (T t) { t.value; };} ));
static_assert(mp::list<2, 3>() == filter(mp::list<1, 2, 3>(), [](auto i) { return i > 1; }));
static_assert(mp::list<"fbar">() == filter(mp::list<"foobar">(), [](auto c) { return c != 'o'; }));
static_assert(std::tuple{2, 3} == filter([]{return std::tuple{1, 2, 3};}, [](auto i) { return i > 1; }));
// clang-format on

int main() {}
