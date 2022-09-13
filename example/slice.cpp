//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <ranges>
#include <tuple>
#include <variant>

namespace mp = boost::mp;

using boost::mp::operator""_c;

// clang-format off
auto slice = [](auto list, auto begin, auto end) {
  using boost::mp::operator|;
  return list
    | std::views::drop(begin)      // use std.ranges
    | std::views::take(end - 1_c); // any library which can operate on containers is supported!
};

// type_list
static_assert(slice(mp::list<int, double, float, short>(), 1_c, 3_c) ==
                    mp::list<double, float>());

// variant.type
static_assert(std::is_same_v<
    mp::typeof<slice, std::variant<int, double, float, short>, mp::const_t<1>, mp::const_t<3>>,
                      std::variant<double, float>>
);

// value_list
static_assert(slice(mp::list<1, 2, 3, 4>(), 1_c, 3_c) ==
                    mp::list<2, 3>());

// tuple of values
static_assert(slice(std::tuple{1, 2, 3, 4}, 1_c, 3_c) ==
                    std::tuple{2, 3});

#include <cassert>

int main(int argc, const char**) {
  // run-time tuple of values
  assert((slice(std::tuple{1, argc, 3, 4}, 1_c, 3_c) ==
                std::tuple{argc, 3}));
}
