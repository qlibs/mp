//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#if defined(__cpp_modules) && !defined(BOOST_UT_DISABLE_MODULE)
export module boost.mpl;
export import std;
#else
#pragma once
#endif

#include <array>
#include <concepts>
#include <string_view>
#include <utility>
#include <vector>

#define BOOST_MP_VERSION 0'0'1

namespace boost::mp::inline v0_0_1 {
namespace detail {
template <auto Value>
struct value_type {
  static constexpr auto value = Value;
};
template <auto N, auto... Ns>
constexpr auto nth_pack_element =
    __type_pack_element<N, value_type<Ns>...>::value;

template <class T>
constexpr auto type_id() {
  std::size_t result{};
  for (const auto& c : __PRETTY_FUNCTION__) {
    (result ^= c) <<= 1;
  }
  return result;
}
}  // namespace detail

namespace concepts {
namespace detail {
template <class T>
concept meta = requires(T t) {
                 std::size_t{t.id};
                 std::size_t{t.type};
                 std::size_t{t.size};
               };
}

template <class T>
concept meta = std::ranges::random_access_range<T> and
               detail::meta<typename T::value_type>;
}  // namespace concepts

template <class T>
constexpr auto type_id = detail::type_id<T>();

struct meta {
  std::size_t id{};
  std::size_t type{};
  std::size_t size{};

