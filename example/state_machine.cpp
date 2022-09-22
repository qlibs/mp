//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>
#include <boost/mp.hpp>
#include <ranges>
#include <string_view>

namespace mp = boost::mp;

// clang-format off
template<class... Ts> struct pool : Ts... {
  constexpr explicit(true) pool(Ts... ts) : Ts{std::move(ts)}... {}
};

constexpr auto filter = [](auto pred) {
  return [pred]<class... Ts>(auto types) {
    const auto fns = std::array{pred.template operator()<Ts>()...};
    decltype(types) ret{};
    for (const auto& m : types) {
      if (fns[m]) {
        ret.push_back(m);
      }
    }
    return ret;
  };
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

constexpr auto name = [](const std::string_view name) {
  return name[0] == '*' ? name.substr(1) : name;
};

constexpr auto by_type = []<class... TEvents> {
  return std::array{mp::reflection::type_name<TEvents>()...};
};

constexpr auto by_name = []<mp::fixed_string... Names> {
  return std::array{std::string_view{Names}...};
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
    | std::views::filter([]<mp::fixed_string State> { return not std::empty(std::string_view(State)); })
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
        if (static_cast<Ts&>(transitions)(event)) {
          current_state = state_id<Ts::dst>;
          return true;
        }
        return false;
      }());
      return false;
    };
  };

  template<class TEvent>
  static constexpr auto event_transitions = transitions
    | std::views::filter([]<class T> { return std::same_as<typename T::event, TEvent>; });

 public:
  constexpr explicit(false) sm(const TList<Transitions...>& transition_table)
    : transition_table_{transition_table} {
      states | [&]<mp::fixed_string... States> {
        const auto states = std::array{std::string_view{States}...};
        auto* current_state = &current_state_[0];
        for (auto i = 0u; i < std::size(states); ++i) {
          if (states[i][0] == '*') {
            *current_state++ = i;
          }
        }
      };
  }

  template<class TEvent> constexpr auto process_event(const TEvent& event) -> void {
    process_event(event, std::make_index_sequence<num_of_regions>{});
  }

  template<mp::fixed_string... States>
  [[nodiscard]] constexpr auto is() const -> bool {
    auto i = 0;
    return ((state_id<States> == current_state_[i++]) and ...);
  }

 private:
  template<class TEvent, auto... Rs>
  constexpr auto process_event(const TEvent& event, std::index_sequence<Rs...>) -> void {
    constexpr auto transitions = states
      | std::views::transform([]<mp::fixed_string State> {
          return event_transitions<TEvent>
            | filter([]<class T> { return (std::string_view(T::src) == State); })
            | transition;
        });
    (dispatch<Rs>(event, transitions), ...);
  }

  template<auto N, class TEvent>
  constexpr auto dispatch(const TEvent& event, const auto& transitions) -> void {
      mp::for_each([&](const auto index, const auto& transition) {
        if (index == current_state_[N]) {
          transition(event, current_state_[N], transition_table_);
          return true;
        }
        return false;
      }, transitions);
  }

  static constexpr auto num_of_regions =
    states | []<mp::fixed_string... States> {
      return ((std::string_view{States}[0] == '*') + ...);
    };

  static_assert(num_of_regions > 0, "At least one region is required!");

  std::size_t current_state_[num_of_regions]{};
  [[no_unique_address]] TList<Transitions...> transition_table_{};
};
} // namespace back

namespace front {
namespace concepts {
struct invokable_base { void operator()(); };
template <class T> struct invokable_impl : T, invokable_base { };
template <class T> concept invokable = not requires { &invokable_impl<T>::operator(); };
} // namespace concepts

constexpr auto invoke(const concepts::invokable auto& fn, const auto& event) {
  if constexpr (requires { fn(event); }) {
    return fn(event);
  } else if constexpr (requires { fn(); }) {
    return fn();
  }
}

namespace detail {
constexpr auto none = []{};
constexpr auto always = []{ return true; };
} // namespace detail

template<mp::fixed_string Src = "",
        class TEvent = decltype(detail::none),
        class TGuard = decltype(detail::always),
        class TAction = decltype(detail::none),
        mp::fixed_string Dst = "">
struct transition {
  static constexpr auto src = Src;
  static constexpr auto dst = Dst;
  using event = TEvent;

