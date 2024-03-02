//
// Copyright (c) 2022-2024 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#if not defined(MP)
#define MP 1'0'0

#include <array>
#include <concepts>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#if defined(_MSC_VER)
#define __has_builtin(...) 0
#endif

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wundefined-var-template"
#endif

namespace mp::inline v1_0_0 {
#if defined(__clang__) and __has_builtin(__type_pack_element)
template <auto N, class... Ts> using nth_pack_element = __type_pack_element<N, Ts...>;
template <auto N, auto... Ns> constexpr auto nth_pack_element_v = __type_pack_element<N, std::integral_constant<decltype(Ns), Ns>...>::value;
#else
namespace detail {
template <class, std::size_t> struct type_id {};
template <class... Ts> struct inherit : Ts... {};
template <auto N, class T> auto nth_pack_element_impl(type_id<T, N>) -> T;
template <auto N, class... Ts, std::size_t... Ns>
auto nth_pack_element(std::index_sequence<Ns...>) -> decltype(nth_pack_element_impl<N>( inherit<type_id<std::type_identity<Ts>, Ns>...>{}));
} // namespace detail
template <auto N, class... Ts> using nth_pack_element = typename decltype(detail::nth_pack_element<N, Ts...>( std::make_index_sequence<sizeof...(Ts)>{}))::type;
template <auto N, auto... Ns> constexpr auto nth_pack_element_v = nth_pack_element<N, std::integral_constant<decltype(Ns), Ns>...>::value;
#endif

template <auto N>
struct integral_constant : std::integral_constant<decltype(N), N> {
  [[nodiscard]] constexpr auto operator+(const auto value) const { return integral_constant<N + value>{}; }
  [[nodiscard]] constexpr auto operator-(const auto value) const { return integral_constant<N - value>{}; }
  [[nodiscard]] constexpr auto operator*(const auto value) const { return integral_constant<N * value>{}; }
  [[nodiscard]] constexpr auto operator/(const auto value) const { return integral_constant<N / value>{}; }
};

template <char... Cs> [[nodiscard]] consteval auto operator""_c() {
  return integral_constant<[] {
    std::size_t result{};
    for (const auto c : std::array{Cs...}) {
      result = result * std::size_t(10) + std::size_t(c - '0');
    }
    return result;
  }()>{};
}

template <auto N> inline constexpr auto _c = integral_constant<N>{};

template <std::size_t Size>
struct fixed_string {
  static constexpr auto size = Size;
  constexpr fixed_string() = default;
  constexpr explicit(false) fixed_string(const char* str) { for (auto i = 0u; i < Size; ++i) { data[i] = str[i]; } }
  [[nodiscard]] constexpr auto operator<=>(const fixed_string&) const = default;
  [[nodiscard]] constexpr explicit(false) operator std::string_view() const {
    return {std::data(data), Size};
  }
  std::array<char, Size> data{};
};
template <std::size_t Size> fixed_string(const char (&)[Size]) -> fixed_string<Size-1>;

template <auto Lhs, auto Rhs>
[[nodiscard]] constexpr auto operator+(const fixed_string<Lhs>& lhs, const fixed_string<Rhs>& rhs) {
  fixed_string<Lhs + Rhs> str{};
  for (auto i = 0u; i < Lhs; ++i) { str.data[i] = lhs.data[i]; }
  for (auto i = 0u; i < Rhs; ++i) { str.data[i + Lhs] = rhs.data[i]; }
  return str;
}

template<class T> extern T type;

template <class... Ts>
struct type_list {
  static constexpr auto size = sizeof...(Ts);
  constexpr auto operator==(type_list<Ts...>) const -> bool { return true; }
  template <class... Us> constexpr auto operator==(type_list<Us...>) const -> bool { return false; }
  constexpr auto operator[](const auto N) const { return nth_pack_element<N, Ts...>(); }
};

template <auto... Vs>
struct value_list {
  static constexpr auto size = sizeof...(Vs);
  constexpr auto operator==(value_list<Vs...>) const -> bool { return true; }
  template <auto... Us> constexpr auto operator==(value_list<Us...>) const -> bool { return false; }
  constexpr auto operator[](const auto N) const { return nth_pack_element_v<N, Vs...>; }
};

template <class... Ts> [[nodiscard]] constexpr auto list() { return type_list<Ts...>{}; }

template <auto... Vs> requires(not(requires { Vs.data; Vs.size; } or ...))
[[nodiscard]] constexpr auto list() { return value_list<Vs...>{}; }

template <fixed_string Str>
[[nodiscard]] constexpr auto list() {
  return []<auto... Ns>(std::index_sequence<Ns...>) {
    return value_list<Str.data[Ns]...>{};
  }(std::make_index_sequence<Str.size>{});
}

namespace detail {
#if defined(_MSC_VER)
template <template<class...> class T, class Fn, class... Ts>
constexpr auto propagate(const T<Fn, Ts...>&){ return T<Fn, Ts...>{Ts{}...}; }
template <class T> constexpr auto propagate(const T&) { return T{}; }
#else
template <class T> constexpr auto propagate(const T& t) -> decltype(auto) { return t; }
#endif

template <class T, std::size_t N>
struct static_vector {
  constexpr auto push_back(const T& value) { values_[size_++] = value; }
  [[nodiscard]] constexpr auto operator[](auto i) const { return values_[i]; }
  [[nodiscard]] constexpr auto size() const { return size_; }
  std::array<T, N> values_{};
  std::size_t size_{};
};

template<class T>
constexpr auto to(auto r) {
  T v{};
  for (const auto x : r) { v.push_back(x); }
  return v;
}

template<auto Begin, auto End>
constexpr auto iota() {
  std::array<int, End-Begin> r{};
  for (auto i = Begin; auto& e : r) { e = i++; }
  return r;
}

template<auto... Ns, class... Ts> [[nodiscard]] constexpr auto make(std::tuple<Ts...> t) { return std::tuple{std::get<Ns>(t)...}; }
template<auto... Ns, template<class...> class T, class... Ts> [[nodiscard]] constexpr auto make(T<Ts...>) { return T<mp::nth_pack_element<Ns, Ts...>...>{}; }
template<auto... Ns, template<auto...> class T, auto... Ts> [[nodiscard]] constexpr auto make(T<Ts...>) { return T<mp::nth_pack_element_v<Ns, Ts...>...>{}; }
template<template<class...> class TRng, class A, class Fn> auto adapter(TRng<A, Fn>) -> A;
template<template<class...> class F, template<class...> class TRng, class A, class Fn> auto adapter(F<TRng<A, Fn>>) -> A;
template<class T> auto adapter(T) -> void;
template<template<class...> class TRng, class A, class Fn> auto pred(TRng<A, Fn>) -> Fn;
template<template<class...> class F, template<class...> class TRng, template<class...> class X, class A, class Fn> auto pred(F<TRng<A, X<Fn>>>) -> Fn;
template<class T> auto pred(T) -> void;
} // namespace detail

template <template<class...> class T, class... Ts> // requires
[[nodiscard]] constexpr auto operator|(T<Ts...> t, auto fn) {
  if constexpr (requires { fn.template operator()<T, Ts...>(); }) {
    return fn.template operator()<T, Ts...>();
  } else if constexpr (requires { fn.template operator()<Ts...>(); }) {
    return fn.template operator()<Ts...>();
  } else {
    using namespace detail;
    using A = decltype(adapter(fn));
    using Fn = decltype(pred(fn));

    if constexpr ((requires { Fn{}(Ts{}); } and ...)) {
      if constexpr (requires { std::array{Fn{}(Ts{})...}; }) { // invocable
        constexpr auto indices = to<static_vector<std::size_t, sizeof...(Ts)>>(A{}([indices = std::array{Fn{}(Ts{})...}](auto i) { return indices[i]; })(iota<0, sizeof...(Ts)>()));
        return [&]<auto... Is>(std::index_sequence<Is...>) {
          return make<indices[Is]...>(t);
        }(std::make_index_sequence<std::size(indices)>());
      } else {
        return T<decltype(Fn{}(Ts{}))...>{}; // pure
      }
    } else if constexpr ((requires { Fn{}.template operator()<Ts>(); } and ...)) {
      if constexpr (requires { std::array{Fn{}.template operator()<Ts>()...}; }) { // invocable
        constexpr auto indices = to<static_vector<std::size_t, sizeof...(Ts)>>(A{}([indices = std::array{Fn{}.template operator()<Ts>()...}](auto i) { return indices[i]; })(iota<0, sizeof...(Ts)>()));
        return [&]<auto... Is>(std::index_sequence<Is...>) {
          return make<indices[Is]...>(t);
        }(std::make_index_sequence<std::size(indices)>());
      } else {
        return T<decltype(Fn{}.template operator()<Ts>())...>{}; // pure
      }
    } else {
      constexpr auto indices = [](auto fn) {
        auto i = 0;
        std::vector vs{(void((Ts*)0), i++)...};
        if constexpr (requires { fn.template operator()<Ts...>(vs); }) {
          return detail::to<detail::static_vector<std::size_t, sizeof...(Ts)>>(fn.template operator()<Ts...>(vs)); // others
        } else if constexpr (requires{ fn(vs); }) {
          return detail::to<detail::static_vector<std::size_t, sizeof...(Ts)>>(fn(vs)); // others
        }
      }(fn);
      return [&]<auto... Is>(std::index_sequence<Is...>) {
        return make<indices[Is]...>(t);
      }(std::make_index_sequence<std::size(indices)>());
    }
  }
}

template <template<auto...> class T, auto... Ts> // requires
[[nodiscard]] constexpr auto operator|(T<Ts...> t, auto fn) {
  if constexpr (requires { fn.template operator()<T, Ts...>(); }) {
    return fn.template operator()<T, Ts...>();
  } else if constexpr (requires { fn.template operator()<Ts...>(); }) {
    return fn.template operator()<Ts...>();
  } else {
    using namespace detail;
    using A = decltype(adapter(fn));
    using Fn = decltype(pred(fn));

    if constexpr ((requires { Fn{}(Ts); } and ...)) {
      if constexpr (requires { std::array{Fn{}(Ts)...}; }) { // invocable
        constexpr auto indices = to<static_vector<std::size_t, sizeof...(Ts)>>(A{}([indices = std::array{Fn{}(Ts)...}](auto i) { return indices[i]; })(iota<0, sizeof...(Ts)>()));
        return [&]<auto... Is>(std::index_sequence<Is...>) {
          return make<indices[Is]...>(t);
        }(std::make_index_sequence<std::size(indices)>());
      } else {
        return T<Fn{}(Ts)...>{}; // pure
      }
    } else if constexpr ((requires { Fn{}.template operator()<Ts>(); } and ...)) {
      if constexpr (requires { std::array{Fn{}.template operator()<Ts>()...}; }) { // invocable 
        constexpr auto indices = to<static_vector<std::size_t, sizeof...(Ts)>>(A{}([indices = std::array{Fn{}.template operator()<Ts>()...}](auto i) { return indices[i]; })(iota<0, sizeof...(Ts)>()));
        return [&]<auto... Is>(std::index_sequence<Is...>) {
          return make<indices[Is]...>(t);
        }(std::make_index_sequence<std::size(indices)>());
      } else {
        return T<Fn{}.template operator()<Ts>()...>{}; // pure
      }
    } else {
      constexpr auto indices = [](auto fn) {
        auto i = 0;
        std::vector vs{(void(Ts), i++)...};
        if constexpr (requires { fn.template operator()<Ts...>(vs); }) {
          return detail::to<detail::static_vector<std::size_t, sizeof...(Ts)>>(fn.template operator()<Ts...>(vs)); // others
        } else if constexpr (requires{ fn(vs); }) {
          return detail::to<detail::static_vector<std::size_t, sizeof...(Ts)>>(fn(vs)); // others
        }
      }(fn);
      return [&]<auto... Is>(std::index_sequence<Is...>) {
        return make<indices[Is]...>(t);
      }(std::make_index_sequence<std::size(indices)>());
    }
  }
}

template <template <class...> class T, class... Ts, template <class...> class U, class... Us>
[[nodiscard]] constexpr auto operator+(T<Ts...>, U<Us...>) { return type<T<Ts..., Us...>>; }
template <class... Ts, class... Us> [[nodiscard]] constexpr auto operator+(std::tuple<Ts...> lhs, std::tuple<Us...> rhs) { return std::tuple_cat(lhs, rhs); }
} // namespace mp::inline v1_0_0

