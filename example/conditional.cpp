//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <utility>

template <class T>
concept has_value = requires(T t) { t.value; };

template <auto List>
auto conditional = List | []<class... Ts> {
  if constexpr (constexpr auto any_has_value = (has_value<Ts> or ...);
                any_has_value) {
    return std::integral_constant<int, (has_value<Ts> + ... + 0uz)>{};
  } else {
    return boost::mp::type_list{};
  }
};

struct foo {
  int value{};
};

struct bar {};

static_assert(conditional<boost::mp::type_list<foo, bar>{}>.value == 1);
static_assert(conditional<boost::mp::type_list<bar, bar>{}> ==
              boost::mp::type_list{});

#include <ranges>

template <auto List>
auto first_or_last_depending_on_size = List | []<class...> {
  using boost::mp::operator""_c;
  auto first = List | std::ranges::views::take(1_c);
  auto last =
      List | std::ranges::views::reverse | std::ranges::views::take(1_c);
  auto size = [](auto v) { return v | []<class T> { return sizeof(T); }; };

  if constexpr (size(first) > size(last)) {
    return first;
  } else {
    return last;
  }
};

static_assert(first_or_last_depending_on_size<
                  boost::mp::list<std::byte[42], std::byte[43]>()> ==
              boost::mp::list<std::byte[43]>());

static_assert(
    first_or_last_depending_on_size<
        boost::mp::list<std::byte[42], std::byte[999], std::byte[43]>()> ==
    boost::mp::list<std::byte[43]>());

static_assert(
    first_or_last_depending_on_size<
        boost::mp::list<std::byte[142], std::byte[999], std::byte[43]>()> ==
    boost::mp::list<std::byte[142]>());

static_assert(first_or_last_depending_on_size<boost::mp::list<
                  std::byte[1], std::byte[2], std::byte[3], std::byte[2]>()> ==
              boost::mp::list<std::byte[2]>());

int main() {}
