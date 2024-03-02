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

#if defined (_MSC_VER)
#define __has_builtin(...) 0
#endif

namespace mp::inline v1_0_0 {
namespace utility {
#if __has_builtin(__type_pack_element)
template <auto N, class... Ts>
using nth_pack_element = __type_pack_element<N, Ts...>;

template <auto N, auto... Ns>
constexpr auto nth_pack_element_v =
    __type_pack_element<N, std::integral_constant<decltype(Ns), Ns>...>::value;
#else
namespace detail {
template <class, std::size_t> struct type_id {};
template <class... Ts>
struct inherit : Ts... {};
template <auto N, class T>
auto nth_pack_element_impl(type_id<T, N>) -> T;
template <auto N, class... Ts, std::size_t... Ns>
auto nth_pack_element(std::index_sequence<Ns...>)
    -> decltype(nth_pack_element_impl<N>(
        inherit<type_id<std::type_identity<Ts>, Ns>...>{}));
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

struct callable_base { void operator()(); };
template <class T> struct callable : T, callable_base {};
}  // namespace detail

template <class T>
concept callable = not requires { &detail::callable<T>::operator(); };

template <class T>
concept meta = detail::meta<T>;
}  // namespace concepts

struct meta {
  constexpr meta() = default;
  constexpr explicit(true) meta(const std::size_t index, const std::size_t size) : index{index}, size {size} { }

  [[nodiscard]] constexpr operator auto() const { return index; }
  [[nodiscard]] constexpr auto operator<=>(const meta&) const = default;

  std::size_t index{};
  std::size_t size{};
};

template <auto N> using const_t = utility::integral_constant<N>;

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

template <std::size_t Size>
struct fixed_string {
  static constexpr auto size = Size;
  constexpr fixed_string() = default;
  constexpr explicit(false) fixed_string(const char* str) {
    for (auto i = 0u; i < Size; ++i) { data[i] = str[i]; }
  }
  [[nodiscard]] constexpr auto operator<=>(const fixed_string&) const = default;
  [[nodiscard]] constexpr explicit(false) operator std::string_view() const {
    return {std::data(data), Size};
  }

  std::array<char, Size> data{};
};
template <std::size_t Size> fixed_string(const char (&)[Size]) -> fixed_string<Size-1>;

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
  requires(not(requires { Vs.data; Vs.size; } or ...))
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
template<class T> extern const T ext{};

template <template<class...> class T, class Fn, class... Ts>
constexpr auto default_construct(std::type_identity<T<Fn, Ts...>>){ return T<Fn, Ts...>{Ts{}...}; }
template <class T> constexpr auto default_construct(std::type_identity<T>) { return T{}; }
}  // namespace detail

template <template <class...> class T, class... Ts, template <class...> class U,
          class... Us>
  requires(not concepts::callable<U<Us...>>)
[[nodiscard]] constexpr auto operator|(T<Ts...>, U<Us...>) {
  return detail::ext<T<Ts..., Us...>>;
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
    const std::vector<meta> types{meta{i++, sizeof(Ts)}...};
    return detail::invoke<T, Pred>{}.template operator()<Ts...>(types);
  } else {
    return fn;
  }
}

template <class... Ts, class T>
constexpr decltype(auto) invoke(auto types, T fn)
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
      [[maybe_unused]] auto i = 0u;
      if constexpr (const std::vector<meta> types{
                        meta{i++, sizeof(Ts)}...};
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
        return detail::ext<T<utility::nth_pack_element<expr_fn.vs[Ids], Ts...>...>>;
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
      [[maybe_unused]] auto i = 0u;
      if constexpr (const std::vector<meta> types{
                        meta{i++, sizeof(Vs)}...};
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
      [[maybe_unused]] auto i = 0u;
      if constexpr (const std::vector<meta> types{
                        meta{i++, sizeof(Vs)}...};
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
                            meta{Ids, sizeof(Ts)}...};
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
      [[maybe_unused]] auto i = 0u;
      if constexpr (const std::vector<meta> types{meta{i++, sizeof(Ts)}...}; requires { fn.template operator()<Ts...>(types); }) {
        return make(invoke<Ts...>(types, fn.template operator()<Ts...>(types)));
      } else if constexpr (requires { fn(types); }) {
        return make(invoke<Ts...>(types, fn(types)));
      } else if constexpr (requires { fn(); }) {
        return make(invoke<Ts...>(types, fn()));
      } else {
        return make(invoke<Ts...>(types, fn));
      }
    };
    constexpr auto expr_fn = expr(detail::default_construct(std::type_identity<decltype(fn)>{}));
    return [expr_fn, t]<auto... Ids>(std::index_sequence<Ids...>) {
      return std::tuple{std::get<expr_fn.vs[Ids]>(t)...};
    }(std::make_index_sequence<expr_fn.size>{});
  }
}

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

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
}  // namespace mp::inline v0_0_1