  [[nodiscard]] constexpr operator auto() const { return id; }
  [[nodiscard]] constexpr auto operator==(const meta&) const -> bool = default;
};

template <class... Ts>
struct type_list {
  static constexpr auto size = sizeof...(Ts);
  constexpr auto operator==(type_list<Ts...>) const { return true; }
  template<class... Us>
  constexpr auto operator==(type_list<Us...>) const { return false; }
};

template <auto... Vs>
struct value_list {
  static constexpr auto size = sizeof...(Vs);
  constexpr auto operator==(value_list<Vs...>) const { return true; }
  template<auto... Us>
  constexpr auto operator==(value_list<Us...>) const { return false; }
};

template <class... Ts>
[[nodiscard]] constexpr auto operator|(type_list<Ts...> lhs, auto fn) {
  if constexpr (requires { fn.template operator()<Ts...>(); }) {
    return fn.template operator()<Ts...>();
  } else if constexpr (requires { fn(); }) {
    return [=]<auto... Ns>(std::index_sequence<Ns...>) {
      auto size = [=] {
        std::vector types{
            meta{.id = Ns, .type = type_id<Ts>, .size = sizeof(Ts)}...};
        const auto v = types | fn();
        std::vector<meta> vs{std::begin(v), std::end(v)};
        return std::size(vs);
      };
      return [=]<auto... Ids>(std::index_sequence<Ids...>) {
        auto f = [=] {
          std::vector types{meta{
              .id = Ns, .type = type_id<Ts>, .size = sizeof(Ts)}...};
          auto v = types | fn();
          std::vector<meta> vs{v.begin(), v.end()};
          return std::array<std::size_t, sizeof...(Ids)>{vs[Ids].id...};
        };
        return type_list<__type_pack_element<f()[Ids], Ts...>...>{};
      }(std::make_index_sequence<size()>{});
    }(std::make_index_sequence<sizeof...(Ts)>{});
  } else {
      auto size = [fn]<auto... Ns>(std::index_sequence<Ns...>) {
        std::vector<meta> types{
            meta{.id = Ns, .type = type_id<Ts>, .size = sizeof(Ts)}...};
        if constexpr (requires { fn.template operator()<Ts...>(types); }) {
          if constexpr (auto ts = fn.template operator()<Ts...>(types); requires { std::size(ts); }) {
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
        } else {
          return 2u;
        }
      };

      auto vs = [fn]<auto... Ids>(std::index_sequence<Ids...>) {
        auto i = 0uz;
        std::vector<meta> types{meta{.id = i++, .type = type_id<Ts>, .size = sizeof(Ts)}...};
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
          if constexpr (auto ts = fn(types); requires { ts[0]; }) {
            return std::array{ts[Ids].id...};
          } else {
            return std::array{ts.id};
          }
        }
      };

      constexpr auto out = std::make_index_sequence<size(std::make_index_sequence<sizeof...(Ts)>{})>{};
      constexpr auto vs_out = vs(out);

      return [vs_out]<auto... Ids>(std::index_sequence<Ids...>) {
        return type_list<__type_pack_element<vs_out[Ids], Ts...>...>{};
      }(out);
  }
}

template <auto N, class T>
constexpr auto get2(const T& t) {
  return [&] { return std::get<N>(t); };
}

template <class... Ts>
[[nodiscard]] constexpr auto operator|(std::tuple<Ts...> lhs, auto fn) {
  return [=]<auto... Ns>(std::index_sequence<Ns...>) {
    if constexpr (requires { fn(std::get<Ns>(lhs)...); }) {
      return fn(std::get<Ns>(lhs)...);
    } else {
      auto size = [=] {
        std::vector types{
            meta{.id = Ns, .type = type_id<Ts>, .size = sizeof(Ts)}...};

        if constexpr (requires { fn(types, get2<Ns>(lhs)...); }) {
          auto vs = fn(types, get2<Ns>(lhs)...);
          return std::size(vs);
        } else if constexpr (requires { fn(get2<Ns>(lhs)...); }) {
          return std::tuple_size_v<decltype(fn(get2<Ns>(lhs)...))>;
        } else if constexpr (requires { fn(types); }) {
          if constexpr (auto ts = fn(types); requires { std::size(ts); }) {
            return std::size(ts);

          } else {
            return 1uz;
          }
        } else {
          return 2u;
        }
      };
      return [=]<auto... Ids>(std::index_sequence<Ids...>) {
        auto vs = [=] {
          std::vector types{meta{
              .id = Ns, .type = type_id<Ts>, .size = sizeof(Ts)}...};

          if constexpr (requires { fn(types, get2<Ns>(lhs)...); }) {
            auto ts = fn(types, get2<Ns>(lhs)...);
            return std::array{ts[Ids].id...};
          } else if constexpr (requires { fn(get2<Ns>(lhs)...); }) {
            return std::array{Ids...};
          } else if constexpr (requires { fn(types); }) {
            if constexpr (auto ts = fn(types); requires { ts[0]; }) {
              return std::array{ts[Ids].id...};
            } else {
              return std::array{ts.id};
            }
          }
        };
        return std::tuple{std::get<vs()[Ids]>(lhs)...};
      }(std::make_index_sequence<size()>{});
    }
  }(std::make_index_sequence<sizeof...(Ts)>{});
}

template <auto... Ts>
[[nodiscard]] constexpr auto operator|(value_list<Ts...> lhs, auto fn) {
  if constexpr (requires { fn.template operator()<Ts...>(); }) {
    return fn.template operator()<Ts...>();
  } else if constexpr (requires { fn(); }) {
    return [=]<auto... Ns>(std::index_sequence<Ns...>) {
      auto sizes = [=] {
        std::vector types{meta{.id = Ns,
                               .type = type_id<decltype(Ts)>,
                               .size = sizeof(Ts)}...};
        auto v = types | fn();
        std::vector<meta> vs{v.begin(), v.end()};
        return std::size(vs);
      };
      return [=]<auto... Ids>(std::index_sequence<Ids...>) {
        auto f = [=] {
          std::vector types{meta{.id = Ns,
                                 .type = type_id<decltype(Ts)>,
                                 .size = sizeof(Ts)}...};
          auto v = types | fn();
          std::vector<meta> vs{v.begin(), v.end()};
          return std::array<std::size_t, sizeof...(Ids)>{vs[Ids].id...};
        };
        return value_list<detail::nth_pack_element<f()[Ids], Ts...>...>{};
      }(std::make_index_sequence<sizes()>{});
    }(std::make_index_sequence<sizeof...(Ts)>{});
  } else {
    return [=]<auto... Ns>(std::index_sequence<Ns...>) {
      auto size = [=] {
        std::vector types{meta{.id = Ns,
                               .type = type_id<decltype(Ts)>,
                               .size = sizeof(Ts)}...};
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
        } else {
          return 2u;
        }
      };
      return [=]<auto... Ids>(std::index_sequence<Ids...>) {
        auto vs = [=] {
          std::vector types{meta{.id = Ns,
                                 .type = type_id<decltype(Ts)>,
                                 .size = sizeof(Ts)}...};

          if constexpr (requires { fn.template operator()<Ts...>(types); }) {
            if constexpr (auto ts = fn.template operator()<Ts...>(types);
                          requires { ts[0]; }) {
              return std::array{ts[Ids].id...};
            } else {
              return std::array{ts.id};
            }
          } else if constexpr (requires { fn.template operator()<Ts...>(); }) {
            return std::array{Ids...};
          } else if constexpr (requires { fn(types); }) {
            if constexpr (auto ts = fn(types); requires { ts[0]; }) {
              return std::array{ts[Ids].id...};
            } else {
              return std::array{ts.id};
            }
          }
        };
        return value_list<detail::nth_pack_element<vs()[Ids], Ts...>...>{};
      }(std::make_index_sequence<size()>{});
    }(std::make_index_sequence<sizeof...(Ts)>{});
  }
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

constexpr auto to_tuple = [](auto&& obj) {
  if constexpr (requires { [&obj] { auto&& [p1, p2, p3] = obj; }; }) {
    auto&& [p1, p2, p3] = obj;
    return std::make_tuple(p1, p2, p3);
  } else if constexpr (requires { [&obj] { auto&& [p1, p2] = obj; }; }) {
    auto&& [p1, p2] = obj;
    return std::make_tuple(p1, p2);
  } else if constexpr (requires { [&obj] { auto&& [p1] = obj; }; }) {
    auto&& [p1] = obj;
    return std::make_tuple(p1);
  } else {
    return std::make_tuple();
  }
}();
}  // namespace boost::mp::inline v0_0_1
