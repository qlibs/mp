//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <ranges>

template <boost::mp::fixed_string Str, auto N>
auto split = boost::mp::list<Str>() | []<auto... Cs> {
  auto v = boost::mp::list<Cs...>();
  auto head = v | std::ranges::views::take(N);
  auto tail = v | std::ranges::views::drop(boost::mp::_c<N + 1>);
  return std::tuple(head, tail);
};

using boost::mp::operator""_c;
static_assert(split<"Hello World", 5_c> ==
              std::tuple(boost::mp::list<"Hello">(),
                         boost::mp::list<"World">()));

int main() {}
