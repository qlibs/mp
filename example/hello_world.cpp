//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/mp.hpp>
#include <ranges>

namespace mp = boost::mp;

// clang-format off
auto hello_world = [](auto list, auto add_const, auto has_value){
  using mp::operator""_c;
  return list                           // int, foo, val, bar, double
    | std::views::take(4_c)             // int, foo, val, bar
    | std::views::drop(1_c)             // foo, bar, val
    | std::views::transform(add_const)  // foo const, val const, bar const
    | std::views::filter(has_value)     // foo const, val const
    | std::views::reverse               // val const, foo const
    ;
};

auto add_const = []<class T>()-> T const {};
auto has_value = []<class T> { return requires(T t) { t.value; }; };

struct bar {};
struct foo { int value; };
struct val { int value; };

static_assert(
  mp::list<val const, foo const>() ==
  hello_world(mp::list<int, foo, val, bar, double>(), add_const, has_value)
);
// clang-format on

#if __has_include(<boost/ut.hpp>)
#include <boost/ut.hpp>
#include <vector>

int main() {
  using namespace boost::ut;

  struct stub_type {
    std::size_t type{};
    bool has_value{};
    bool add_const{};
    constexpr auto operator<=>(const stub_type&) const = default;
  };

  "hello world"_test = [] {
    // given
    std::vector list{stub_type{.type = 0},
                     stub_type{.type = 1, .has_value = true}};

    auto add_const = [](auto& t) {
      t.add_const = true;
      return t;
    };
    auto has_value = [](auto t) { return t.has_value; };

    // when
    auto out = hello_world(list, add_const, has_value);
    const std::vector<stub_type> result{std::begin(out), std::end(out)};

    // then
    expect(1_u == std::size(result));
    expect(result[0] ==
           stub_type{.type = 1, .has_value = true, .add_const = true});
  };
}
#else
int main() {}
#endif
