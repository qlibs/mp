//
// Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#if defined(GNU) and not defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif

namespace boost::inline v_2_0_0 {
namespace mp {
template <int...>
struct index_sequence {
  using type = index_sequence;
};
#if __has_builtin(__make_integer_seq)
template <class T, T...>
struct integer_sequence;
template <int... Ns>
struct integer_sequence<int, Ns...> {
  using type = index_sequence<Ns...>;
};
template <int N>
struct make_index_sequence_impl {
  using type = typename __make_integer_seq<integer_sequence, int, N>::type;
};
#else
template <class, class>
struct concat;
template <int... I1, int... I2>
struct concat<index_sequence<I1...>, index_sequence<I2...>> : index_sequence<I1..., (sizeof...(I1) + I2)...> {};
template <int N>
struct make_index_sequence_impl
    : concat<typename make_index_sequence_impl<N / 2>::type, typename make_index_sequence_impl<N - N / 2>::type>::type {};
template <>
struct make_index_sequence_impl<0> : index_sequence<> {};
template <>
struct make_index_sequence_impl<1> : index_sequence<0> {};
#endif
template <int N>
using make_index_sequence = typename make_index_sequence_impl<N>::type;

template <auto N>
struct fixed_string {
  static constexpr auto size = N;

  constexpr fixed_string() = default;
  constexpr explicit(false) fixed_string(const char (&str)[N + 1]) {
    for (auto i = 0u; i < N; ++i) {
      data[i] = str[i];
     (hash ^= data[i]) <<= 1;
    }
  }

  constexpr auto operator*() const {
    fixed_string<N + 1> str{};
    str.data[0] = '*';
    for (auto i = 0u; i < N; ++i) {
      str.data[i+1] = data[i];
      (str.hash ^= str.data[i+1]) <<= 1;
    }
    return str;
  }

  char data[N + 1]{};
  unsigned int hash{};
};

template <auto N>
fixed_string(const char (&str)[N]) -> fixed_string<N - 1>;

template<class T, auto N>
struct reader { friend auto counted_flag(reader<T, N>); };

template<class T, auto N>
struct setter {
    friend auto counted_flag(reader<T, N>) {}
    static constexpr unsigned value = N;
};

template< class T, auto Tag, unsigned NextVal = 0 >
[[nodiscard]] consteval auto counter_impl() {
    if constexpr (requires(reader<T, NextVal> r) { counted_flag(r); }) {
        return counter_impl<T, Tag, NextVal + 1>();
    }
    else {
        return setter<T, NextVal>::value;
    }
}

template<class T, auto Tag = []{}, auto N = counter_impl<T, Tag>()>
constexpr auto counter = N;

template<class... Ts> struct inherit : Ts... {};

template<class T> auto declval() -> T&&;
} // namespace mp

namespace back {
template<class... Ts> struct pool : Ts... {
  constexpr explicit(true) pool(Ts... ts) : Ts{ts}... {}
};

template<unsigned int, class...> struct transition {};

template<class> class sm;
template<template<class...> class TList, class... Transitions>
class sm<TList<Transitions...>> {
  using mappings_t = mp::inherit<transition<mp::counter<typename Transitions::event>, typename Transitions::event, Transitions>...>;

  static constexpr mappings_t mappings{};
  static constexpr auto num_of_regions = ((Transitions::src.data[0] == '*') + ...);

  static_assert(num_of_regions > 0, "At least one region is required!");

 public:
  constexpr explicit(true) sm(const TList<Transitions...>& transition_table)
    : transition_table_{transition_table} {
    const unsigned int states[]{(Transitions::src.data[0] == '*' ? Transitions::src.hash : 0u)...};
    for (auto* current_state = &current_state_[0]; auto state : states) {
      if (state) {
        *current_state++ = state;
      }
    }
  }

  constexpr auto process_event(const auto& event) -> void {
    process_event(event, mp::make_index_sequence<1>{});
  }

 private:
  template<auto... Rs, class TEvent>
  constexpr auto process_event(const TEvent& event, mp::index_sequence<Rs...>) -> void {
    (dispatch<TEvent>(event, current_state_[Rs], &mappings), ...);
  }

  template<class TEvent, unsigned int N = 0u, class T>
  constexpr auto dispatch(const TEvent& event, auto& current_state, const transition<N, TEvent, T>*) -> void {
    if (T::src.hash == current_state) {
      if (not static_cast<T&>(transition_table_)(event, current_state)) {
        dispatch<TEvent, N + 1u>(event, current_state, &mappings);
      }
    } else {
      dispatch<TEvent, N + 1u>(event, current_state, &mappings);
    }
  }

