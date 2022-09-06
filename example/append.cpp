//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <variant>

template <class... TRhs>
auto append = []<template <class...> class T, class... TLhs> {
  return boost::mp::type<T<TLhs..., TRhs...>>;
};

template <auto List, class... Ts>
auto add = List | append<Ts...>;

static_assert(*add<boost::mp::list<int, double>(), void> ==
              boost::mp::list<int, double, void>());
using boost::mp::operator|;
static_assert(add<boost::mp::type<std::variant<int, double>>, float> ==
              boost::mp::type<std::variant<int, double, float>>);

template <class... Ts>
struct list {
  list() = delete;
};

static_assert(add<boost::mp::type<list<int, double>>, float> ==
              boost::mp::type<list<int, double, float>>);

int main() {}
