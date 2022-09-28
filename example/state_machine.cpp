//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>
#include <array>
#include <boost/mp.hpp>
#include <ranges>
#include <string_view>

#include <iostream>
template<class...> struct q;

template<class T, unsigned N>
struct reader {
    friend auto counted_flag(reader<T, N>);
};


template<class T, unsigned N>
struct setter {
    friend auto counted_flag(reader<T, N>) {}

    static constexpr unsigned n = N;
};


template<
    class T,
    auto Tag,
    unsigned NextVal = 0
>
[[nodiscard]]
consteval auto counter_impl() {
    constexpr bool counted_past_value = requires(reader<T, NextVal> r) {
        counted_flag(r);
    };

    if constexpr (counted_past_value) {
        return counter_impl<T, Tag, NextVal + 1>();
    }
    else {
        setter<T, NextVal> s;
        return s.n;
    }
}


template<
   class T,
    auto Tag = []{},
    auto Val = counter_impl<T, Tag>()
>
constexpr auto counter = Val;


// clang-format off
namespace mp = boost::mp;

template<class... Ts> struct pool : Ts... {
  constexpr explicit(true) pool(Ts... ts) : Ts{std::move(ts)}... {}
};

template<auto Fn>
constexpr auto unique = [] {
  constexpr auto impl = [](const auto& fns, auto types) {
    std::ranges::sort(types, [&](auto lhs, auto rhs) { return fns[lhs] < fns[rhs]; });
    auto [first, last] = std::ranges::unique(types, [&]( auto lhs, auto rhs) { return fns[lhs] == fns[rhs]; });
    types.erase(first, last);
    return types;
  };

  return mp::overloaded{
    [impl]<mp::fixed_string... Ts>(auto types) { return impl(Fn.template operator()<Ts...>(), types); },
    [impl]<class... Ts>           (auto types) { return impl(Fn.template operator()<Ts...>(), types); },
  };
}();

//constexpr auto by_type = []<class... TEvents>          { return std::array{mp::reflection::type_name<TEvents>()...}; };
constexpr auto by_name = []<mp::fixed_string... Names> { return std::array{std::string_view{Names}...}; };

template<int, class, class> struct foo {};

namespace back {
template<class> class sm;
template<template<class...> class TList, class... Transitions>
class sm<TList<Transitions...>> {
  static constexpr auto transitions = mp::list<Transitions...>();

  static constexpr auto mappings = transitions
    | []<class... Ts> {
        struct : foo<counter<typename Ts::event>, typename Ts::event, Ts>... { } _;
        return _;
      };

  static constexpr auto states = transitions
    | []<class... Ts>() { return mp::value_list<Ts::src..., Ts::dst...>(); }
    | std::views::filter([]<mp::fixed_string State> { return not std::empty(std::string_view(State)); })
    | unique<by_name>
    ;

  template<mp::fixed_string State>
  static constexpr auto state_id = states
    | []<mp::fixed_string... States> {
      constexpr auto name = [](const std::string_view name) {
        return not std::empty(name) and name[0] == '*' ? name.substr(1) : name;
      };
      constexpr auto states = std::array{name(States)...};
      return std::distance(std::begin(states), std::ranges::find(states, name(State)));
    }
    ;

 public:
  constexpr explicit(true) sm(const TList<Transitions...>& transition_table)
    : transition_table_{transition_table}
    , current_state_{states
        | std::views::filter([]<mp::fixed_string State>{ return std::string_view{State}[0] == '*'; } )
        | []<mp::fixed_string... States> { return std::array<std::size_t, sizeof...(States)>{state_id<States>...};  }
      }
  {  }

  constexpr auto process_event(const auto& event) -> void {
    process_event(event, std::make_index_sequence<num_of_regions>{});
  }

  [[nodiscard]] constexpr auto is(auto... states) const -> bool {
    auto i = 0;
    return ((state_id<states.src> == current_state_[i++]) and ...);
  }

 private:
  template<auto... Rs>
  constexpr auto process_event(const auto& event, std::index_sequence<Rs...>) -> void {
    (dispatch<Rs, 0>(event, &mappings), ...);
  }

  template<auto N, auto I, class TEvent, class T>
  constexpr auto dispatch(const TEvent& event, const foo<I, TEvent, T>*) -> void {
      if (state_id<T::src> == current_state_[N]) {
        if (not static_cast<T&>(transition_table_)(event, [&]<auto State> { current_state_[N] = state_id<State>; })) {
          dispatch<N, I + 1>(event, &mappings);
        }
      } else {
        dispatch<N, I + 1>(event, &mappings);
      }
  }

  template<auto...> constexpr auto dispatch(...) -> void { }

  static constexpr auto num_of_regions =
    states | []<mp::fixed_string... States> {
      return ((std::string_view{States}[0] == '*') + ...);
    };

  static_assert(num_of_regions > 0, "At least one region is required!");

  [[no_unique_address]] TList<Transitions...> transition_table_{};
  std::array<std::size_t, num_of_regions> current_state_{};
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
        class TEvent  = decltype(detail::none),
        class TGuard  = decltype(detail::always),
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

  [[nodiscard]] constexpr auto operator()(const TEvent& event, auto update_state) -> bool {
    if (invoke(guard, event)) {
      invoke(action, event);
      if constexpr (constexpr auto has_dst_state = not std::empty(std::string_view{dst}); has_dst_state) {
        update_state.template operator()<dst>();
      }
      return true;
    } else {
      return false;
    }
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

template<class T>
struct sm final : back::sm<decltype(std::declval<T>()())> {
  constexpr explicit(false) sm(T t) : back::sm<decltype(std::declval<T>()())>{t()} {}
};
template<class T> sm(T&&) -> sm<T>;

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
    constexpr auto close     = []{ std::puts("close"); };
    constexpr auto is_valid  = [](const auto& event) { return event.valid; };
    constexpr auto setup     = []{ std::puts("setup"); };

    using namespace dsl;
    /**
     * src_state + event [ guard ] / action = dst_state
     */
    return transition_table{
      * "Disconnected"_s + event<connect>           / establish   = "Connecting"_s,
        "Connecting"_s   + event<established>                     = "Connected"_s,
        "Connected"_s    + event<ping> [ is_valid ] / setup,
        "Connected"_s    + event<timeout>           / establish   = "Connecting"_s,
        "Connected"_s    + event<disconnect>        / close       = "Disconnected"_s,
    };
  }
};

#include <cassert>

int main() {
  using dsl::operator""_s;

  sm connection{Connection{}};
  assert(connection.is("Disconnected"_s));

  connection.process_event(connect{});
  assert(connection.is("Connecting"_s));

  connection.process_event(established{});
  assert(connection.is("Connected"_s));

  connection.process_event(ping{true});
  assert(connection.is("Connected"_s));

  connection.process_event(disconnect{});
  assert(connection.is("Disconnected"_s));
}
// clang-format on
