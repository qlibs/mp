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
#define BOOST_MP_PRETTY_FUNCTION __PRETTY_FUNCTION__

#if defined(__cpp_modules) && !defined(BOOST_MP_DISABLE_MODULE)
export
#endif
    namespace boost::inline ext::mp::inline v0_0_1 {
namespace utility {
#if __has_builtin(__type_pack_element)
template <auto N, class... Ts>
using nth_pack_element = __type_pack_element<N, Ts...>;

template <auto N, auto... Ns>
constexpr auto nth_pack_element_v =
    __type_pack_element<N, std::integral_constant<decltype(Ns), Ns>...>::value;
#else
namespace detail {
template <class T, std::size_t N>
struct any {};
template <class... Ts>
struct inherit : Ts... {};
template <auto N, class T>
auto nth_pack_element_impl(any<T, N>) -> T;
template <auto N, class... Ts, std::size_t... Ns>
auto nth_pack_element(std::index_sequence<Ns...>)
    -> decltype(nth_pack_element_impl<N>(
        inherit<any<std::type_identity<Ts>, Ns>...>{}));
}  // namespace detail

template <auto N, class... Ts>
using nth_pack_element = typename decltype(detail::nth_pack_element<N, Ts...>(
    std::make_index_sequence<sizeof...(Ts)>{}))::type;

template <auto N, auto... Ns>
constexpr auto nth_pack_element_v =
    nth_pack_element<N, std::integral_constant<decltype(Ns), Ns>...>::value;
#endif

template <auto N>
struct integral_constant : std::integral_constant<decltype(N), N> {
  [[nodiscard]] constexpr auto operator+(const auto value) const {
    return integral_constant<N + value>{};
  }
  [[nodiscard]] constexpr auto operator-(const auto value) const {
    return integral_constant<N - value>{};
  }
  [[nodiscard]] constexpr auto operator*(const auto value) const {
    return integral_constant<N * value>{};
  }
  [[nodiscard]] constexpr auto operator/(const auto value) const {
    return integral_constant<N / value>{};
  }
};

template <char... Cs>
[[nodiscard]] consteval auto operator""_c() {
  return integral_constant<[] {
    std::size_t result{};
    for (const auto c : std::array{Cs...}) {
      result = result * std::size_t(10) + std::size_t(c - '0');
    }
    return result;
  }()>{};
}
}  // namespace utility

namespace concepts {
namespace detail {
template <class...>
constexpr auto meta = false;
template <template <class...> class T, class... Ts>
constexpr auto meta<T<Ts...>> = true;
template <template <auto...> class T, auto... Vs>
constexpr auto meta<T<Vs...>> = true;

struct callable_base {
  void operator()();
};
template <class T>
struct callable : T, callable_base {};
}  // namespace detail

template <class T>
concept callable = not requires { &detail::callable<T>::operator(); };

template <class T>
concept meta = detail::meta<T>;
}  // namespace concepts

struct meta {
  std::size_t index{};
  std::size_t size{};

  [[nodiscard]] constexpr operator auto() const { return index; }
  [[nodiscard]] constexpr auto operator==(const meta&) const -> bool = default;
};

template <auto N>
using const_t = utility::integral_constant<N>;
using utility::operator""_c;

template <class... Ts>
struct type_list {
  static constexpr auto size = sizeof...(Ts);
  constexpr auto operator==(type_list<Ts...>) const -> bool { return true; }
  template <class... Us>
  constexpr auto operator==(type_list<Us...>) const -> bool {
    return false;
  }
  constexpr auto operator[](const auto N) const {
    return utility::nth_pack_element<N, Ts...>();
  }
};

template <auto... Vs>
struct value_list {
  static constexpr auto size = sizeof...(Vs);
  constexpr auto operator==(value_list<Vs...>) const -> bool { return true; }
  template <auto... Us>
  constexpr auto operator==(value_list<Us...>) const -> bool {
    return false;
  }
  constexpr auto operator[](const auto N) const {
    return utility::nth_pack_element_v<N, Vs...>;
  }
};

template <std::size_t N>
struct fixed_string {
  static constexpr auto size = N;

  constexpr explicit(true) fixed_string(const auto... cs) : data{cs...} {}
  constexpr explicit(false) fixed_string(const char (&str)[N + 1]) {
    for (auto i = 0u; i <= N; ++i) {
      data[i] = str[i];
    }
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

template <auto Lhs, auto Rhs>
[[nodiscard]] constexpr auto operator+(const fixed_string<Lhs>& lhs,
                                       const fixed_string<Rhs>& rhs) {
  fixed_string<Lhs + Rhs> str{};
  for (auto i = 0u; i < Lhs; ++i) {
    str.data[i] = lhs.data[i];
  }
  for (auto i = 0u; i < Rhs; ++i) {
    str.data[i + Lhs] = rhs.data[i];
  }
  return str;
}

template <class... Ts>
[[nodiscard]] constexpr auto list() {
  return type_list<Ts...>{};
}

template <auto... Vs>
  requires(not(requires {
                 Vs.data;
                 Vs.size;
               } or ...))
[[nodiscard]] constexpr auto list() {
  return value_list<Vs...>{};
}

template <fixed_string Str>
[[nodiscard]] constexpr auto list() {
  return []<auto... Ns>(std::index_sequence<Ns...>) {
    return value_list<Str.data[Ns]...>{};
  }(std::make_index_sequence<Str.size>{});
}

template <auto expr, class... Ts>
using typeof = decltype(expr(std::declval<Ts>()...));

namespace detail {
template <auto N>
struct size_vs {
  std::size_t size{};
  std::array<std::size_t, N> vs{};
};
}  // namespace detail

template <class T>
constexpr auto declval()
    -> std::conditional_t<std::is_default_constructible_v<T>, T, T&&> {
  return T{};
}

template <template <class...> class T, class... Ts, template <class...> class U,
          class... Us>
  requires(not concepts::callable<U<Us...>>)
[[nodiscard]] constexpr auto operator|(T<Ts...>, U<Us...>) {
  return declval<T<Ts..., Us...>>();
}

template <class... Ts, class... Us>
[[nodiscard]] constexpr auto operator|(std::tuple<Ts...> lhs,
                                       std::tuple<Us...> rhs) {
  return std::tuple_cat(lhs, rhs);
}

namespace detail {
template <class TFn, class TPred>
struct invoke {
  TFn fn{};
  TPred pred{};

  constexpr auto operator()(std::ranges::range auto types, auto... args) const {
    const auto fns =
        std::array<bool, sizeof...(args)>{pred.template operator()<args>()...};
    if constexpr (auto v = fn(types, [fns](auto type) { return fns[type]; });
                  requires {
                    std::begin(v);
                    std::end(v);
                  }) {
      return decltype(types){std::begin(v), std::end(v)};
    } else {
      return v;
    }
  }

  template <auto... Vs>
    requires(sizeof...(Vs) > 0u)
  constexpr auto operator()(std::ranges::range auto types) const {
    if constexpr ((requires {
                     { pred.template operator()<Vs>() } -> std::same_as<bool>;
                   } or ...)) {
      const auto fns =
          std::array<bool, sizeof...(Vs)>{pred.template operator()<Vs>()...};
      if constexpr (auto v = fn(types, [fns](auto type) { return fns[type]; });
                    requires {
                      std::begin(v);
                      std::end(v);
                    }) {
        return decltype(types){std::begin(v), std::end(v)};
      } else {
        return v;
      }
    } else {
      return list<decltype(pred.template operator()<Vs>())...>();
    }
  }

  template <class... Ts>
  constexpr auto operator()(std::ranges::range auto types) const {
    if constexpr ((requires {
                     { pred.template operator()<Ts>() } -> std::same_as<bool>;
                   } or ...)) {
      const auto fns =
          std::array<bool, sizeof...(Ts)>{pred.template operator()<Ts>()...};
      if constexpr (auto v = fn(types, [fns](auto type) { return fns[type]; });
                    requires {
                      std::begin(v);
                      std::end(v);
                    }) {
        return decltype(types){std::begin(v), std::end(v)};
      } else {
        return v;
      }
    } else {
      return list<decltype(pred.template operator()<Ts>())...>();
    }
  }
};
template <class>
struct invoke_t;

template <template <class...> class T, template <class...> class U,
          template <class...> class F, class TFn, class TPred>
  requires concepts::callable<TFn> and concepts::callable<TPred>
struct invoke_t<T<U<TFn, F<TPred>>>> : invoke<TFn, TPred> {};

template <template <class...> class T, template <class...> class U, class TFn,
          class TPred>
  requires concepts::callable<TFn> and concepts::callable<TPred>
struct invoke_t<T<U<TFn, TPred>>> : invoke<TFn, TPred> {};

template <template <class...> class T, class TFn, class TPred>
  requires concepts::callable<TFn> and concepts::callable<TPred>
struct invoke_t<T<TFn, TPred>> : invoke<TFn, TPred> {};
}  // namespace detail

template <class... Ts, template <class...> class X, class T, class Pred>
constexpr auto invoke(T fn, X<Ts...>, Pred) {
  if constexpr (requires { detail::invoke<T, Pred>{}; }) {
    auto i = 0u;
    const std::vector<meta> types{meta{.index = i++, .size = sizeof(Ts)}...};
    return detail::invoke<T, Pred>{}.template operator()<Ts...>(types);
  } else {
    return fn;
  }
}

template <class... Ts, class T>
constexpr auto invoke(auto types, T fn)
  requires(sizeof...(Ts) > 0u)
{
  if constexpr (requires { detail::invoke_t<T>{}; }) {
    return detail::invoke_t<T>{}.template operator()<Ts...>(types);
  } else {
    return fn;
  }
}

template <auto... Vs, class T>
constexpr auto invoke(auto types, T fn)
  requires(sizeof...(Vs) > 0u)
{
  if constexpr (requires { detail::invoke_t<T>{}; }) {
    return detail::invoke_t<T>{}.template operator()<Vs...>(types);
  } else {
    return fn;
  }
}

template <class T>
constexpr auto invoke(auto, T fn, auto...) {
  return fn;
}

template <template <class...> class T, class... Ts>
[[nodiscard]] constexpr auto operator|(T<Ts...>, auto fn) {
  if constexpr (requires { fn.template operator()<T, Ts...>(); }) {
    return fn.template operator()<T, Ts...>();
  } else if constexpr (requires { fn.template operator()<Ts...>(); }) {
    return fn.template operator()<Ts...>();
  } else {
    constexpr auto make = [](const auto& vs) {
      if constexpr (requires { std::size(vs); }) {
        auto svs = detail::size_vs<sizeof...(Ts)>{std::size(vs)};
        for (auto i = 0u; i < svs.size; ++i) {
          svs.vs[i] = vs[i].index;
        }
        return svs;
      } else {
        return vs;
      }
    };
    constexpr auto expr = [make](auto fn) {
      auto i = 0u;
      if constexpr (const std::vector<meta> types{
                        meta{.index = i++, .size = sizeof(Ts)}...};
                    requires { fn.template operator()<Ts...>(types); }) {
        return make(invoke<Ts...>(types, fn.template operator()<Ts...>(types)));
      } else if constexpr (requires { fn(types); }) {
        return make(invoke<Ts...>(types, fn(types)));
      } else if constexpr (requires { fn(); }) {
        return make(invoke<Ts...>(types, fn()));
      } else {
        return make(invoke<Ts...>(types, fn));
      }
    };

    if constexpr (constexpr auto expr_fn = expr(fn); requires { expr_fn.vs; }) {
      return [expr_fn]<auto... Ids>(std::index_sequence<Ids...>) {
        return declval<
            T<utility::nth_pack_element<expr_fn.vs[Ids], Ts...>...>>();
      }(std::make_index_sequence<expr_fn.size>{});
    } else {
      return expr_fn;
    }
  }
}

template <template <auto...> class T, auto... Vs>
[[nodiscard]] constexpr auto operator|(T<Vs...>, auto fn) {
  if constexpr (requires { fn.template operator()<T, Vs...>(); }) {
    return fn.template operator()<T, Vs...>();
  } else if constexpr (requires { fn.template operator()<Vs...>(); }) {
    return fn.template operator()<Vs...>();
  } else {
    constexpr auto make = [](const auto& vs) {
      if constexpr (requires { std::size(vs); }) {
        auto svs = detail::size_vs<sizeof...(Vs)>{std::size(vs)};
        for (auto i = 0u; i < svs.size; ++i) {
          svs.vs[i] = vs[i].index;
        }
        return svs;
      } else {
        return vs;
      }
    };
    constexpr auto expr = [make](auto fn) {
      auto i = 0u;
      if constexpr (const std::vector<meta> types{
                        meta{.index = i++, .size = sizeof(Vs)}...};
                    requires { fn.template operator()<Vs...>(types); }) {
        return make(invoke<Vs...>(types, fn.template operator()<Vs...>(types)));
      } else if constexpr (requires { fn(types); }) {
        return make(invoke<Vs...>(types, fn(types)));
      } else if constexpr (requires { fn(); }) {
        return make(invoke<Vs...>(types, fn()));
      } else {
        return make(invoke<Vs...>(types, fn));
      }
    };

    if constexpr (constexpr auto expr_fn = expr(fn); requires { expr_fn.vs; }) {
      return [expr_fn]<auto... Ids>(std::index_sequence<Ids...>) {
        return T<utility::nth_pack_element_v<expr_fn.vs[Ids], Vs...>...>{};
      }(std::make_index_sequence<expr_fn.size>{});
    } else {
      return expr_fn;
    }
  }
}

template <template <class, auto...> class T, class R, auto... Vs>
[[nodiscard]] constexpr auto operator|(T<R, Vs...>, auto fn) {
  if constexpr (requires { fn.template operator()<T, Vs...>(); }) {
    return fn.template operator()<T, Vs...>();
  } else if constexpr (requires { fn.template operator()<Vs...>(); }) {
    return fn.template operator()<Vs...>();
  } else {
    constexpr auto make = [](const auto& vs) {
      if constexpr (requires { std::size(vs); }) {
        auto svs = detail::size_vs<sizeof...(Vs)>{std::size(vs)};
        for (auto i = 0u; i < svs.size; ++i) {
          svs.vs[i] = vs[i].index;
        }
        return svs;
      } else {
        return vs;
      }
    };
    constexpr auto expr = [make](auto fn) {
      auto i = 0u;
      if constexpr (const std::vector<meta> types{
                        meta{.index = i++, .size = sizeof(Vs)}...};
                    requires { fn.template operator()<Vs...>(types); }) {
        return make(invoke<Vs...>(types, fn.template operator()<Vs...>(types)));
      } else if constexpr (requires { fn(types); }) {
        return make(invoke<Vs...>(types, fn(types)));
      } else if constexpr (requires { fn(); }) {
        return make(invoke<Vs...>(types, fn()));
      } else {
        return make(invoke<Vs...>(types, fn));
      }
    };

    if constexpr (constexpr auto expr_fn = expr(fn); requires { expr_fn.vs; }) {
      return [expr_fn]<auto... Ids>(std::index_sequence<Ids...>) {
        return T<R, utility::nth_pack_element_v<expr_fn.vs[Ids], Vs...>...>{};
      }(std::make_index_sequence<expr_fn.size>{});
    } else {
      return expr_fn;
    }
  }
}

template <class T>
[[nodiscard]] constexpr auto operator|(T t, auto fn)
  requires requires { t(); }
{
  return [&]<class... Ts>(std::tuple<Ts...>) {
    if constexpr (requires { fn.template operator()<std::tuple, Ts...>(); }) {
      return fn.template operator()<std::tuple, Ts...>();
    } else if constexpr (requires { fn.template operator()<Ts...>(); }) {
      return fn.template operator()<Ts...>();
    } else {
      constexpr auto make = [](const auto& vs) {
        auto svs = detail::size_vs<sizeof...(Ts)>{std::size(vs)};
        for (auto i = 0u; i < svs.size; ++i) {
          svs.vs[i] = vs[i].index;
        }
        return svs;
      };
      constexpr auto expr = [make](auto&& fn, auto t) {
        return [&]<auto... Ids>(std::index_sequence<Ids...>) {
          if constexpr (const std::vector<meta> types{
                            meta{.index = Ids, .size = sizeof(Ts)}...};
                        requires { fn(types, std::get<Ids>(t())...); }) {
            return make(invoke<std::get<Ids>(t())...>(
                types, fn(types, std::get<Ids>(t())...)));
          } else if constexpr (requires { fn(types); }) {
            return make(invoke<std::get<Ids>(t())...>(types, fn(types)));
          } else if constexpr (requires { fn(); }) {
            return make(invoke<std::get<Ids>(t())...>(types, fn()));
          } else {
            return make(invoke<std::get<Ids>(t())...>(types, fn));
          }
        }(std::make_index_sequence<sizeof...(Ts)>{});
      };
      constexpr auto expr_fn = expr(fn, t);
      return [&]<auto... Ids>(std::index_sequence<Ids...>) {
        return std::tuple{std::get<expr_fn.vs[Ids]>(t())...};
      }(std::make_index_sequence<expr_fn.size>{});
    }
  }(t());
}

template <class... Ts>
[[nodiscard]] constexpr auto operator|(std::tuple<Ts...> t, auto fn) {
  if constexpr (requires { fn.template operator()<std::tuple, Ts...>(); }) {
    return fn.template operator()<std::tuple, Ts...>();
  } else if constexpr (requires { fn.template operator()<Ts...>(); }) {
    return fn.template operator()<Ts...>();
  } else if constexpr (requires { fn(std::get<Ts>(t)...); }) {
    return fn(std::get<Ts>(t)...);
  } else {
    constexpr auto make = [](const auto& vs) {
      auto svs = detail::size_vs<sizeof...(Ts)>{std::size(vs)};
      for (auto i = 0u; i < svs.size; ++i) {
        svs.vs[i] = vs[i].index;
      }
      return svs;
    };
    constexpr auto expr = [make](auto fn) {
      auto i = 0u;
      if constexpr (const std::vector<meta> types{
                        meta{.index = i++, .size = sizeof(Ts)}...};
                    requires { fn.template operator()<Ts...>(types); }) {
        return make(invoke<Ts...>(types, fn.template operator()<Ts...>(types)));
      } else if constexpr (requires { fn(types); }) {
        return make(invoke<Ts...>(types, fn(types)));
      } else if constexpr (requires { fn(); }) {
        return make(invoke<Ts...>(types, fn()));
      } else {
        return make(invoke<Ts...>(types, fn));
      }
    };
    constexpr auto expr_fn = expr(fn);
    return [expr_fn, t]<auto... Ids>(std::index_sequence<Ids...>) {
      return std::tuple{std::get<expr_fn.vs[Ids]>(t)...};
    }(std::make_index_sequence<expr_fn.size>{});
  }
}

namespace reflection {
namespace detail {
template <class T>
[[nodiscard]] consteval auto type_id() {
  std::size_t result{};
  for (const auto& c : BOOST_MP_PRETTY_FUNCTION) {
    (result ^= c) <<= 1;
  }
  return result;
}
}  // namespace detail

template <class T>
constexpr auto type_id = detail::type_id<T>();

#if defined(__clang__)
#define BOOST_MP_TYPE_NAME_OFFSET 45
#define BOOST_MP_TYPE_NAME_V_OFFSET 45
#else
#define BOOST_MP_TYPE_NAME_OFFSET 73
#define BOOST_MP_TYPE_NAME_V_OFFSET 78
#endif

template <class T>
[[nodiscard]] consteval auto type_name() {
  return std::string_view{
      &BOOST_MP_PRETTY_FUNCTION[BOOST_MP_TYPE_NAME_OFFSET],
      sizeof(BOOST_MP_PRETTY_FUNCTION) - BOOST_MP_TYPE_NAME_OFFSET - 2};
}

template <auto T>
[[nodiscard]] consteval auto type_name() {
  return std::string_view{
      &BOOST_MP_PRETTY_FUNCTION[BOOST_MP_TYPE_NAME_V_OFFSET],
      sizeof(BOOST_MP_PRETTY_FUNCTION) - BOOST_MP_TYPE_NAME_V_OFFSET - 2};
}

#undef BOOST_MP_TYPE_NAME_OFFSET

template <class T>
[[nodiscard]] constexpr auto to_tuple(T&& obj) {
  // clang-format off
  if constexpr (requires { [&obj] { auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10] = obj; }; }) {
    auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10] = std::forward<T>(obj);
    return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p8, p10);
  } else if constexpr (requires { [&obj] { auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9] = obj; }; }) {
    auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9] = std::forward<T>(obj);
    return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8, p9);
  } else if constexpr (requires { [&obj] { auto&& [p1, p2, p3, p4, p5, p6, p7, p8] = obj; }; }) {
    auto&& [p1, p2, p3, p4, p5, p6, p7, p8] = std::forward<T>(obj);
    return std::make_tuple(p1, p2, p3, p4, p5, p6, p7, p8);
  } else if constexpr (requires { [&obj] { auto&& [p1, p2, p3, p4, p5, p6, p7] = obj; }; }) {
    auto&& [p1, p2, p3, p4, p5, p6, p7] = std::forward<T>(obj);
    return std::make_tuple(p1, p2, p3, p4, p5, p6, p7);
  } else if constexpr (requires { [&obj] { auto&& [p1, p2, p3, p4, p5, p6] = obj; }; }) {
    auto&& [p1, p2, p3, p4, p5, p6] = std::forward<T>(obj);
    return std::make_tuple(p1, p2, p3, p4, p5, p6);
  } else if constexpr (requires { [&obj] { auto&& [p1, p2, p3, p4, p5] = obj; }; }) {
    auto&& [p1, p2, p3, p4, p5] = std::forward<T>(obj);
    return std::make_tuple(p1, p2, p3, p4, p5);
  } else if constexpr (requires { [&obj] { auto&& [p1, p2, p3, p4] = obj; }; }) {
    auto&& [p1, p2, p3, p4] = std::forward<T>(obj);
    return std::make_tuple(p1, p2, p3, p4);
  } else if constexpr (requires { [&obj] { auto&& [p1, p2, p3] = obj; }; }) {
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
  // clang-format on
}

