//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#if defined(__cpp_modules) && !defined(BOOST_MP_DISABLE_MODULE)
export module boost.mp;
export import std;
#else
#pragma once
#endif

#include <array>
#include <cmath>
#include <concepts>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#define BOOST_MP_VERSION 0'0'1

namespace boost::mp::inline v0_0_1 {
namespace utility {
namespace detail {
template <auto Value>
struct value_type {
  static constexpr auto value = Value;
};
}  // namespace detail
template <auto N, auto... Ns>
constexpr auto nth_pack_element_v =
    __type_pack_element<N, detail::value_type<Ns>...>::value;

template <auto N, class... Ts>
using nth_pack_element = __type_pack_element<N, Ts...>;

template <char... Cs>
[[nodiscard]] consteval auto operator""_c() {
  return std::integral_constant<std::size_t, [] {
    std::size_t result{};
    for (const auto c : std::array{Cs...}) {
      result = result * std::size_t(10) + std::size_t(c - '0');
    }
    return result;
  }()>{};
}
namespace detail {
template <class T>
[[nodiscard]] consteval auto type_id() {
  std::size_t result{};
  for (const auto& c : __PRETTY_FUNCTION__) {
    (result ^= c) <<= 1;
  }
  return result;
}
}  // namespace detail

template <class T>
constexpr auto type_id = detail::type_id<T>();
}  // namespace utility

namespace concepts {
namespace detail {
template <class T>
concept meta = requires(T t) {
                 std::size_t{t.id};
                 std::size_t{t.size};
               };
}  // namespace detail

template <class T>
concept meta = std::ranges::random_access_range<T> and
               detail::meta<typename T::value_type>;
}  // namespace concepts

struct meta final {
  std::size_t id{};
  std::size_t size{};

  [[nodiscard]] constexpr operator auto() const { return id; }
  [[nodiscard]] constexpr auto operator==(const meta&) const -> bool = default;
};

template <class... Ts>
struct type_list final {
  static constexpr auto size = sizeof...(Ts);
  constexpr auto operator==(type_list<Ts...>) const -> bool { return true; }
  template <class... Us>
  constexpr auto operator==(type_list<Us...>) const -> bool {
    return false;
  }
};

template <auto... Vs>
struct value_list final {
  static constexpr auto size = sizeof...(Vs);
  constexpr auto operator==(value_list<Vs...>) const -> bool { return true; }
  template <auto... Us>
  constexpr auto operator==(value_list<Us...>) const -> bool {
    return false;
  }
};

template <std::size_t N>
struct fixed_string final {
  static constexpr auto size = N;

  constexpr explicit(true) fixed_string(const auto... cs) : data{cs...} {}

  constexpr explicit(false) fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, std::data(data));
  }

  [[nodiscard]] constexpr auto operator<=>(const fixed_string&) const = default;

  [[nodiscard]] constexpr explicit(false) operator std::string_view() const {
    return {std::data(data), N};
  }

  std::array<char, N + 1> data{};
};

template <std::size_t N>
fixed_string(const char (&str)[N]) -> fixed_string<N - 1>;
fixed_string(const auto... Cs) -> fixed_string<sizeof...(Cs)>;

template <class... Ts>
[[nodiscard]] constexpr auto list() {
  return type_list<Ts...>{};
}

template <auto... Vs>
[[nodiscard]] constexpr auto list() {
  return value_list<Vs...>{};
}

template <fixed_string Str>
  requires requires {
             Str.data;
             Str.size;
           }
[[nodiscard]] constexpr auto list() {
  return []<auto... Ns>(std::index_sequence<Ns...>) {
    return value_list<Str.data[Ns]...>{};
  }(std::make_index_sequence<Str.size>{});
}

template <class T>
constexpr auto to_list = [] {
  if constexpr (requires { [] { auto [p1, p2, p3] = T{}; }; }) {
    auto [p1, p2, p3] = T{};
    return type_list<decltype(p1), decltype(p2), decltype(p3)>{};
  } else if constexpr (requires { [] { auto [p1, p2] = T{}; }; }) {
    auto [p1, p2] = T{};
    return type_list<decltype(p1), decltype(p2)>{};
  } else if constexpr (requires { [] { auto [p1] = T{}; }; }) {
    auto [p1] = T{};
    return type_list<decltype(p1)>{};
  } else {
    return type_list{};
  }
}();

