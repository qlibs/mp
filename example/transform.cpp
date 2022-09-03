//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>

template <auto Fn>
auto transform = []<class... Ts> {
  return boost::mp::type_list<decltype(Fn.template operator()<Ts>())...>{};
};

auto add_pointer = []<class T> -> T* { return {}; };

static_assert((boost::mp::type_list<int, double>{} | transform<add_pointer>) ==
              boost::mp::type_list<int*, double*>{});

int main() {}