template <class T>
constexpr auto to_list = [] /*[[nodiscard]]*/ {
  // clang-format off
  if constexpr (requires { [] { auto [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10] = T{}; }; }) {
    auto [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10] = T{};
    return type_list<decltype(p1), decltype(p2), decltype(p3), decltype(p4), decltype(p5), decltype(p6), decltype(p7), decltype(p8), decltype(p9), decltype(p10)>{};
  } else if constexpr (requires { [] { auto [p1, p2, p3, p4, p5, p6, p7, p8, p9] = T{}; }; }) {
    auto [p1, p2, p3, p4, p5, p6, p7, p8, p9] = T{};
    return type_list<decltype(p1), decltype(p2), decltype(p3), decltype(p4), decltype(p5), decltype(p6), decltype(p7), decltype(p8), decltype(p9)>{};
  } else if constexpr (requires { [] { auto [p1, p2, p3, p4, p5, p6, p7, p8] = T{}; }; }) {
    auto [p1, p2, p3, p4, p5, p6, p7, p8] = T{};
    return type_list<decltype(p1), decltype(p2), decltype(p3), decltype(p4), decltype(p5), decltype(p6), decltype(p7), decltype(p8)>{};
  } else if constexpr (requires { [] { auto [p1, p2, p3, p4, p5, p6, p7] = T{}; }; }) {
    auto [p1, p2, p3, p4, p5, p6, p7] = T{};
    return type_list<decltype(p1), decltype(p2), decltype(p3), decltype(p4), decltype(p5), decltype(p6), decltype(p7)>{};
  } else if constexpr (requires { [] { auto [p1, p2, p3, p4, p5, p6] = T{}; }; }) {
    auto [p1, p2, p3, p4, p5, p6] = T{};
    return type_list<decltype(p1), decltype(p2), decltype(p3), decltype(p4), decltype(p5), decltype(p6)>{};
  } else if constexpr (requires { [] { auto [p1, p2, p3, p4, p5] = T{}; }; }) {
    auto [p1, p2, p3, p4, p5] = T{};
    return type_list<decltype(p1), decltype(p2), decltype(p3), decltype(p4), decltype(p5)>{};
  } else if constexpr (requires { [] { auto [p1, p2, p3, p4] = T{}; }; }) {
    auto [p1, p2, p3, p4] = T{};
    return type_list<decltype(p1), decltype(p2), decltype(p3), decltype(p4)>{};
  } else if constexpr (requires { [] { auto [p1, p2, p3] = T{}; }; }) {
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
  // clang-format on
}();
}  // namespace reflection

template <template <class...> class T, class... Ts>
constexpr auto for_each(const T<Ts...>&, auto expr) {
  if constexpr (requires {
                  expr(std::integral_constant<std::size_t, 0u>{}, (Ts{}, ...));
                }) {
    [&]<std::size_t... Ns>(std::index_sequence<Ns...>) {
      (expr(std::integral_constant<std::size_t, Ns>{}, Ts{}) or ...);
    }
    (std::make_index_sequence<sizeof...(Ts)>{});
  } else {
    (expr(Ts{}) or ...);
  }
}

template <template <auto...> class T, auto... Ts>
constexpr auto for_each(const T<Ts...>&, auto expr) {
  if constexpr (requires {
                  expr(std::integral_constant<std::size_t, 0u>{}, (Ts, ...));
                }) {
    [&]<std::size_t... Ns>(std::index_sequence<Ns...>) {
      (expr(std::integral_constant<std::size_t, Ns>{}, Ts) or ...);
    }
    (std::make_index_sequence<sizeof...(Ts)>{});
  } else {
    (expr(Ts) or ...);
  }
}

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}  // namespace boost::inline ext::mp::inline v0_0_1
#undef BOOST_MP_PRETTY_FUNCTION
