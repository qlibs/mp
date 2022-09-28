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

struct e1{};
struct e2{};
struct e3{};
struct e4{};
struct e5{};
struct e6{};
struct e7{};
struct e8{};
struct e9{};
struct e10{};
struct e11{};
struct e12{};
struct e13{};
struct e14{};
struct e15{};
struct e16{};
struct e17{};
struct e18{};
struct e19{};
struct e20{};
struct e21{};
struct e22{};
struct e23{};
struct e24{};
struct e25{};
struct e26{};
struct e27{};
struct e28{};
struct e29{};
struct e30{};
struct e31{};
struct e32{};
struct e33{};
struct e34{};
struct e35{};
struct e36{};
struct e37{};
struct e38{};
struct e39{};
struct e40{};
struct e41{};
struct e42{};
struct e43{};
struct e44{};
struct e45{};
struct e46{};
struct e47{};
struct e48{};
struct e49{};
struct e50{};

struct Connection {
  constexpr auto operator()() const {
    //constexpr auto establish = []{ std::puts("establish"); };
    //constexpr auto close     = []{ std::puts("close"); };
    //constexpr auto is_valid  = [](const auto& event) { return event.valid; };
    //constexpr auto setup     = []{ std::puts("setup"); };

    constexpr auto guard = [] { return true; };
    constexpr auto action = [] {};

    using namespace dsl;
    /**
     * src_state + event [ guard ] / action = dst_state
     */
    return transition_table{
      //* "Disconnected"_s + event<connect>           / establish   = "Connecting"_s,
        //"Connecting"_s   + event<established>                     = "Connected"_s,
        //"Connected"_s    + event<ping> [ is_valid ] / setup,
        //"Connected"_s    + event<timeout>           / establish   = "Connecting"_s,
        //"Connected"_s    + event<disconnect>        / close       = "Disconnected"_s,
      *"s1"_s + event<e1> [ guard ] / action = "s2"_s,
      "s2"_s + event<e2> [ guard ] / action = "s3"_s,
      "s3"_s + event<e3> [ guard ] / action = "s4"_s,
      "s4"_s + event<e4> [ guard ] / action = "s5"_s,
      "s5"_s + event<e5> [ guard ] / action = "s6"_s,
      "s6"_s + event<e6> [ guard ] / action = "s7"_s,
      "s7"_s + event<e7> [ guard ] / action = "s8"_s,
      "s8"_s + event<e8> [ guard ] / action = "s9"_s,
      "s9"_s + event<e9> [ guard ] / action = "s10"_s,
      "s10"_s + event<e10> [ guard ] / action = "s11"_s,
      "s11"_s + event<e11> [ guard ] / action = "s12"_s,
      "s12"_s + event<e12> [ guard ] / action = "s13"_s,
      "s13"_s + event<e13> [ guard ] / action = "s14"_s,
      "s14"_s + event<e14> [ guard ] / action = "s15"_s,
      "s15"_s + event<e15> [ guard ] / action = "s16"_s,
      "s16"_s + event<e16> [ guard ] / action = "s17"_s,
      "s17"_s + event<e17> [ guard ] / action = "s18"_s,
      "s18"_s + event<e18> [ guard ] / action = "s19"_s,
      "s19"_s + event<e19> [ guard ] / action = "s20"_s,
      "s20"_s + event<e20> [ guard ] / action = "s21"_s,
      "s21"_s + event<e21> [ guard ] / action = "s22"_s,
      "s22"_s + event<e22> [ guard ] / action = "s23"_s,
      "s23"_s + event<e23> [ guard ] / action = "s24"_s,
      "s24"_s + event<e24> [ guard ] / action = "s25"_s,
      "s25"_s + event<e25> [ guard ] / action = "s26"_s,
      "s26"_s + event<e26> [ guard ] / action = "s27"_s,
      "s27"_s + event<e27> [ guard ] / action = "s28"_s,
      "s28"_s + event<e28> [ guard ] / action = "s29"_s,
      "s29"_s + event<e29> [ guard ] / action = "s30"_s,
      "s30"_s + event<e30> [ guard ] / action = "s31"_s,
      "s31"_s + event<e31> [ guard ] / action = "s32"_s,
      "s32"_s + event<e32> [ guard ] / action = "s33"_s,
      "s33"_s + event<e33> [ guard ] / action = "s34"_s,
      "s34"_s + event<e34> [ guard ] / action = "s35"_s,
      "s35"_s + event<e35> [ guard ] / action = "s36"_s,
      "s36"_s + event<e36> [ guard ] / action = "s37"_s,
      "s37"_s + event<e37> [ guard ] / action = "s38"_s,
      "s38"_s + event<e38> [ guard ] / action = "s39"_s,
      "s39"_s + event<e39> [ guard ] / action = "s40"_s,
      "s40"_s + event<e40> [ guard ] / action = "s41"_s,
      "s41"_s + event<e41> [ guard ] / action = "s42"_s,
      "s42"_s + event<e42> [ guard ] / action = "s43"_s,
      "s43"_s + event<e43> [ guard ] / action = "s44"_s,
      "s44"_s + event<e44> [ guard ] / action = "s45"_s,
      "s45"_s + event<e45> [ guard ] / action = "s46"_s,
      "s46"_s + event<e46> [ guard ] / action = "s47"_s,
      "s47"_s + event<e47> [ guard ] / action = "s48"_s,
      "s48"_s + event<e48> [ guard ] / action = "s49"_s,
      "s49"_s + event<e49> [ guard ] / action = "s50"_s,
      "s50"_s + event<e50> [ guard ] / action = "s51"_s,
    };
  }
};

