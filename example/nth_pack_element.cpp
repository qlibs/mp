//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <ranges>

template <auto N>
auto nth_pack_element = []<class... Ts> {
  return boost::mp::list<Ts...>() | std::ranges::views::drop(boost::mp::ct<N>) |
             std::ranges::views::take(boost::mp::ct<1>) |
             []<class T> -> T { return {}; };
};

static_assert((boost::mp::list<int, double, float>() | nth_pack_element<0>) ==
              int{});
static_assert((boost::mp::list<int, double, float>() | nth_pack_element<1>) ==
              double{});
static_assert((boost::mp::list<int, double, float>() | nth_pack_element<2>) ==
              float{});

int main() {}
