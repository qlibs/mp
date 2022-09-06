//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <ranges>

template <class... TRhs>
constexpr auto append() {
  return []<class... TLhs> { return boost::mp::type_list<TLhs..., TRhs...>{}; };
}

template <class... TRhs>
constexpr auto append(boost::mp::type_list<TRhs...>) {
  return []<class... TLhs> { return boost::mp::type_list<TLhs..., TRhs...>{}; };
}

template <auto N, class... Ns>
auto insert = []<class... Ts> {
  auto v = boost::mp::list<Ts...>();
  auto head = v | std::ranges::views::take(N);
  auto tail = v | std::ranges::views::drop(N);
  return boost::mp::type_list{} | append(head) | append<Ns...>() | append(tail);
};

using boost::mp::operator""_c;
static_assert((boost::mp::list<int, double, float>() | insert<1_c, short>) ==
              boost::mp::list<int, short, double, float>());

int main() {}
