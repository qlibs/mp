//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <mp>
#include <ranges>

namespace mp = boost::mp;

using mp::operator""_c;

template <auto...>
struct ct_string {};

template <mp::fixed_string Str, auto N>
constexpr auto split = mp::list<Str>() | []<auto... Cs> {
  auto list = mp::list<Cs...>();
  auto to_ct_string = []<auto... Vs> { return ct_string<Vs...>{}; };
  auto head = list | std::views::take(N) | to_ct_string;
  auto tail = list | std::views::drop(N + 1_c) | to_ct_string;
  return mp::list<head, tail>();
};

static_assert(split<"Hello World", 5_c> ==
              mp::list<ct_string<'H', 'e', 'l', 'l', 'o'>{},
                       ct_string<'W', 'o', 'r', 'l', 'd'>{}>());

int main() {}
