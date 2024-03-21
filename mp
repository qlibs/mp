//
// Copyright (c) 2022-2024 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#if !defined(MP)
#define MP 1'0'0 // SemVer

#if defined(__GNUC__)
#pragma GCC system_header // -Wnon-template-friend
#endif

/**
 * ~~Template~~ Meta-programming library ($CXX -x c++ -std=c++17/20 -c mp)
 */
namespace mp::inline v1_0_0 {
namespace detail {
using size_t = decltype(sizeof(int));
template<class T> T&& declval();
template<class T, T...> struct integer_sequence { };
template<size_t... Ns> using index_sequence = integer_sequence<size_t, Ns...>;
template<size_t Size> using make_index_sequence =
#if defined(__clang__) || defined(_MSC_VER)
  __make_integer_seq<integer_sequence, size_t, Size>;
#else
   index_sequence<__integer_pack(Size)...>;
#endif
#if defined(MP_EXPERIMENTAL)
enum class meta_t : size_t { };
#elif (__cpp_nontype_template_args >= 201911L) && defined (__clang__)
struct meta_t {
  const size_t* id{};
  [[nodiscard]] constexpr operator auto() const { return id; }
};
#else
using meta_t = const size_t*;
#endif
template<meta_t> struct id { constexpr auto friend get(id); };
template<class T> struct type_identity { using type = T; };
template<class T, meta_t Id> struct meta_id { constexpr auto friend get(id<Id>) { return type_identity<T>{}; } };
#if defined(MP_EXPERIMENTAL)
template<class T, size_t Id> [[nodiscard]] constexpr auto type_id(...) -> detail::size_t { return Id; }
template<class T, size_t Id> [[nodiscard]] constexpr auto type_id(int) -> decltype(get(id<meta_t(Id)>{}), size_t{}) { return type_id<T, Id+1>(Id); }
template<class T> [[nodiscard]] constexpr auto meta() -> meta_t {
  constexpr auto id = meta_t(type_id<T, 0>(0));
  meta_id<T, id>();
  return id;
}
#else
template<class T> struct type { static constexpr size_t id{}; };
template<class T> [[nodiscard]] constexpr auto meta() -> meta_t {
  constexpr auto id = meta_t(&type<T>::id);
  meta_id<T, id>();
  return id;
}
#endif
} // namespace detail

using meta_t = detail::meta_t;
template<class T> inline constexpr meta_t meta = detail::meta<T>();
template<meta_t meta> using type_of = typename decltype(get(detail::id<meta>{}))::type;

template<class T, detail::size_t Size>
struct vector {
  constexpr vector() = default;
  template<class... Ts> constexpr vector(const Ts&... ts) : data{ts...}, size_{sizeof...(ts)} { }
  template<class TRange, class = decltype(detail::declval<TRange>().begin(), detail::declval<TRange>().end())> constexpr vector(TRange range) { for (const T& t : range) { data[size_++] = t; } }
  constexpr void push_back(const T& t) { data[size_++] = t; }
  [[nodiscard]] constexpr auto begin() const { return &data[0]; }
  [[nodiscard]] constexpr auto begin() { return &data[0]; }
  [[nodiscard]] constexpr auto end() const { return &data[0]+size_; }
  [[nodiscard]] constexpr auto end() { return &data[0]+size_; }
  [[nodiscard]] constexpr auto operator[](detail::size_t i) const { return data[i]; }
  [[nodiscard]] constexpr auto& operator[](detail::size_t i) { return data[i]; }
  [[nodiscard]] constexpr auto size() const { return size_; }
  [[nodiscard]] constexpr auto capacity() const { return Size; }
  constexpr void clear() { size_ = {}; }
  T data[Size?Size:1]{};
  detail::size_t size_{};
};
template<class T, class... Ts> vector(T, Ts...) -> vector<T, 1+sizeof...(Ts)>;

template<template<class...> class T, class Expr>
[[nodiscard]] constexpr auto apply(Expr expr) {
  constexpr auto ml = mp::vector<mp::meta_t, expr().size()>(expr());
  return [ml]<detail::size_t... Ns>(detail::index_sequence<Ns...>) {
    return T<typename decltype(get(detail::id<ml[Ns]>{}))::type...>();
  }(detail::make_index_sequence<ml.size()>{});
}

#if defined(__cpp_nontype_template_args)
template<class Expr, class Fn>
[[nodiscard]] constexpr auto apply(Expr expr, Fn fn) {
  constexpr auto ml = mp::vector<mp::meta_t, expr().size()>(expr());
  return [ml, fn]<detail::size_t... Ns>(detail::index_sequence<Ns...>) {
    return fn.template operator()<type_of<ml[Ns]>...>();
  }(detail::make_index_sequence<ml.size()>{});
}
#endif

#if defined(__cpp_nontype_template_args)
template<template<class...> class T, auto Expr>
using apply_t = decltype(apply<T>(Expr));
#endif

#if (__cpp_generic_lambdas >= 201707L)
template<auto V, class Fn>
constexpr void for_each(Fn fn) {
  [fn]<detail::size_t... Ns>(detail::index_sequence<Ns...>) {
    (fn.template operator()<V[Ns]>(), ...);
  }(detail::make_index_sequence<V.size()>{});
}
#endif

#if defined(MP_EXPERIMENTAL)
namespace detail {
template<class R, class T, class Fn> constexpr auto call(Fn fn, int) -> decltype(fn.template operator()<T>()) { return fn.template operator()<T>(); }
template<class R, class T, class Fn> constexpr auto call(Fn, ...) -> R { return {}; }
template<template<class> class Fn, class T = void> constexpr auto has_value(int) -> decltype(Fn<T>::value) { return true; }
template<template<class> class Fn, class...> constexpr auto has_value(...) -> bool { return false; }
} // namespace detail

template<class R = meta_t, class Fn> constexpr auto apply(meta_t m, Fn fn) {
  return [&]<detail::size_t... Ns>(detail::index_sequence<Ns...>) {
    R (*dispatch[])(Fn){[](Fn fn) -> R { return detail::call<R, type_of<meta_t(Ns)>>(fn, int{}); }...};
    return dispatch[detail::size_t(m)](fn);
  }(detail::make_index_sequence<detail::size_t(meta<class last>)>{});
}
template<template<class> class Fn>
[[nodiscard]] constexpr auto apply(mp::meta_t meta) {
  if constexpr (detail::has_value<Fn>(int{})) {
    return mp::apply<bool>(meta, []<class T> { return Fn<T>::value; });
  } else {
    return mp::apply<mp::meta_t>(meta, []<class T> { return mp::meta<typename Fn<T>::type>; });
  }
}
#endif
} // namespace mp
#endif // MP