#include <cassert>

int main() {
  using dsl::operator""_s;

  sm sm{Connection{}};
  sm.process_event(e1{});
  sm.process_event(e2{});
  sm.process_event(e3{});
  sm.process_event(e4{});
  sm.process_event(e5{});
  sm.process_event(e6{});
  sm.process_event(e7{});
  sm.process_event(e8{});
  sm.process_event(e9{});
  sm.process_event(e10{});
  sm.process_event(e11{});
  sm.process_event(e12{});
  sm.process_event(e13{});
  sm.process_event(e14{});
  sm.process_event(e15{});
  sm.process_event(e16{});
  sm.process_event(e17{});
  sm.process_event(e18{});
  sm.process_event(e19{});
  sm.process_event(e20{});
  sm.process_event(e21{});
  sm.process_event(e22{});
  sm.process_event(e23{});
  sm.process_event(e24{});
  sm.process_event(e25{});
  sm.process_event(e26{});
  sm.process_event(e27{});
  sm.process_event(e28{});
  sm.process_event(e29{});
  sm.process_event(e30{});
  sm.process_event(e31{});
  sm.process_event(e32{});
  sm.process_event(e33{});
  sm.process_event(e34{});
  sm.process_event(e35{});
  sm.process_event(e36{});
  sm.process_event(e37{});
  sm.process_event(e38{});
  sm.process_event(e39{});
  sm.process_event(e40{});
  sm.process_event(e41{});
  sm.process_event(e42{});
  sm.process_event(e43{});
  sm.process_event(e44{});
  sm.process_event(e45{});
  sm.process_event(e46{});
  sm.process_event(e47{});
  sm.process_event(e48{});
  sm.process_event(e49{});
  sm.process_event(e50{});

  //sm connection{Connection{}};
  //assert(connection.is("Disconnected"_s));

  //connection.process_event(connect{});
  //assert(connection.is("Connecting"_s));

  //connection.process_event(established{});
  //assert(connection.is("Connected"_s));

  //connection.process_event(ping{true});
  //assert(connection.is("Connected"_s));

  //connection.process_event(disconnect{});
  //assert(connection.is("Disconnected"_s));
}
// clang-format on