  template<class TEvent, unsigned int = 0u> constexpr auto dispatch(const TEvent&, auto&, ...) -> void { }

  [[no_unique_address]] TList<Transitions...> transition_table_{};
  unsigned int current_state_[num_of_regions]{};
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
    return transition<*Src, TEvent, TGuard, TAction, Dst>{.guard = guard, .action = action};
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

  [[nodiscard]] constexpr auto operator()(const TEvent& event, [[maybe_unused]] auto& current_state) -> bool {
    if (invoke(guard, event)) {
      invoke(action, event);
      if constexpr (dst.size) {
        current_state = dst.hash;
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
struct sm final : back::sm<decltype(mp::declval<T>()())> {
  constexpr explicit(false) sm(T t) : back::sm<decltype(mp::declval<T>()())>{t()} {}
};
template<class T> sm(T&&) -> sm<T>;

namespace dsl {
template<class... Ts> struct transition_table : back::pool<Ts...> {
  constexpr explicit(false) transition_table(Ts... ts) : back::pool<Ts...>{ts...} {}
};
using front::event;
using front::operator""_s;
using front::operator,;
using front::operator not;
using front::operator and;
using front::operator or;
} // namespace dsl
} // namespace boost

#if defined(GNU) and not defined(__clang__)
#pragma GCC diagnostic push
#endif

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
struct e51{};
struct e52{};
struct e53{};
struct e54{};
struct e55{};
struct e56{};
struct e57{};
struct e58{};
struct e59{};
struct e60{};
struct e61{};
struct e62{};
struct e63{};
struct e64{};
struct e65{};
struct e66{};
struct e67{};
struct e68{};
struct e69{};
struct e70{};
struct e71{};
struct e72{};
struct e73{};
struct e74{};
struct e75{};
struct e76{};
struct e77{};
struct e78{};
struct e79{};
struct e80{};
struct e81{};
struct e82{};
struct e83{};
struct e84{};
struct e85{};
struct e86{};
struct e87{};
struct e88{};
struct e89{};
struct e90{};
struct e91{};
struct e92{};
struct e93{};
struct e94{};
struct e95{};
struct e96{};
struct e97{};
struct e98{};
struct e99{};
struct e100{};
struct e101{};
struct e102{};
struct e103{};
struct e104{};
struct e105{};
struct e106{};
struct e107{};
struct e108{};
struct e109{};
struct e110{};
struct e111{};
struct e112{};
struct e113{};
struct e114{};
struct e115{};
struct e116{};
struct e117{};
struct e118{};
struct e119{};
struct e120{};
struct e121{};
struct e122{};
struct e123{};
struct e124{};
struct e125{};
struct e126{};
struct e127{};
struct e128{};
struct e129{};
struct e130{};
struct e131{};
struct e132{};
struct e133{};
struct e134{};
struct e135{};
struct e136{};
struct e137{};
struct e138{};
struct e139{};
struct e140{};
struct e141{};
struct e142{};
struct e143{};
struct e144{};
struct e145{};
struct e146{};
struct e147{};
struct e148{};
struct e149{};
struct e150{};
struct e151{};
struct e152{};
struct e153{};
struct e154{};
struct e155{};
struct e156{};
struct e157{};
struct e158{};
struct e159{};
struct e160{};
struct e161{};
struct e162{};
struct e163{};
struct e164{};
struct e165{};
struct e166{};
struct e167{};
struct e168{};
struct e169{};
struct e170{};
struct e171{};
struct e172{};
struct e173{};
struct e174{};
struct e175{};
struct e176{};
struct e177{};
struct e178{};
struct e179{};
struct e180{};
struct e181{};
struct e182{};
struct e183{};
struct e184{};
struct e185{};
struct e186{};
struct e187{};
struct e188{};
struct e189{};
struct e190{};
struct e191{};
struct e192{};
struct e193{};
struct e194{};
struct e195{};
struct e196{};
struct e197{};
struct e198{};
struct e199{};
struct e200{};


#include <cstdio>

struct Connection {
  constexpr auto operator()() const {
    constexpr auto establish = []{ std::puts("establish"); };
    constexpr auto close     = []{ std::puts("close"); };
    constexpr auto is_valid  = [](const auto& event) { return event.valid; };
    constexpr auto setup     = []{ std::puts("setup"); };

    constexpr auto guard = [] { return true; };
    constexpr auto action = [] {};

    using namespace boost::dsl;
    /**
     * src_state + event [ guard ] / action = dst_state
     */
    return transition_table{
      * "Disconnected"_s + event<connect>           / establish   = "Connecting"_s,
        "Connecting"_s   + event<established>                     = "Connected"_s,
        "Connected"_s    + event<ping> [ is_valid ] / setup,
        "Connected"_s    + event<timeout>           / establish   = "Connecting"_s,
        "Connected"_s    + event<disconnect>        / close       = "s1"_s,
      "s1"_s + event<e1> [ guard ] / action = "s2"_s,
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
      //"s51"_s + event<e51> [ guard ] / action = "s52"_s,
      //"s52"_s + event<e52> [ guard ] / action = "s53"_s,
      //"s53"_s + event<e53> [ guard ] / action = "s54"_s,
      //"s54"_s + event<e54> [ guard ] / action = "s55"_s,
      //"s55"_s + event<e55> [ guard ] / action = "s56"_s,
      //"s56"_s + event<e56> [ guard ] / action = "s57"_s,
      //"s57"_s + event<e57> [ guard ] / action = "s58"_s,
      //"s58"_s + event<e58> [ guard ] / action = "s59"_s,
      //"s59"_s + event<e59> [ guard ] / action = "s60"_s,
      //"s60"_s + event<e60> [ guard ] / action = "s61"_s,
      //"s61"_s + event<e61> [ guard ] / action = "s62"_s,
      //"s62"_s + event<e62> [ guard ] / action = "s63"_s,
      //"s63"_s + event<e63> [ guard ] / action = "s64"_s,
      //"s64"_s + event<e64> [ guard ] / action = "s65"_s,
      //"s65"_s + event<e65> [ guard ] / action = "s66"_s,
      //"s66"_s + event<e66> [ guard ] / action = "s67"_s,
      //"s67"_s + event<e67> [ guard ] / action = "s68"_s,
      //"s68"_s + event<e68> [ guard ] / action = "s69"_s,
      //"s69"_s + event<e69> [ guard ] / action = "s70"_s,
      //"s70"_s + event<e70> [ guard ] / action = "s71"_s,
      //"s71"_s + event<e71> [ guard ] / action = "s72"_s,
      //"s72"_s + event<e72> [ guard ] / action = "s73"_s,
      //"s73"_s + event<e73> [ guard ] / action = "s74"_s,
      //"s74"_s + event<e74> [ guard ] / action = "s75"_s,
      //"s75"_s + event<e75> [ guard ] / action = "s76"_s,
      //"s76"_s + event<e76> [ guard ] / action = "s77"_s,
      //"s77"_s + event<e77> [ guard ] / action = "s78"_s,
      //"s78"_s + event<e78> [ guard ] / action = "s79"_s,
      //"s79"_s + event<e79> [ guard ] / action = "s80"_s,
      //"s80"_s + event<e80> [ guard ] / action = "s81"_s,
      //"s81"_s + event<e81> [ guard ] / action = "s82"_s,
      //"s82"_s + event<e82> [ guard ] / action = "s83"_s,
      //"s83"_s + event<e83> [ guard ] / action = "s84"_s,
      //"s84"_s + event<e84> [ guard ] / action = "s85"_s,
      //"s85"_s + event<e85> [ guard ] / action = "s86"_s,
      //"s86"_s + event<e86> [ guard ] / action = "s87"_s,
      //"s87"_s + event<e87> [ guard ] / action = "s88"_s,
      //"s88"_s + event<e88> [ guard ] / action = "s89"_s,
      //"s89"_s + event<e89> [ guard ] / action = "s90"_s,
      //"s90"_s + event<e90> [ guard ] / action = "s91"_s,
      //"s91"_s + event<e91> [ guard ] / action = "s92"_s,
      //"s92"_s + event<e92> [ guard ] / action = "s93"_s,
      //"s93"_s + event<e93> [ guard ] / action = "s94"_s,
      //"s94"_s + event<e94> [ guard ] / action = "s95"_s,
      //"s95"_s + event<e95> [ guard ] / action = "s96"_s,
      //"s96"_s + event<e96> [ guard ] / action = "s97"_s,
      //"s97"_s + event<e97> [ guard ] / action = "s98"_s,
      //"s98"_s + event<e98> [ guard ] / action = "s99"_s,
      //"s99"_s + event<e99> [ guard ] / action = "s100"_s,
      //"s100"_s + event<e100> [ guard ] / action = "s101"_s,
      //"s101"_s + event<e101> [ guard ] / action = "s102"_s,
      //"s102"_s + event<e102> [ guard ] / action = "s103"_s,
      //"s103"_s + event<e103> [ guard ] / action = "s104"_s,
      //"s104"_s + event<e104> [ guard ] / action = "s105"_s,
      //"s105"_s + event<e105> [ guard ] / action = "s106"_s,
      //"s106"_s + event<e106> [ guard ] / action = "s107"_s,
      //"s107"_s + event<e107> [ guard ] / action = "s108"_s,
      //"s108"_s + event<e108> [ guard ] / action = "s109"_s,
      //"s109"_s + event<e109> [ guard ] / action = "s110"_s,
      //"s110"_s + event<e110> [ guard ] / action = "s111"_s,
      //"s111"_s + event<e111> [ guard ] / action = "s112"_s,
      //"s112"_s + event<e112> [ guard ] / action = "s113"_s,
      //"s113"_s + event<e113> [ guard ] / action = "s114"_s,
      //"s114"_s + event<e114> [ guard ] / action = "s115"_s,
      //"s115"_s + event<e115> [ guard ] / action = "s116"_s,
      //"s116"_s + event<e116> [ guard ] / action = "s117"_s,
      //"s117"_s + event<e117> [ guard ] / action = "s118"_s,
      //"s118"_s + event<e118> [ guard ] / action = "s119"_s,
      //"s119"_s + event<e119> [ guard ] / action = "s120"_s,
      //"s120"_s + event<e120> [ guard ] / action = "s121"_s,
      //"s121"_s + event<e121> [ guard ] / action = "s122"_s,
      //"s122"_s + event<e122> [ guard ] / action = "s123"_s,
      //"s123"_s + event<e123> [ guard ] / action = "s124"_s,
      //"s124"_s + event<e124> [ guard ] / action = "s125"_s,
      //"s125"_s + event<e125> [ guard ] / action = "s126"_s,
      //"s126"_s + event<e126> [ guard ] / action = "s127"_s,
      //"s127"_s + event<e127> [ guard ] / action = "s128"_s,
      //"s128"_s + event<e128> [ guard ] / action = "s129"_s,
      //"s129"_s + event<e129> [ guard ] / action = "s130"_s,
      //"s130"_s + event<e130> [ guard ] / action = "s131"_s,
      //"s131"_s + event<e131> [ guard ] / action = "s132"_s,
      //"s132"_s + event<e132> [ guard ] / action = "s133"_s,
      //"s133"_s + event<e133> [ guard ] / action = "s134"_s,
      //"s134"_s + event<e134> [ guard ] / action = "s135"_s,
      //"s135"_s + event<e135> [ guard ] / action = "s136"_s,
      //"s136"_s + event<e136> [ guard ] / action = "s137"_s,
      //"s137"_s + event<e137> [ guard ] / action = "s138"_s,
      //"s138"_s + event<e138> [ guard ] / action = "s139"_s,
      //"s139"_s + event<e139> [ guard ] / action = "s140"_s,
      //"s140"_s + event<e140> [ guard ] / action = "s141"_s,
      //"s141"_s + event<e141> [ guard ] / action = "s142"_s,
      //"s142"_s + event<e142> [ guard ] / action = "s143"_s,
      //"s143"_s + event<e143> [ guard ] / action = "s144"_s,
      //"s144"_s + event<e144> [ guard ] / action = "s145"_s,
      //"s145"_s + event<e145> [ guard ] / action = "s146"_s,
      //"s146"_s + event<e146> [ guard ] / action = "s147"_s,
      //"s147"_s + event<e147> [ guard ] / action = "s148"_s,
      //"s148"_s + event<e148> [ guard ] / action = "s149"_s,
      //"s149"_s + event<e149> [ guard ] / action = "s150"_s,
      //"s150"_s + event<e150> [ guard ] / action = "s151"_s,
      //"s151"_s + event<e151> [ guard ] / action = "s152"_s,
      //"s152"_s + event<e152> [ guard ] / action = "s153"_s,
      //"s153"_s + event<e153> [ guard ] / action = "s154"_s,
      //"s154"_s + event<e154> [ guard ] / action = "s155"_s,
      //"s155"_s + event<e155> [ guard ] / action = "s156"_s,
      //"s156"_s + event<e156> [ guard ] / action = "s157"_s,
      //"s157"_s + event<e157> [ guard ] / action = "s158"_s,
      //"s158"_s + event<e158> [ guard ] / action = "s159"_s,
      //"s159"_s + event<e159> [ guard ] / action = "s160"_s,
      //"s160"_s + event<e160> [ guard ] / action = "s161"_s,
      //"s161"_s + event<e161> [ guard ] / action = "s162"_s,
      //"s162"_s + event<e162> [ guard ] / action = "s163"_s,
      //"s163"_s + event<e163> [ guard ] / action = "s164"_s,
      //"s164"_s + event<e164> [ guard ] / action = "s165"_s,
      //"s165"_s + event<e165> [ guard ] / action = "s166"_s,
      //"s166"_s + event<e166> [ guard ] / action = "s167"_s,
      //"s167"_s + event<e167> [ guard ] / action = "s168"_s,
      //"s168"_s + event<e168> [ guard ] / action = "s169"_s,
      //"s169"_s + event<e169> [ guard ] / action = "s170"_s,
      //"s170"_s + event<e170> [ guard ] / action = "s171"_s,
      //"s171"_s + event<e171> [ guard ] / action = "s172"_s,
      //"s172"_s + event<e172> [ guard ] / action = "s173"_s,
      //"s173"_s + event<e173> [ guard ] / action = "s174"_s,
      //"s174"_s + event<e174> [ guard ] / action = "s175"_s,
      //"s175"_s + event<e175> [ guard ] / action = "s176"_s,
      //"s176"_s + event<e176> [ guard ] / action = "s177"_s,
      //"s177"_s + event<e177> [ guard ] / action = "s178"_s,
      //"s178"_s + event<e178> [ guard ] / action = "s179"_s,
      //"s179"_s + event<e179> [ guard ] / action = "s180"_s,
      //"s180"_s + event<e180> [ guard ] / action = "s181"_s,
      //"s181"_s + event<e181> [ guard ] / action = "s182"_s,
      //"s182"_s + event<e182> [ guard ] / action = "s183"_s,
      //"s183"_s + event<e183> [ guard ] / action = "s184"_s,
      //"s184"_s + event<e184> [ guard ] / action = "s185"_s,
      //"s185"_s + event<e185> [ guard ] / action = "s186"_s,
      //"s186"_s + event<e186> [ guard ] / action = "s187"_s,
      //"s187"_s + event<e187> [ guard ] / action = "s188"_s,
      //"s188"_s + event<e188> [ guard ] / action = "s189"_s,
      //"s189"_s + event<e189> [ guard ] / action = "s190"_s,
      //"s190"_s + event<e190> [ guard ] / action = "s191"_s,
      //"s191"_s + event<e191> [ guard ] / action = "s192"_s,
      //"s192"_s + event<e192> [ guard ] / action = "s193"_s,
      //"s193"_s + event<e193> [ guard ] / action = "s194"_s,
      //"s194"_s + event<e194> [ guard ] / action = "s195"_s,
      //"s195"_s + event<e195> [ guard ] / action = "s196"_s,
      //"s196"_s + event<e196> [ guard ] / action = "s197"_s,
      //"s197"_s + event<e197> [ guard ] / action = "s198"_s,
      //"s198"_s + event<e198> [ guard ] / action = "s199"_s,
      //"s199"_s + event<e199> [ guard ] / action = "s200"_s,
    };
  }
};

#include <cassert>

int main() {
  using boost::dsl::operator""_s;

  boost::sm sm{Connection{}};

  sm.process_event(connect{});
  sm.process_event(established{});
  sm.process_event(ping{true});
  sm.process_event(disconnect{});

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
  //sm.process_event(e51{});
  //sm.process_event(e52{});
  //sm.process_event(e53{});
  //sm.process_event(e54{});
  //sm.process_event(e55{});
  //sm.process_event(e56{});
  //sm.process_event(e57{});
  //sm.process_event(e58{});
  //sm.process_event(e59{});
  //sm.process_event(e60{});
  //sm.process_event(e61{});
  //sm.process_event(e62{});
  //sm.process_event(e63{});
  //sm.process_event(e64{});
  //sm.process_event(e65{});
  //sm.process_event(e66{});
  //sm.process_event(e67{});
  //sm.process_event(e68{});
  //sm.process_event(e69{});
  //sm.process_event(e70{});
  //sm.process_event(e71{});
  //sm.process_event(e72{});
  //sm.process_event(e73{});
  //sm.process_event(e74{});
  //sm.process_event(e75{});
  //sm.process_event(e76{});
  //sm.process_event(e77{});
  //sm.process_event(e78{});
  //sm.process_event(e79{});
  //sm.process_event(e80{});
  //sm.process_event(e81{});
  //sm.process_event(e82{});
  //sm.process_event(e83{});
  //sm.process_event(e84{});
  //sm.process_event(e85{});
  //sm.process_event(e86{});
  //sm.process_event(e87{});
  //sm.process_event(e88{});
  //sm.process_event(e89{});
  //sm.process_event(e90{});
  //sm.process_event(e91{});
  //sm.process_event(e92{});
  //sm.process_event(e93{});
  //sm.process_event(e94{});
  //sm.process_event(e95{});
  //sm.process_event(e96{});
  //sm.process_event(e97{});
  //sm.process_event(e98{});
  //sm.process_event(e99{});
  //sm.process_event(e100{});
  //sm.process_event(e101{});
  //sm.process_event(e102{});
  //sm.process_event(e103{});
  //sm.process_event(e104{});
  //sm.process_event(e105{});
  //sm.process_event(e106{});
  //sm.process_event(e107{});
  //sm.process_event(e108{});
  //sm.process_event(e109{});
  //sm.process_event(e110{});
  //sm.process_event(e111{});
  //sm.process_event(e112{});
  //sm.process_event(e113{});
  //sm.process_event(e114{});
  //sm.process_event(e115{});
  //sm.process_event(e116{});
  //sm.process_event(e117{});
  //sm.process_event(e118{});
  //sm.process_event(e119{});
  //sm.process_event(e120{});
  //sm.process_event(e121{});
  //sm.process_event(e122{});
  //sm.process_event(e123{});
  //sm.process_event(e124{});
  //sm.process_event(e125{});
  //sm.process_event(e126{});
  //sm.process_event(e127{});
  //sm.process_event(e128{});
  //sm.process_event(e129{});
  //sm.process_event(e130{});
  //sm.process_event(e131{});
  //sm.process_event(e132{});
  //sm.process_event(e133{});
  //sm.process_event(e134{});
  //sm.process_event(e135{});
  //sm.process_event(e136{});
  //sm.process_event(e137{});
  //sm.process_event(e138{});
  //sm.process_event(e139{});
  //sm.process_event(e140{});
  //sm.process_event(e141{});
  //sm.process_event(e142{});
  //sm.process_event(e143{});
  //sm.process_event(e144{});
  //sm.process_event(e145{});
  //sm.process_event(e146{});
  //sm.process_event(e147{});
  //sm.process_event(e148{});
  //sm.process_event(e149{});
  //sm.process_event(e150{});
  //sm.process_event(e151{});
  //sm.process_event(e152{});
  //sm.process_event(e153{});
  //sm.process_event(e154{});
  //sm.process_event(e155{});
  //sm.process_event(e156{});
  //sm.process_event(e157{});
  //sm.process_event(e158{});
  //sm.process_event(e159{});
  //sm.process_event(e160{});
  //sm.process_event(e161{});
  //sm.process_event(e162{});
  //sm.process_event(e163{});
  //sm.process_event(e164{});
  //sm.process_event(e165{});
  //sm.process_event(e166{});
  //sm.process_event(e167{});
  //sm.process_event(e168{});
  //sm.process_event(e169{});
  //sm.process_event(e170{});
  //sm.process_event(e171{});
  //sm.process_event(e172{});
  //sm.process_event(e173{});
  //sm.process_event(e174{});
  //sm.process_event(e175{});
  //sm.process_event(e176{});
  //sm.process_event(e177{});
  //sm.process_event(e178{});
  //sm.process_event(e179{});
  //sm.process_event(e180{});
  //sm.process_event(e181{});
  //sm.process_event(e182{});
  //sm.process_event(e183{});
  //sm.process_event(e184{});
  //sm.process_event(e185{});
  //sm.process_event(e186{});
  //sm.process_event(e187{});
  //sm.process_event(e188{});
  //sm.process_event(e189{});
  //sm.process_event(e190{});
  //sm.process_event(e191{});
  //sm.process_event(e192{});
  //sm.process_event(e193{});
  //sm.process_event(e194{});
  //sm.process_event(e195{});
  //sm.process_event(e196{});
  //sm.process_event(e197{});
  //sm.process_event(e198{});
  //sm.process_event(e199{});
  //sm.process_event(e200{});
}
