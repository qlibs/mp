//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>
#include <boost/mp.hpp>
#include <cstdio>
#include <ranges>
#include <string_view>

namespace mp = boost::mp;

// clang-format off
template<class... Ts> struct pool : Ts...{
  constexpr explicit(true) pool(Ts... ts) : Ts{std::move(ts)}... {}
};

constexpr auto inherit = []<class... Ts> {
  struct : Ts... { } _;
  return _;
};

template<auto Fn>
constexpr auto unique = []<class... Ts>(auto types) {
  auto fns = Fn.template operator()<Ts...>();
  std::ranges::sort(types, [&](auto lhs, auto rhs) { return fns[lhs] < fns[rhs]; });
  auto [first, last] = std::ranges::unique(types, [&]( auto lhs, auto rhs) { return fns[lhs] == fns[rhs]; });
  types.erase(first, last);
  return types;
};

template<auto Fn>
constexpr auto unique_v = []<mp::fixed_string... Ts>(auto types) {
  auto fns = Fn.template operator()<Ts...>();
  std::ranges::sort(types, [&](auto lhs, auto rhs) { return fns[lhs] < fns[rhs]; });
  auto [first, last] = std::ranges::unique(types, [&]( auto lhs, auto rhs) { return fns[lhs] == fns[rhs]; });
  types.erase(first, last);
  return types;
};

constexpr auto name = [](std::string_view name) {
  return name[0] == '*' ? name.substr(1) : name;
};

constexpr auto by_type = []<class... TEvents> {
  return std::array{mp::reflection::type_id<TEvents>...};
};

constexpr auto by_name = []<mp::fixed_string... Names> {
  return std::array{name(Names)...};
};

namespace back {
template<class> class sm;
template<template<class...> class TList, class... Transitions>
class sm<TList<Transitions...>> {
  static constexpr auto transitions = mp::list<Transitions...>();
  static constexpr auto events = transitions
    | std::views::transform([]<class T>() -> typename T::event {})
    | unique<by_type>
    ;

  static constexpr auto states = transitions
    | []<class... Ts>() { return mp::value_list<Ts::src..., Ts::dst...>(); }
    | unique_v<by_name>
    ;

  template<mp::fixed_string State>
  static constexpr auto state_id = states
    | []<mp::fixed_string... States> {
      const auto states = std::array{name(States)...};
      return std::distance(std::begin(states), std::ranges::find(states, State));
    }
    ;

  static constexpr auto transition = []<class... Ts> {
    return [](const auto& event, auto& current_state, auto& transitions) {
      return (... or [&] {
        if (static_cast<Ts&>(transitions).execute(event)) {
          current_state = state_id<Ts::dst>;
          return true;
        }
        return false;
      }());
      return false;
    };
  };

  static constexpr auto mappings = events
    | std::views::transform([]<class TEvent> {
        return std::pair{std::type_identity<TEvent>{}, states
          | std::views::transform([]<mp::fixed_string State> {
              return transitions
                | std::views::filter([]<class T> {
                    return std::string_view{T::src} == State and
                           std::same_as<typename T::event, TEvent>; })
                | transition;
            })
        };
      })
    ;

 public:
  constexpr explicit(false) sm(const TList<Transitions...>& transition_table) : transition_table_{transition_table} {}

  template<class TEvent> constexpr auto process_event(const TEvent& event) -> void {
    [this, &event]<class T>(const std::pair<std::type_identity<TEvent>, T>& transitions) {
      auto& [_, ts] = transitions;
      mp::for_each([&](auto index, auto transition) {
        if (index == current_state_) {
          transition(event, current_state_, transition_table_);
          return true;
        }
        return false;
      }, ts);
    }(mappings | inherit);
  }

  template<mp::fixed_string State>
  [[nodiscard]] constexpr auto is() const {
    return state_id<State> == current_state_;
  }

 private:
  std::size_t current_state_{
    states | []<mp::fixed_string... States> {
      const auto states = std::array{std::string_view{States}[0]...};
      return std::distance(std::begin(states), std::ranges::find(states, '*'));
    }
  };
  [[no_unique_address]] TList<Transitions...> transition_table_{};
};
} // namespace back

namespace front {
struct none {};
template<mp::fixed_string Src = "", class TEvent = none, class TGuard = none, class TAction = none, mp::fixed_string Dst = "">
struct transition {
  static constexpr auto src = Src;
  static constexpr auto dst = Dst;
  using event = TEvent;

  [[no_unique_address]] TGuard guard;
  [[no_unique_address]] TAction action;

  template<class T>
  [[nodiscard]] constexpr auto operator+(const T& t) const {
    return transition<Src, typename T::event, decltype(T::guard), decltype(T::action)>{.guard = t.guard, .action = t.action};
  }

  template<class T>
  [[nodiscard]] constexpr auto operator/(const T& action) const {
    return transition<Src, TEvent, TGuard, T>{.guard = guard, .action = action};
  }

  template<class T>
  [[nodiscard]] constexpr auto operator=(const T&) const {
    return transition<src, TEvent, TGuard, TAction, T::src>{.guard = guard, .action = action};
  }

  [[nodiscard]] constexpr auto operator*() const {
    return transition<mp::fixed_string{"*"} + Src, TEvent, TGuard, TAction, Dst>{.guard = guard, .action = action};
  }

  [[nodiscard]] constexpr auto execute(const auto& event) -> bool {
    if (guard(event)) [[likely]] {
      action(event);
      return true;
    }
    return false;
  }
};

namespace detail {
template<class T> struct event {
  template<class TGuard>
  [[nodiscard]] constexpr auto operator[](const TGuard& guard) const {
    return transition<"", T, TGuard>{.guard = guard, .action = none{}};
  }
};
} // namespace detail

template<class T> constexpr auto event = detail::event<T>{};

template <mp::fixed_string Str> constexpr auto operator""_s() { return transition<Str>{}; }
} // namespace front

template<class... Ts> struct transition_table : pool<Ts...> {
  constexpr explicit(false) transition_table(Ts... ts) : pool<Ts...>{std::move(ts)...} {}
};

using front::event;
using front::operator""_s;

template<class Fn>
struct sm : back::sm<decltype(std::declval<Fn>()())> {
  constexpr explicit(false) sm(Fn fn) : back::sm<decltype(std::declval<Fn>()())>{fn()} {}
};

#include <cassert>

int main() {
  struct e {};

  auto transitions = [] {
    auto guard  = []([[maybe_unused]] const auto& event) { std::puts("guard"); return true; };
    auto action = []([[maybe_unused]] const auto& event) { std::puts("action"); };

    return transition_table{
     * "s1"_s + event<e> [ guard ] / action = "s2"_s,
       "s2"_s + event<e> [ guard ] / action = "s1"_s,
    };
  };

  sm sm{transitions};

  assert(sm.is<"s1">());
  sm.process_event(e{});
  assert(sm.is<"s2">());
  sm.process_event(e{});
  assert(sm.is<"s1">());
}
// clang-format on