#if !defined(DISABLE_STATIC_ASSERT_TESTS)
namespace mp::inline v1_0_0::test {
  template<class...> struct type_list { };
  template<class...> inline constexpr auto is_same = false;
  template<class TLhs, class TRhs> inline constexpr auto is_same<TLhs, TRhs> = true;
  constexpr void failed();
  constexpr void expect(bool cond) { if (not cond) { failed(); } }
} // namespace mp::test

static_assert(([] {
  using mp::test::expect;

  // mp::detail
  {
    // type_identity
    {
      static_assert([](int){ return true; }(typename mp::detail::type_identity<int>::type{}));
      static_assert([](void*){ return true; }(typename mp::detail::type_identity<void*>::type{}));
    }

    // declval
    {
      struct s { s() = delete; };
      static_assert(sizeof(s) == sizeof(mp::detail::declval<s>()));
      static_assert(sizeof(s&) == sizeof(mp::detail::declval<s&>()));
    }

    // index_sequence
    {
      static_assert([](mp::detail::integer_sequence<mp::detail::size_t>){ return true; }(mp::detail::make_index_sequence<0>{}));
      static_assert([](mp::detail::integer_sequence<mp::detail::size_t, 0>){ return true; }(mp::detail::make_index_sequence<1>{}));
      static_assert([](mp::detail::integer_sequence<mp::detail::size_t, 0, 1>){ return true; }(mp::detail::make_index_sequence<2>{}));
      static_assert([](mp::detail::index_sequence<>){ return true; }(mp::detail::make_index_sequence<0>{}));
      static_assert([](mp::detail::index_sequence<0>){ return true; }(mp::detail::make_index_sequence<1>{}));
      static_assert([](mp::detail::index_sequence<0, 1>){ return true; }(mp::detail::make_index_sequence<2>{}));
    }
  }

  // mp::meta
  {
    static_assert(mp::meta<int> == mp::meta<int>);
    static_assert(mp::meta<int> != mp::meta<double>);
    static_assert(mp::meta<double> == mp::meta<double>);
    static_assert(mp::meta<int> == mp::meta<int>);
    static_assert(mp::meta<void> == mp::meta<void>);
  }

  // mp::type_of
  {
    enum class e : int { };
    static_assert(mp::test::is_same<void, mp::type_of<mp::meta<void>>>);
    static_assert(mp::test::is_same<const int, mp::type_of<mp::meta<const int>>>);
    static_assert(mp::test::is_same<const void*, mp::type_of<mp::meta<const void*>>>);
    static_assert(mp::test::is_same<e, mp::type_of<mp::meta<e>>>);

    constexpr mp::vector v{mp::meta<int>, mp::meta<e>};
    static_assert(mp::test::is_same<int, mp::type_of<v[0]>>);
    static_assert(mp::test::is_same<e, mp::type_of<v[1]>>);
  }

  // mp::apply
  {
    static_assert([](mp::test::type_list<>){ return true; }(mp::apply<mp::test::type_list>([] { return mp::vector<mp::meta_t, 0>{}; })));
    static_assert([](mp::test::type_list<int>){ return true; }(mp::apply<mp::test::type_list>([] { return mp::vector{mp::meta<int>}; })));
    static_assert([](mp::test::type_list<void, void>){ return true; }(mp::apply<mp::test::type_list>([] { return mp::vector{mp::meta<void>, mp::meta<void>}; })));
  }

  // mp::apply_t
#if (__cpp_nontype_template_args >= 201911L)
  static_assert(mp::test::is_same<mp::test::type_list<>, mp::apply_t<mp::test::type_list, [] { return mp::vector<mp::meta_t, 0>{}; }>>);
  static_assert(mp::test::is_same<mp::test::type_list<int, void>, mp::apply_t<mp::test::type_list, [] { return mp::vector{mp::meta<int>, mp::meta<void>}; }>>);
#endif

#if defined(__cpp_generic_lambdas) && (__cpp_generic_lambdas >= 201707L)
  // mp::for_each
  {
    {
      constexpr mp::vector<mp::meta_t, 0> m{};
      mp::vector<mp::meta_t, 42> v{};
      mp::for_each<m>([&]<auto meta> { v.push_back(meta); });
      expect(0 == v.size());
    }

    {
      mp::for_each<mp::vector{mp::meta<int>}>([&]<auto meta> { expect(meta == mp::meta<int>); });
    }

#if !defined(_MSC_VER)
    {
      constexpr mp::vector m{mp::meta<int>, mp::meta<void>};
      mp::vector<mp::meta_t, 42> v{};
      mp::for_each<m>([&]<auto meta> { v.push_back(meta); });
      expect(2 == v.size());
      expect(m[0] == v[0]);
      expect(m[1] == v[1]);
    }
#endif
  }
#endif

  // mp::vector
  {
    {
      mp::vector<mp::detail::size_t, 0> v{};
      expect(0 == v.size());
      expect(0 == v.capacity());
    }

    {
      mp::vector v{1, 2, 3};
      expect(3 == v.size());
      expect(3 == v.capacity());
    }

    {
      constexpr mp::vector<int, 1> v1{1};
      static_assert(1 == v1.size());
      static_assert(1 == v1[0]);

      constexpr mp::vector<int, 1> v2{v1};
      static_assert(1 == v2.size());
      static_assert(1 == v2[0]);
    }

    {
      mp::vector<int, 4> v1{1, 2};
      expect(2 == v1.size());
      expect(4 == v1.capacity());
      expect(1 == v1[0]);
      expect(2 == v1[1]);

      mp::vector<int, 2> v2{v1};
      expect(2 == v2.size());
      expect(2 == v2.capacity());
      expect(1 == v2[0]);
      expect(2 == v2[1]);
    }

    {
      mp::vector<int, 2> v;
      expect(0 == v.size());
      expect(2 == v.capacity());

      v.push_back(1);
      expect(1 == v.size());
      expect(1 == v[0]);

      v.push_back(2);
      expect(2 == v.size());
      expect(2 == v[1]);
    }

    {
      mp::vector<int, 1> v{42};
      expect(42 == v[0]);
      v[0]++;
      expect(43 == v[0]);
      v[0] = 0;
      expect(0 == v[0]);
    }

    {
      mp::vector<int, 2> v{1, 2};
      expect(2 == v.size());
      expect(2 == v.capacity());

      v.clear();
      expect(0 == v.size());
      expect(2 == v.capacity());
    }

    {
      mp::vector<int, 42> v{};
      expect(0 == v.size() && 0 == v.end() - v.begin());
      expect(v.begin() == v.end());

      v.push_back(1);
      expect(1 == v.size() && 1 == v.end() - v.begin());
      expect(v.begin() != v.end());

      v.push_back(2);
      expect(2 == v.size() && 2 == v.end() - v.begin());
      expect(v.begin() != v.end());
    }
  }
}(), true));
#endif