  [[nodiscard]] constexpr auto operator*() const {
    return transition<mp::fixed_string{"*"} + Src, TEvent, TGuard, TAction, Dst>{.guard = guard, .action = action};
  }

  template<class T>
  [[nodiscard]] constexpr auto operator+(const T& t) const {
    return transition<Src, typename T::event, decltype(T::guard), decltype(T::action)>{.guard = t.guard, .action = t.action};
  }

  template<class T>
  [[nodiscard]] constexpr auto operator[](const T& guard) const {
    return transition<Src, TEvent, T>{.guard = guard, .action = action};
  }

  template<class T>
  [[nodiscard]] constexpr auto operator/(const T& action) const {
    return transition<Src, TEvent, TGuard, T>{.guard = guard, .action = action};
  }

  template<class T>
  [[nodiscard]] constexpr auto operator=(const T&) const {
    return transition<src, TEvent, TGuard, TAction, T::src>{.guard = guard, .action = action};
  }

  [[nodiscard]] constexpr auto operator()(const auto& event) -> bool {
    if (invoke(guard, event)) [[likely]] {
      invoke(action, event);
      return true;
    }
    return false;
  }

  [[no_unique_address]] TGuard guard;
  [[no_unique_address]] TAction action;
};

template<class TEvent> constexpr auto event = transition<"", TEvent>{};
template <mp::fixed_string Str> constexpr auto operator""_s() {
  static_assert(not std::empty(std::string_view(Str)), "State requires a name!");
  return transition<Str>{};
}

[[nodiscard]] constexpr auto operator,(const concepts::invokable auto& lhs, const concepts::invokable auto& rhs) {
  return [=](const auto& event) {
    invoke(lhs, event);
    invoke(rhs, event);
  };
}
[[nodiscard]] constexpr auto operator and(const concepts::invokable auto& lhs, const concepts::invokable auto& rhs) {
  return [=](const auto& event) {
    return invoke(lhs, event) and invoke(rhs, event);
  };
}
[[nodiscard]] constexpr auto operator or(const concepts::invokable auto& lhs, const concepts::invokable auto& rhs) {
  return [=](const auto& event) {
    return invoke(lhs, event) or invoke(rhs, event);
  };
}
[[nodiscard]] constexpr auto operator not(const concepts::invokable auto& t) {
  return [=](const auto& event) {
    return not invoke(t, event);
  };
}
} // namespace front

template<class Fn>
struct sm : back::sm<decltype(std::declval<Fn>()())> {
  constexpr explicit(false) sm(Fn fn) : back::sm<decltype(std::declval<Fn>()())>{fn()} {}
};
template<class Fn> sm(Fn&&) -> sm<Fn>;

namespace dsl {
template<class... Ts> struct transition_table : pool<Ts...> {
  constexpr explicit(false) transition_table(Ts... ts) : pool<Ts...>{std::move(ts)...} {}
};
using front::event;
using front::operator""_s;
using front::operator,;
using front::operator not;
using front::operator and;
using front::operator or;
} // namespace dsl

#include <cstdio>

struct connect {};
struct ping { bool valid = false; };
struct established {};
struct timeout {};
struct disconnect {};

struct Connection {
  constexpr auto operator()() const {
    constexpr auto establish = []{ std::puts("establish"); };
    constexpr auto close = []{ std::puts("close"); };
    constexpr auto is_valid = [](const auto& event) { return event.valid; };
    constexpr auto resetTimeout = [] { std::puts("resetTimeout"); };

    using namespace dsl;
    return transition_table{
      * "Disconnected"_s + event<connect> / establish               = "Connecting"_s,
        "Connecting"_s   + event<established>                       = "Connected"_s,
        "Connected"_s    + event<ping> [ is_valid ] / resetTimeout,
        "Connected"_s    + event<timeout> / establish               = "Connecting"_s,
        "Connected"_s    + event<disconnect> / close                = "Disconnected"_s,
    };
  }
};

int main() {
  sm connection{Connection{}};

  connection.process_event(connect{});
  connection.process_event(established{});
  connection.process_event(ping{true});
  connection.process_event(disconnect{});
}
