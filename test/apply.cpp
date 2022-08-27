//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.cpp>

#include "boost/meta.hpp"

#include <bit>
#include <array>
#include <typeinfo>
#include <algorithm>

int main() {
  using namespace meta = boost::meta;

  "apply"_test = [] {
    auto sort_and_unique = []<class... Ts>(auto types) {
        std::sort(std::begin(types), std::end(types),
                  [size = std::array{sizeof(Ts)...}](auto lhs, auto rhs) {
                      return size[lhs.id] < size[rhs.id];
                  });

        types.erase(std::unique(types.begin(), types.end(),
                                [](auto lhs, auto rhs) {
                                    return lhs.type == rhs.type;
                                }), std::end(types));
        return types[1];
    };

    static_assert(typeid(meta::apply<sort_and_unique, meta::list_t<std::byte[4], std::byte[1], std::byte[2], std::byte[2], std::byte[3]>>)
               == typeid(meta::list<std::byte[2]>));
  };
}
