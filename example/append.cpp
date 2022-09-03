//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>

template <class... TRhs>
auto append =
    []<class... TLhs> { return boost::mp::type_list<TLhs..., TRhs...>{}; };

template <auto v>
auto add = v | append<void>;

static_assert(add<boost::mp::type_list<int, double>{}> ==
              boost::mp::type_list<int, double, void>{});

int main() {}