constexpr auto to_tuple = []<class T>(T&& obj) {
  if constexpr (requires { [&obj] { auto&& [p1, p2, p3] = obj; }; }) {
    auto&& [p1, p2, p3] = std::forward<T>(obj);
    return std::make_tuple(p1, p2, p3);
  } else if constexpr (requires { [&obj] { auto&& [p1, p2] = obj; }; }) {
    auto&& [p1, p2] = std::forward<T>(obj);
    return std::make_tuple(p1, p2);
  } else if constexpr (requires { [&obj] { auto&& [p1] = obj; }; }) {
    auto&& [p1] = std::forward<T>(obj);
    return std::make_tuple(p1);
  } else {
    return std::make_tuple();
  }
};

template <template <class...> class T, class... Ts>
[[nodiscard]] constexpr auto operator|(T<Ts...>, auto fn) {
  if constexpr (requires { fn.template operator()<Ts...>(); }) {
    return fn.template operator()<Ts...>();
  } else {
    constexpr auto vs = []<auto... Ids>(auto fn, std::index_sequence<Ids...>) {
      auto id = 0uz;
      const std::vector<meta> types{meta{.id = id++, .size = sizeof(Ts)}...};
      if constexpr (requires { fn.template operator()<Ts...>(types); }) {
        const auto vs = fn.template operator()<Ts...>(types);
        return std::pair{std::size(vs), std::array<std::size_t, sizeof...(Ids)>{
                                            vs[Ids].id...}};
      } else if constexpr (requires { fn(types); }) {
        const auto vs = fn.template operator()(types);
        return std::pair{std::size(vs), std::array<std::size_t, sizeof...(Ids)>{
                                            vs[Ids].id...}};
      }
    };

    constexpr auto size = vs(fn, std::index_sequence<>{}).first;
    constexpr auto vs_out = vs(fn, std::make_index_sequence<size>{}).second;

    return [vs_out]<auto... Ids>(std::index_sequence<Ids...>) {
      return T<utility::nth_pack_element<vs_out[Ids], Ts...>...>{};
    }(std::make_index_sequence<size>{});
  }
}

template <template <auto...> class T, auto... Vs>
[[nodiscard]] constexpr auto operator|(T<Vs...>, auto fn) {
  if constexpr (requires { fn.template operator()<Vs...>(); }) {
    return fn.template operator()<Vs...>();
  } else {
    constexpr auto vs = []<auto... Ids>(auto fn, std::index_sequence<Ids...>) {
      auto id = 0uz;
      const std::vector<meta> types{meta{.id = id++, .size = sizeof(Vs)}...};
      if constexpr (requires { fn.template operator()<Vs...>(types); }) {
        const auto vs = fn.template operator()<Vs...>(types);
        return std::pair{std::size(vs), std::array<std::size_t, sizeof...(Ids)>{
                                            vs[Ids].id...}};
      } else if constexpr (requires { fn(types); }) {
        const auto vs = fn.template operator()(types);
        return std::pair{std::size(vs), std::array<std::size_t, sizeof...(Ids)>{
                                            vs[Ids].id...}};
      }
    };

    constexpr auto size = vs(fn, std::index_sequence<>{}).first;
    constexpr auto vs_out = vs(fn, std::make_index_sequence<size>{}).second;

    return [vs_out]<auto... Ids>(std::index_sequence<Ids...>) {
      return T<utility::nth_pack_element_v<vs_out[Ids], Vs...>...>{};
    }(std::make_index_sequence<size>{});
  }
}

template <class... Ts>
[[nodiscard]] constexpr auto operator|(std::tuple<Ts...> t, auto fn) {
  if constexpr (requires { fn.template operator()<Ts...>(); }) {
    return fn.template operator()<Ts...>();
  } else {
    constexpr auto vs = []<auto... Ids>(auto fn, std::index_sequence<Ids...>) {
      auto id = 0uz;
      const std::vector<meta> types{meta{.id = id++, .size = sizeof(Ts)}...};
      if constexpr (requires { fn.template operator()<Ts...>(types); }) {
        const auto vs = fn.template operator()<Ts...>(types);
        return std::pair{std::size(vs), std::array<std::size_t, sizeof...(Ids)>{
                                            vs[Ids].id...}};
      } else if constexpr (requires { fn(types); }) {
        const auto vs = fn.template operator()(types);
        return std::pair{std::size(vs), std::array<std::size_t, sizeof...(Ids)>{
                                            vs[Ids].id...}};
      }
    };

    constexpr auto size = vs(fn, std::index_sequence<>{}).first;
    constexpr auto vs_out = vs(fn, std::make_index_sequence<size>{}).second;

    return [vs_out, t]<auto... Ids>(std::index_sequence<Ids...>) {
      return std::make_tuple(std::get<vs_out[Ids]>(t)...);
    }(std::make_index_sequence<size>{});
  }
}

template <auto N>
constexpr auto _c = std::integral_constant<decltype(N), N>{};
using utility::operator""_c;
}  // namespace boost::mp::inline v0_0_1