#if not defined(MP_DISABLE_STATIC_ASSERT_TESTS)
#include <ranges>
#include <algorithm>
#include <variant>

namespace MP_TEST {
using namespace std::literals;
using namespace mp;

static_assert(([] {
  struct bar { };
  struct foo { int value; };
  struct val { int value; };
  struct structural {};

  // nth_pack_element
  static_assert(std::is_same_v<int, nth_pack_element<0, int, void, double>>);
  static_assert(std::is_same_v<void, nth_pack_element<1, int, void, double>>);
  static_assert(std::is_same_v<double, nth_pack_element<2, int, void, double>>);
  static_assert(1 == nth_pack_element_v<0, 1, 2, 3>);
  static_assert(2 == nth_pack_element_v<1, 1, 2, 3>);
  static_assert(3 == nth_pack_element_v<2, 1, 2, 3>);

  // ""_c/_c
  static_assert(0_c == 0);
  static_assert(42_c == 42);
  static_assert(1000000_c == 1'000'000);
  static_assert(1_c + 2_c == 3);
  static_assert(2_c - 1_c == 1);
  static_assert(_c<0> == 0_c);
  static_assert(42_c == _c<42>);
  static_assert(2_c - _c<1> == 1);

  // fixed_string
  {
    static_assert(0u == mp::fixed_string<0>{}.size);
    static_assert(""sv == mp::fixed_string{""});
    static_assert("Foo"sv == mp::fixed_string{"Foo"});

    constexpr auto foo = mp::fixed_string{"Foo"};
    static_assert(foo == mp::fixed_string{"Foo"});
    static_assert(foo != mp::fixed_string{"Bar"});
  }

  // type_list
  {
    static_assert(mp::type_list{} == mp::type_list{});
    static_assert(mp::type_list<void>{} == mp::type_list<void>{});
    static_assert(mp::type_list<int, double>{} == mp::type_list<int, double>{});
    static_assert(mp::type_list<void>{} != mp::type_list<int>{});
    static_assert(mp::type_list<void, int>{} != mp::type_list<int, void>{});
  }

  // value list
  {
    static_assert(mp::value_list{} == mp::value_list{});
    static_assert(mp::value_list<1>{} == mp::value_list<1>{});
    static_assert(mp::value_list<1, 2>{} == mp::value_list<1, 2>{});
    static_assert(mp::value_list<2>{} != mp::value_list<1, 2>{});
    static_assert(mp::value_list<2, 1>{} != mp::value_list<1, 2>{});
    static_assert(mp::value_list<structural{}>{} == mp::value_list<structural{}>{});
    static_assert(mp::value_list<structural{}>{} != mp::value_list<42>{});
    }

  // list
  {
    static_assert(mp::list<int, void>() == mp::type_list<int, void>{});
    static_assert(mp::list<1, 2, 3>() == mp::value_list<1, 2, 3>{});
    static_assert(mp::list<structural{}, 1>() == mp::value_list<structural{}, 1>{});
  }

  // tuple.reverse
  {
    static_assert((std::tuple{1} | std::views::reverse) == std::tuple{1});
    static_assert((std::tuple{1, 2} | std::views::reverse) == std::tuple{2, 1});
    static_assert((std::tuple{1, 2., '3'} | std::views::reverse) == std::tuple{'3', 2., 1});
  }

  // tuple.take
  {
    static_assert((std::tuple{1, 2, 3} | std::views::take(1_c)) == std::tuple{1});
    static_assert((std::tuple{1, 2, 3} | std::views::take(2_c)) == std::tuple{1, 2});
  }

  // tuple.drop
  {
    static_assert((std::tuple{1, 2, 3} | std::views::drop(1_c)) == std::tuple{2, 3});
    static_assert((std::tuple{1, 2, 3} | std::views::drop(2_c)) == std::tuple{3});
  }

  // tuple.take.drop
  {
    static_assert((std::tuple{1, 2, 3} | std::views::drop(1_c) | std::views::take(1_c)) == std::tuple{2});
  }

  // tuple.complex
  {
    auto ops = [](auto list) {
      auto as_const = []<class T>() -> const T { return {}; };

      return list
        | std::views::take(4_c)
        | std::views::drop(1_c)
        | std::views::transform(as_const)
        | std::views::filter([](auto t) { return requires { t.value; }; })
        | std::views::reverse
        ;
    };

    static_assert(mp::list<val const, foo const>() == ops(mp::list<int, foo, val, bar, double>()));
  }

  // type_list<Ts...>
  {
    static_assert((mp::type_list{} | []<class... Ts> { return mp::type_list<Ts...>{}; }) == mp::type_list{});
    static_assert((mp::list<int, double>() | []<class... Ts> { return mp::type_list<Ts...>{}; }) == mp::list<int, double>());
  }

  // type_list.types
  {
    static_assert((mp::type_list{} | [](std::ranges::range auto types) { return types; }) == mp::type_list{});
    static_assert((mp::list<int, double>() | [](std::ranges::range auto types) { return types; }) == mp::list<int, double>());
    static_assert((mp::list<int, double>() | [](std::ranges::range auto types) { return std::array{types[0]}; }) == mp::list<int>());
  }

  // type_list<Ts...>(types)
  {
    static_assert((mp::type_list{} | []<class... Ts>(std::ranges::range auto types) { return types; }) == mp::type_list{});
    static_assert((mp::list<int, double>() | []<class... Ts>(std::ranges::range auto types) { return types; }) == mp::list<int, double>());
  }

  // type_list.ranges
  {
    static_assert((mp::type_list{} | std::views::reverse) == mp::type_list{});
    static_assert((mp::list<int, double>() | std::views::reverse) == mp::list<double, int>());
  }

  // value_list<Vs...>
  {
    static_assert((mp::value_list{} | []<auto... Vs> { return mp::value_list<Vs...>{}; }) == mp::value_list{});
    static_assert((mp::list<1, 2>() | []<auto... Vs> { return mp::value_list<Vs...>{}; }) == mp::list<1, 2>());
  }

  // value_list(types)
  {
    static_assert((mp::value_list{} | [](std::ranges::range auto types) { return types; }) == mp::value_list{});
    static_assert((mp::list<1, 2>() | [](std::ranges::range auto types) { return types; }) == mp::list<1, 2>());
  }

  // value_list<Vs...>(types)
  {
    static_assert((mp::value_list{} | []<auto... Vs>(std::ranges::range auto types) { return types; }) == mp::value_list{});
    static_assert((mp::list<1, 2>() | []<auto... Vs>(std::ranges::range auto types) { return types; }) == mp::list<1, 2>());
  }

  // value_list.compound
  {
    auto sort = []<auto... Vs>(std::ranges::range auto types) {
      std::sort(std::begin(types), std::end(types), [values = std::array{Vs...}](auto lhs, auto rhs) { return values[lhs] < values[rhs]; });
      return types;
    };

    auto unique = []<auto... Vs>(std::ranges::range auto types) {
      types.erase(std::unique(std::begin(types), std::end(types), [values = std::array{Vs...}](auto lhs, auto rhs) { return values[lhs] == values[rhs]; }), std::end(types));
      return types;
    };

    static_assert((mp::list<1, 2>() | sort) == mp::list<1, 2>());
    static_assert((mp::list<1, 2, 2>() | sort | unique) == mp::list<1, 2>());
    static_assert((mp::list<1, 1, 2, 2>() | sort | unique) == mp::list<1, 2>());
    static_assert((mp::list<1, 2, 1, 2>() | sort | unique) == mp::list<1, 2>());
    static_assert((mp::list<1, 2, 3>() | sort | unique) == mp::list<1, 2, 3>());
    static_assert((mp::list<1, 3, 2>() | sort | unique) == mp::list<1, 2, 3>());
    static_assert((mp::list<2, 2, 1, 1, 3>() | sort | unique) == mp::list<1, 2, 3>());

    auto slice = [](auto list, auto begin, auto end) {
      return list
        | std::views::drop(begin)
        | std::views::take(end - 1_c);
    };
    static_assert(std::is_same_v<decltype(slice(type<std::variant<int, double, float, short>>, 1_c, 3_c)), std::variant<double, float>>);
  }
}(), true));
} // namespace MP_TEST
#endif
#endif // MP
