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
#include <concepts>
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

template <auto N>
static constexpr std::integral_constant<decltype(N), N> ct{};

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

template <class... Ts>
[[nodiscard]] constexpr auto list() {
  return type_list<Ts...>{};
}

template <auto... Vs>
[[nodiscard]] constexpr auto list() {
  return value_list<Vs...>{};
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

template <class... Ts>
[[nodiscard]] constexpr auto operator|(type_list<Ts...>, auto fn) {
  if constexpr (requires { fn.template operator()<Ts...>(); }) {
    return fn.template operator()<Ts...>();
  } else {
    auto size = [fn] {
      auto id = 0uz;
      const std::vector<meta> types{meta{.id = id++, .size = sizeof(Ts)}...};
      if constexpr (requires { fn.template operator()<Ts...>(types); }) {
        if constexpr (auto ts = fn.template operator()<Ts...>(types);
                      requires { std::size(ts); }) {
          return std::size(ts);
        } else {
          return 1uz;
        }
      } else if constexpr (requires { fn.template operator()<Ts...>(); }) {
        return fn.template operator()<Ts...>().size;
      } else if constexpr (requires { fn(types); }) {
        if constexpr (auto ts = fn(types); requires { std::size(ts); }) {
          return std::size(ts);
        } else {
          return 1uz;
        }
      }
    };
    auto vs = [fn]<auto... Ids>(std::index_sequence<Ids...>) {
      auto id = 0uz;
      const std::vector<meta> types{meta{.id = id++, .size = sizeof(Ts)}...};
      if constexpr (requires { fn.template operator()<Ts...>(types); }) {
        if constexpr (auto ts = fn.template operator()<Ts...>(types);
                      requires { ts[0]; }) {
          return std::array{ts[Ids].id...};
        } else if constexpr (requires { ts.id; }) {
          return std::array{ts.id};
        }
      } else if constexpr (requires { fn.template operator()<Ts...>(); }) {
        return std::array{Ids...};
      } else if constexpr (requires { fn(types); }) {
        if constexpr (auto ts = fn(types); requires { ts[0].id; }) {
          return std::array{ts[Ids].id...};
        } else {
          return std::array{ts.id};
        }
      }
    };

    constexpr auto out = std::make_index_sequence<size()>{};
    constexpr auto vs_out = vs(out);

    return [vs_out]<auto... Ids>(std::index_sequence<Ids...>) {
      return type_list<utility::nth_pack_element<vs_out[Ids], Ts...>...>{};
    }(out);
  }
}

template <auto... Ts>
[[nodiscard]] constexpr auto operator|(value_list<Ts...> lhs, auto fn) {
  if constexpr (requires { fn.template operator()<Ts...>(); }) {
    return fn.template operator()<Ts...>();
  } else {
    auto size = [fn] {
      auto id = 0uz;
      const std::vector<meta> types{meta{.id = id++, .size = sizeof(Ts)}...};
      if constexpr (requires { fn.template operator()<Ts...>(types); }) {
        if constexpr (auto ts = fn.template operator()<Ts...>(types);
                      requires { std::size(ts); }) {
          return std::size(ts);
        } else {
          return 1uz;
        }
      } else if constexpr (requires { fn.template operator()<Ts...>(); }) {
        return fn.template operator()<Ts...>().size;
      } else if constexpr (requires { fn(types); }) {
        if constexpr (auto ts = fn(types); requires { std::size(ts); }) {
          return std::size(ts);
        } else {
          return 1uz;
        }
      }
    };
    auto vs = [fn]<auto... Ids>(std::index_sequence<Ids...>) {
      auto id = 0uz;
      const std::vector<meta> types{meta{.id = id++, .size = sizeof(Ts)}...};
      if constexpr (requires { fn.template operator()<Ts...>(types); }) {
        if constexpr (auto ts = fn.template operator()<Ts...>(types);
                      requires { ts[0]; }) {
          return std::array{ts[Ids].id...};
        } else if constexpr (requires { ts.id; }) {
          return std::array{ts.id};
        }
      } else if constexpr (requires { fn.template operator()<Ts...>(); }) {
        return std::array{Ids...};
      } else if constexpr (requires { fn(types); }) {
        if constexpr (auto ts = fn(types); requires { ts[0].id; }) {
          return std::array{ts[Ids].id...};
        } else {
          return std::array{ts.id};
        }
      }
    };

    constexpr auto out = std::make_index_sequence<size()>{};
    constexpr auto vs_out = vs(out);

    return [vs_out]<auto... Ids>(std::index_sequence<Ids...>) {
      return value_list<utility::nth_pack_element_v<vs_out[Ids], Ts...>...>{};
    }(out);
  }
}

template <class... Ts>
[[nodiscard]] constexpr auto operator|(std::tuple<Ts...> t, auto fn) {
  if constexpr (requires { fn.template operator()<Ts...>(); }) {
    return fn.template operator()<Ts...>();
  } else {
    auto size = [fn] {
      auto id = 0uz;
      const std::vector<meta> types{meta{.id = id++, .size = sizeof(Ts)}...};
      if constexpr (requires { fn.template operator()<Ts...>(types); }) {
        if constexpr (auto ts = fn.template operator()<Ts...>(types);
                      requires { std::size(ts); }) {
          return std::size(ts);
        } else {
          return 1uz;
        }
      } else if constexpr (requires { fn.template operator()<Ts...>(); }) {
        return fn.template operator()<Ts...>().size;
      } else if constexpr (requires { fn(types); }) {
        if constexpr (auto ts = fn(types); requires { std::size(ts); }) {
          return std::size(ts);
        } else {
          return 1uz;
        }
      }
    };
    auto vs = [fn]<auto... Ids>(std::index_sequence<Ids...>) {
      auto id = 0uz;
      const std::vector<meta> types{meta{.id = id++, .size = sizeof(Ts)}...};
      if constexpr (requires { fn.template operator()<Ts...>(types); }) {
        if constexpr (auto ts = fn.template operator()<Ts...>(types);
                      requires { ts[0]; }) {
          return std::array{ts[Ids].id...};
        } else if constexpr (requires { ts.id; }) {
          return std::array{ts.id};
        }
      } else if constexpr (requires { fn.template operator()<Ts...>(); }) {
        return std::array{Ids...};
      } else if constexpr (requires { fn(types); }) {
        if constexpr (auto ts = fn(types); requires { ts[0].id; }) {
          return std::array{ts[Ids].id...};
        } else {
          return std::array{ts.id};
        }
      }
    };

    constexpr auto out = std::make_index_sequence<size()>{};
    constexpr auto vs_out = vs(out);

    return [t, vs_out]<auto... Ids>(std::index_sequence<Ids...>) {
      return std::tuple{std::get<vs_out[Ids]>(t)...};
    }(out);
  }
}
}  // namespace boost::mp::inline v0_0_1