#if not defined(MP_DISABLE_STATIC_ASSERT_TESTS)
#include <ranges>
#include <algorithm>

static_assert(([] {
  using namespace std::literals;
  using namespace mp;

  struct foo {};
  struct structural {};

  // concepts.meta
  static_assert(not mp::concepts::meta<void>);
  static_assert(not mp::concepts::meta<int>);
  static_assert(mp::concepts::meta<std::tuple<>>);

  // make fixed_string
  static_assert(0u == mp::fixed_string<0>{}.size);
  static_assert(""sv == mp::fixed_string{""});
  static_assert("Foo"sv == mp::fixed_string{"Foo"});

  // compare
  constexpr auto foo = mp::fixed_string{"Foo"};
  static_assert(foo == mp::fixed_string{"Foo"});
  static_assert(foo != mp::fixed_string{"Bar"});

  // type_list
  static_assert(mp::type_list{} == mp::type_list{});
  static_assert(mp::type_list<void>{} == mp::type_list<void>{});
  static_assert(mp::type_list<int, double>{} == mp::type_list<int, double>{});
  static_assert(mp::type_list<void>{} != mp::type_list<int>{});
  static_assert(mp::type_list<void, int>{} != mp::type_list<int, void>{});

  // value list
  static_assert(mp::value_list{} == mp::value_list{});
  static_assert(mp::value_list<1>{} == mp::value_list<1>{});
  static_assert(mp::value_list<1, 2>{} == mp::value_list<1, 2>{});
  static_assert(mp::value_list<2>{} != mp::value_list<1, 2>{});
  static_assert(mp::value_list<2, 1>{} != mp::value_list<1, 2>{});
  static_assert(mp::value_list<structural{}>{} == mp::value_list<structural{}>{});
  static_assert(mp::value_list<structural{}>{} != mp::value_list<42>{});

  // list
  static_assert(mp::list<int, void>() == mp::type_list<int, void>{});
  static_assert(mp::list<1, 2, 3>() == mp::value_list<1, 2, 3>{});
  static_assert(mp::list<structural{}, 1>() == mp::value_list<structural{}, 1>{});

  // meta.comparable
  static_assert(mp::meta{} == mp::meta{});
  static_assert(mp::meta{42, 4} == mp::meta{42, 4});
  static_assert(mp::meta{42, 4} != mp::meta{42, 10000});
  static_assert(mp::meta{42, 4} != mp::meta{1000, 4});

  // tuple.reverse
  static_assert((std::tuple{1} | std::views::reverse) == std::tuple{1});
  static_assert((std::tuple{1, 2} | std::views::reverse) == std::tuple{2, 1});
  static_assert((std::tuple{1, 2., '3'} | std::views::reverse) == std::tuple{'3', 2., 1});

  // tuple.take
  static_assert((std::tuple{1, 2, 3} | std::views::take(1_c)) == std::tuple{1});
  static_assert((std::tuple{1, 2, 3} | std::views::take(2_c)) == std::tuple{1, 2});
  // tuple.drop
  static_assert((std::tuple{1, 2, 3} | std::views::drop(1_c)) == std::tuple{2, 3});
  static_assert((std::tuple{1, 2, 3} | std::views::drop(2_c)) == std::tuple{3});

  // tuple.take.drop
  static_assert((std::tuple{1, 2, 3} | std::views::drop(1_c) | std::views::take(1_c)) == std::tuple{2});

  // type_list<Ts...>
  static_assert((mp::type_list{} | []<class... Ts> { return mp::type_list<Ts...>{}; }) == mp::type_list{});
  static_assert((mp::list<int, double>() | []<class... Ts> { return mp::type_list<Ts...>{}; }) == mp::list<int, double>());

  // type_list.(types)
  static_assert((mp::type_list{} | [](std::ranges::range auto types) { return types; }) == mp::type_list{});
  static_assert((mp::list<int, double>() | [](std::ranges::range auto types) { return types; }) == mp::list<int, double>());

  // type_list<Ts...>(types)
  static_assert((mp::type_list{} | []<class... Ts>(std::ranges::range auto types) { return types; }) == mp::type_list{});
  static_assert((mp::list<int, double>() | []<class... Ts>(std::ranges::range auto types) { return types; }) == mp::list<int, double>());

  // type_list.ranges
  static_assert((mp::type_list{} | std::views::reverse) == mp::type_list{});
  static_assert((mp::list<int, double>() | std::views::reverse) == mp::list<double, int>());

  // utility.nth_pack_element
  static_assert(std::is_same_v<int, mp::utility::nth_pack_element<0, int, void, double>>);
  static_assert(std::is_same_v<void, mp::utility::nth_pack_element<1, int, void, double>>);
  static_assert(std::is_same_v<double, mp::utility::nth_pack_element<2, int, void, double>>);
  static_assert(1 == mp::utility::nth_pack_element_v<0, 1, 2, 3>);
  static_assert(2 == mp::utility::nth_pack_element_v<1, 1, 2, 3>);
  static_assert(3 == mp::utility::nth_pack_element_v<2, 1, 2, 3>);

  // utility<char...>_c
  static_assert(0_c == 0);
  static_assert(42_c == 42);
  static_assert(1000000_c == 1'000'000);
  static_assert(1_c + 2_c == 3);
  static_assert(2_c - 1_c == 1);

  // value_list<Vs...>
  static_assert((mp::value_list{} | []<auto... Vs> { return mp::value_list<Vs...>{}; }) == mp::value_list{});
  static_assert((mp::list<1, 2>() | []<auto... Vs> { return mp::value_list<Vs...>{}; }) == mp::list<1, 2>());

  // value_list(types)
  static_assert((mp::value_list{} | [](std::ranges::range auto types) { return types; }) == mp::value_list{});
  static_assert((mp::list<1, 2>() | [](std::ranges::range auto types) { return types; }) == mp::list<1, 2>());

  // value_list<Vs...>(types)
  static_assert((mp::value_list{} | []<auto... Vs>(std::ranges::range auto types) { return types; }) == mp::value_list{});
  static_assert((mp::list<1, 2>() | []<auto... Vs>(std::ranges::range auto types) { return types; }) == mp::list<1, 2>());

  // value_list.compund
  auto sort = []<auto... Vs>(std::ranges::range auto types) {
    std::sort(std::begin(types), std::end(types), [values = std::array{Vs...}](auto lhs, auto rhs) { return values[lhs] < values[rhs]; });
    return types;
  };

  auto unique = []<auto... Vs>(std::ranges::range auto types) {
    types.erase(std::unique(std::begin(types), std::end(types), [values = std::array{Vs...}](auto lhs, auto rhs) { return values[lhs] == values[rhs]; }), std::end(types));
    return types;
  };

  static_assert((mp::list<1, 2>() | sort | unique) == mp::list<1, 2>());
  static_assert((mp::list<1, 2, 2>() | sort | unique) == mp::list<1, 2>());
  static_assert((mp::list<1, 1, 2, 2>() | sort | unique) == mp::list<1, 2>());
  static_assert((mp::list<1, 2, 1, 2>() | sort | unique) == mp::list<1, 2>());
  static_assert((mp::list<1, 2, 3>() | sort | unique) == mp::list<1, 2, 3>());
  static_assert((mp::list<1, 3, 2>() | sort | unique) == mp::list<1, 2, 3>());
  static_assert((mp::list<2, 2, 1, 1, 3>() | sort | unique) == mp::list<1, 2, 3>());
}(), true));
#endif
#endif // MP
