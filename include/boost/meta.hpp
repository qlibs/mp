//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <array>
#include <utility>
#include <vector>

namespace meta::inline v1 {
namespace detail {
[[nodiscard]] constexpr unsigned const_hash(char const *input) {
  return *input ? static_cast<unsigned int>(*input) + 33 * const_hash(input + 1)
                : 5381;
}

template <class T> constexpr auto type_id() {
  return const_hash(__PRETTY_FUNCTION__);
}
} // namespace detail

template <class...> struct list_t {};
template <class... Ts> constexpr list_t<Ts...> list{};

struct meta {
  std::size_t id{};
  std::size_t type{};
};

template <class T> constexpr auto type_id = detail::type_id<T>();

template <auto, class> struct apply_t;

template <auto F, template <class...> class TList, class... Ts>
struct apply_t<F, TList<Ts...>> {
  using type = decltype([]<auto... Ns>(std::index_sequence<Ns...>) {
    auto size = [] {
      std::vector types{meta{Ns, type_id<Ts>}...};
      if constexpr (auto ts = F.template operator()<Ts...>(types);
                    requires { std::size(ts); }) {
        return std::size(ts);
      } else {
        return 1uz;
      }
    };
    return []<auto... Ids>(std::index_sequence<Ids...>) {
      auto fn = [] {
        std::vector types{meta{Ns, type_id<Ts>}...};
        if constexpr (auto ts = F.template operator()<Ts...>(types);
                      requires { ts[0]; }) {
          return std::array{ts[Ids].id...};
        } else {
          return std::array{ts.id};
        }
      };
      return TList<__type_pack_element<fn()[Ids], Ts...>...>{};
    }
    (std::make_index_sequence<size()>{});
  }(std::make_index_sequence<sizeof...(Ts)>{}));
};

template <auto F, class T> typename apply_t<F, T>::type apply{};

template <class... Ts>
[[nodiscard]] constexpr auto operator|(list_t<Ts...> lhs, auto rhs) {
  return apply<rhs, list_t<Ts...>>;
}
} // namespace meta::v1

