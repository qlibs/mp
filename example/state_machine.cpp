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

constexpr auto by_type = []<class... TEvents> {
  return std::array{mp::reflection::type_id<TEvents>...};
};

constexpr auto by_name = []<mp::fixed_string... Names> {
  return std::array{std::string_view{Names}...};
};

namespace back {
template<class TransitionTable>
class sm {
  static constexpr auto transitions = TransitionTable{};
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
      const auto states = std::array{std::string_view{States}...};
      return std::distance(std::begin(states), std::ranges::find(states, State));
    }
    ;
  static constexpr auto transition = []<class... Ts> {
    return [](const auto& event, auto& current_state) {
      return (... or [&] {
        if (Ts{}.execute(event)) {
          current_state = state_id<Ts::dst>;
          return true;
        }
        return false;
      }());
    };
  };
  static constexpr auto mappings = events
    | std::views::transform([]<class TEvent> {
        return std::pair{TEvent{}, states
          | std::views::transform([]<mp::fixed_string State> {
              return transitions
                | std::views::filter([]<class T> { return T::src == State and std::same_as<typename T::event, TEvent>; })
                | transition;
            })
        };
      })
    ;

 public:
  constexpr explicit(false) sm(TransitionTable) {}

  template<class TEvent> constexpr auto process_event(const TEvent& event) -> void {
    [this, &event]<class T>(const std::pair<TEvent, T>& transitions) {
      auto& [_, ts] = transitions;
      mp::for_each([&](auto index, auto transition) {
        if (index == current_state_) {
          transition(event, current_state_);
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
  std::size_t current_state_{};
};
} // namespace back

namespace front {
struct none {};
template<mp::fixed_string Src = "", class TEvent = none, class TGuard = none, class TAction = none, mp::fixed_string Dst = "">
struct transition {
  static constexpr auto src = Src;
  static constexpr auto dst = Dst;
  using event = TEvent;

  TGuard guard;
  TAction action;

  template<class T>
  [[nodiscard]] constexpr auto operator+(T) const {
    return transition<Src, typename T::event, decltype(T::guard), decltype(T::action)>{};
  }

  template<class T>
  [[nodiscard]] constexpr auto operator/(T) const {
    return transition<Src, TEvent, TGuard, T>{};
  }

  template<class T>
  [[nodiscard]] constexpr auto operator=(T) const {
    return transition<src, TEvent, TGuard, TAction, T::src>{};
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
  [[nodiscard]] constexpr auto operator[](TGuard) const {
    return transition<"", T, TGuard>{};
  }
};
}

template<class T> constexpr auto event = detail::event<T>{};

template <mp::fixed_string Str> constexpr auto operator""_s() { return transition<Str>{}; }
} // namespace front

template<class... Ts> [[nodiscard]] constexpr auto transition_table(Ts...) -> mp::type_list<Ts...> { return {}; };
using back::sm;
using front::event;
using front::operator""_s;

#include <cassert>

int main() {
  struct e {};
  auto guard  = []([[maybe_unused]] const auto& event) { std::puts("guard"); return true; };
  auto action = []([[maybe_unused]] const auto& event) { std::puts("action"); };

  sm sm{
    transition_table(
      "s1"_s + event<e> [ guard ] / action = "s2"_s,
      "s2"_s + event<e> [ guard ] / action = "s1"_s
    )
  };

  assert(sm.is<"s1">());
  sm.process_event(e{});
  assert(sm.is<"s2">());
  sm.process_event(e{});
  assert(sm.is<"s1">());
}
// clang-format on
