//
// Copyright (c) 2022-2024 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MP
#define MP 1'0'0 // SemVer
#pragma GCC system_header // -Wnon-template-friend

/**
 * ~~Template~~ Meta-Programming library (https://github.com/boost-ext/mp)
 */
namespace mp::inline v1_0_0 {
using size_t = decltype(sizeof(int));
namespace utility {
template<class T> T&& declval();
template<class T, T...> struct integer_sequence { };
template<size_t... Ns> using index_sequence = integer_sequence<size_t, Ns...>;
template<size_t Size> using make_index_sequence =
#if defined(__clang__) || defined(_MSC_VER)
  __make_integer_seq<integer_sequence, size_t, Size>;
#else
   index_sequence<__integer_pack(Size)...>;
#endif
} // namespace utility

/**
 * Meta type object representation
 */
struct meta_info { };
#if defined(MP_META_INFO)
using meta_t = const MP_META_INFO*;
#elif (__cpp_constexpr >= 202306L) // [C++23] Constexpr cast from void*
using meta_t = const void*;
#else
using meta_t = const meta_info*;
#endif

namespace detail {
template<meta_t>  struct meta_id { constexpr auto friend get(meta_id); };
template<class T> struct meta_info {
  using value_type = T;
#if defined(MP_META_INFO)
  static constexpr MP_META_INFO info{};
#else
  static constexpr mp::meta_info info{};
#endif
  constexpr auto friend get(meta_id<meta_t{&info}>) { return meta_info{}; }
};
} // namespace detail

/**
 * Creates meta type
 *
 * @code
 * static_assert(meta<void> == meta<void>);
 * static_assert(meta<void> != meta<int>);
 * @endcode
 */
template<class T>
inline constexpr meta_t meta = meta_t{&detail::meta_info<T>::info};

/**
 * Returns underlying meta_info from meta type
 *
 * @code
 * static_assert(typeid(meta_info_of<meta<void>>) == typeid(meta_info<void>));
 * @endcode
 */
template<meta_t meta>
using meta_info_of = decltype(get(detail::meta_id<meta>{}));

/**
 * Returns underlying type from meta type
 *
 * @code
 * static_assert(typeid(type_of<meta<void>>) == typeid(void));
 * @endcode
 */
template<meta_t meta>
using type_of = typename meta_info_of<meta>::value_type;

/**
 * Returns value of meta type
 *
 * @code
 * static_assert(42 = value_of_v<mp::meta<std::integral_constant<int, 42>>>);
 * @endcode
 */
template<meta_t meta>
constexpr auto value_of_v = type_of<meta>::value;

/**
 * Returns value of meta type underlying object
 */
template<meta_t meta, class T>
[[nodiscard]] constexpr decltype(auto) value_of(T&& t) {
  return meta_info_of<meta>::value_of(static_cast<T&&>(t));
}

/**
 * Minimal (not standard compliant) array
 * implementation optimized for fast compilation-times with meta_t
 *
 * @code
 * array v{meta<void>, meta<int>};
 * assert(2 == v.size());
 * assert(meta<void> == v[0]);
 * assert(meta<int> == v[1]);
 * @endcode
 */
template<class T, size_t Size>
struct array {
  using value_type = T;

  [[nodiscard]] constexpr auto begin() const { return &data[0]; }
  [[nodiscard]] constexpr auto begin() { return &data[0]; }
  [[nodiscard]] constexpr auto end() const { return &data[0]+Size; }
  [[nodiscard]] constexpr auto end() { return &data[0]+Size; }
  [[nodiscard]] constexpr auto operator[](size_t I) const { return data[I]; }
  [[nodiscard]] constexpr auto& operator[](size_t I) { return data[I]; }
  [[nodiscard]] constexpr auto size() const { return Size; }
  [[nodiscard]] constexpr bool operator==(const array& other) const {
    for (auto i = 0u; i < Size; ++i) { if (data[i] != other.data[i]) { return false; } }
    return true;
  }
  [[nodiscard]] constexpr bool operator!=(const array& other) const {
    return !operator==(other);
  }

  T data[Size];
};
template<class T, class... Ts> array(T, Ts...) -> array<T, 1 + sizeof...(Ts)>;

/**
 * Minimal (not standard compliant) inplace/static vector
 * implementation optimized for fast compilation-times with meta_t
 *
 * @code
 * vector v{meta<void>, meta<int>};
 * assert(2 == v.size());
 * assert(meta<void> == v[0]);
 * assert(meta<int> == v[1]);
 * @endcode
 */
template<class T, size_t Size>
struct vector {
  using value_type = T;
  constexpr vector() = default;
  template<class... Ts> constexpr explicit vector(Ts&&... ts) : data{ts...}, size_{sizeof...(ts)} { }
  template<class TRange, class = decltype(utility::declval<TRange>().begin(), utility::declval<TRange>().end())> constexpr vector(TRange range) { for (const T& t : range) { data[size_++] = t; } }
  constexpr void push_back(const T& t) { data[size_++] = t; }
  constexpr void emplace_back(T&& t) { data[size_++] = static_cast<T&&>(t); }
  [[nodiscard]] constexpr auto begin() const { return &data[0]; }
  [[nodiscard]] constexpr auto begin() { return &data[0]; }
  [[nodiscard]] constexpr auto end() const { return &data[0]+size_; }
  [[nodiscard]] constexpr auto end() { return &data[0]+size_; }
  [[nodiscard]] constexpr auto operator[](size_t i) const { return data[i]; }
  [[nodiscard]] constexpr auto& operator[](size_t i) { return data[i]; }
  [[nodiscard]] constexpr auto size() const { return size_; }
  [[nodiscard]] constexpr auto& size() { return size_; }
  [[nodiscard]] constexpr auto capacity() const { return Size; }
  template<size_t size> [[nodiscard]] constexpr auto operator==(const vector<T, size>& other) const {
    if (size_ != other.size_) { return false; }
    for (auto i = 0u; i < size_; ++i) { if (data[i] != other.data[i]) { return false; } }
    return true;
  }
  template<size_t size> [[nodiscard]] constexpr auto operator!=(const vector<T, size>& other) const {
    return !operator==(other);
  }
  constexpr void clear() { size_ = {}; }

  union { T data[Size]{}; }; // !trivial union
  size_t size_{};
};
template<class T, class... Ts> vector(T, Ts...) -> vector<T, 1 + sizeof...(Ts)>;

/**
 * Applies expression expr to `R<type_of<meta_t>...>`
 *
 * @code
 * static_assert(typeid(variant<int>) == typeid(apply<variant>([] { return vector{meta<int>}; })));
 * @endcode
 */
template<template<class...> class R, class Expr>
[[nodiscard]] constexpr auto apply(Expr expr) {
  constexpr auto v = expr();
  return [v]<size_t... Ns>(utility::index_sequence<Ns...>) {
    return R<type_of<v[Ns]>...>{};
  }(utility::make_index_sequence<v.size()>{});
}

/**
 * Applies vector V to `R<type_of<meta_t>...>`
 *
 * @code
 * static_assert(typeid(variant<int>) == typeid(apply<variant, vector{meta<int>}>));
 * @endcode
 */
#if defined(__cpp_nontype_template_args)
template<template<class...> class R, auto V>
inline constexpr auto apply_v = []<size_t... Ns>(utility::index_sequence<Ns...>) {
  return R<type_of<V[Ns]>...>{};
}(utility::make_index_sequence<V.size()>{});
#endif

/**
 * Applies vector V with object t to `R{value_of<V>(t)...}
 */
#if defined(__cpp_nontype_template_args)
template<template<class...> class R, auto V, class T>
[[nodiscard]] constexpr auto apply(T&& t) {
  return [&]<size_t... Ns>(utility::index_sequence<Ns...>) {
    return R{meta_info_of<V[Ns]>::value_of(static_cast<T&&>(t))...};
  }(utility::make_index_sequence<V.size()>{});
}
#endif

/**
 * Applies vector V with object t to `R<type_of<V>...>
 */
#if defined(__cpp_nontype_template_args)
template<template<class...> class R, auto V>
[[nodiscard]] constexpr auto apply() {
  return []<size_t... Ns>(utility::index_sequence<Ns...>) {
    return R<type_of<V[Ns]>...>{};
  }(utility::make_index_sequence<V.size()>{});
}
#endif

/**
 * Alternative to write `decltype(apply_v<T, Expr>))`
 *
 * @code
 * static_assert(typeid(variant<int>) == typeid(apply_t<variant, [] { return vector{meta<int>}; }>));
 * @endcode
 */
#if defined(__cpp_nontype_template_args)
#if defined(__clang__) || defined(_MSC_VER)
template<template<class...> class R, auto V>
using apply_t = decltype(apply<R, V>());
#else
namespace detail {
template<template<class...> class R, auto V, size_t... Ns>
using apply_t = R<type_of<V[Ns]>...>;
} // namespace detail
template<template<class...> class R, auto V>
using apply_t = detail::apply_t<R, V, __integer_pack(V.size())...>;
#endif
#endif

/**
 * Iterates over all elements of constexpr container
 *
 * @code
 * constexpr vector v{meta<int>};
 * for_each<v>([]<auto m> {
 *   static_assert(typeid(int) == typeid(type_of<m>));
 * });
 * @endcode
 */
#if (__cpp_generic_lambdas >= 201707L)
template<auto V, class Fn>
constexpr void for_each(Fn fn) {
  [fn]<size_t... Ns>(utility::index_sequence<Ns...>) {
    (fn.template operator()<V[Ns]>(), ...);
  }(utility::make_index_sequence<V.size()>{});
}
#endif
} // namespace mp

#if !defined(DISABLE_STATIC_ASSERT_TESTS)
namespace mp::inline v1_0_0::test {
  template<class...> struct type_list { };
  template<auto I> struct _i { static constexpr auto value = I; };
  template<class...> inline constexpr auto is_same_v = false;
  template<class T> inline constexpr auto is_same_v<T, T> = true;
  void failed(); constexpr void expect(bool cond) { if (!cond) { failed(); } }
} // namespace mp::test

static_assert(([] {
  using mp::test::expect;

  // mp::test
  {
    static_assert(!mp::test::is_same_v<int, void>);
    static_assert(!mp::test::is_same_v<void, int>);
    static_assert(!mp::test::is_same_v<void*, int>);
    static_assert(!mp::test::is_same_v<int, const int>);
    static_assert(mp::test::is_same_v<void, void>);
    static_assert(mp::test::is_same_v<int, int>);
  }

  // mp::utilty::declval
  {
    struct s { s() = delete; };
    static_assert(sizeof(s) == sizeof(mp::utility::declval<s>()));
    static_assert(sizeof(s&) == sizeof(mp::utility::declval<s&>()));
  }

  // mp::utility::index_sequence
  {
    static_assert([](mp::utility::integer_sequence<mp::size_t>){ return true; }(mp::utility::make_index_sequence<0>{}));
    static_assert([](mp::utility::integer_sequence<mp::size_t, 0>){ return true; }(mp::utility::make_index_sequence<1>{}));
    static_assert([](mp::utility::integer_sequence<mp::size_t, 0, 1>){ return true; }(mp::utility::make_index_sequence<2>{}));
    static_assert([](mp::utility::index_sequence<>){ return true; }(mp::utility::make_index_sequence<0>{}));
    static_assert([](mp::utility::index_sequence<0>){ return true; }(mp::utility::make_index_sequence<1>{}));
    static_assert([](mp::utility::index_sequence<0, 1>){ return true; }(mp::utility::make_index_sequence<2>{}));
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
    static_assert(mp::test::is_same_v<void, mp::type_of<mp::meta<void>>>);
    static_assert(mp::test::is_same_v<const int, mp::type_of<mp::meta<const int>>>);
    static_assert(mp::test::is_same_v<const void*, mp::type_of<mp::meta<const void*>>>);
    static_assert(mp::test::is_same_v<e, mp::type_of<mp::meta<e>>>);

    constexpr mp::vector v{mp::meta<int>, mp::meta<e>};
    static_assert(mp::test::is_same_v<int, mp::type_of<v[0]>>);
    static_assert(mp::test::is_same_v<e, mp::type_of<v[1]>>);
  }

  // mp::value_of
  {
    static_assert(0 == mp::value_of_v<mp::meta<mp::test::_i<0>>>);
    static_assert('a' == mp::value_of_v<mp::meta<mp::test::_i<'a'>>>);
    static_assert(42u == mp::value_of_v<mp::meta<mp::test::_i<42u>>>);
  }

  // mp::apply
  {
    static_assert([](mp::test::type_list<>){ return true; }(mp::apply<mp::test::type_list>([] { return mp::vector<mp::meta_t, 1>{}; })));
    static_assert([](mp::test::type_list<int>){ return true; }(mp::apply<mp::test::type_list>([] { return mp::vector{mp::meta<int>}; })));
    static_assert([](mp::test::type_list<void, void>){ return true; }(mp::apply<mp::test::type_list>([] { return mp::vector{mp::meta<void>, mp::meta<void>}; })));
  }

#if defined(__cpp_generic_lambdas) && (__cpp_generic_lambdas >= 201707L)
  // mp::for_each
  {
    {
      constexpr mp::vector<mp::meta_t, 1> m{};
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

  // mp::array
  {
    {
      mp::array<mp::size_t, 1> a{};
      expect(1 == a.size());
    }

    {
      mp::array a{1, 2};
      expect(2u == a.size());
      expect(1 == a[0]);
      expect(2 == a[1]);
    }

    {
      expect(mp::array{1, 2} == mp::array{1, 2});
      expect(mp::array{1, 2} != mp::array{1, 3});
    }

    {
      mp::array a{1};
      a[0] = 2;
      expect(2 == a[0]);
    }

    {
      mp::array a{1, 2, 3};
      expect(3u == a.size());
      expect(a.begin() != a.end());
      expect(a.size() == mp::size_t(a.end() - a.begin()));
      expect(a.end() == a.begin() + a.size());
    }
  }

  // mp::vector
  {
    {
      mp::vector<mp::size_t, 1> v{};
      expect(0u == v.size());
      expect(1 == v.capacity());
    }

    {
      mp::vector<int, 3> v{1, 2, 3};
      expect(3u == v.size());
      expect(3 == v.capacity());
    }

    {
      constexpr mp::vector<int, 1> v1{1};
      static_assert(1u == v1.size());
      static_assert(1 == v1[0]);

      constexpr mp::vector<int, 1> v2{v1};
      static_assert(1u == v2.size());
      static_assert(1 == v2[0]);
    }

    {
      mp::vector<int, 4> v1{1, 2};
      expect(2u == v1.size());
      expect(4 == v1.capacity());
      expect(1 == v1[0]);
      expect(2 == v1[1]);

      mp::vector<int, 2> v2{v1};
      expect(2u == v2.size());
      expect(2 == v2.capacity());
      expect(1 == v2[0]);
      expect(2 == v2[1]);
    }

    {
      mp::vector<int, 2> v;
      expect(0u == v.size());
      expect(2 == v.capacity());

      v.push_back(1);
      expect(1u == v.size());
      expect(1 == v[0]);

      v.push_back(2);
      expect(2u == v.size());
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
      expect(2u == v.size());
      expect(2 == v.capacity());

      v.clear();
      expect(0u == v.size());
      expect(2 == v.capacity());
    }

    {
      mp::vector<int, 42> v{};
      expect(0u == v.size() && 0u == mp::size_t(v.end() - v.begin()));
      expect(v.begin() == v.end());

      v.emplace_back(1);
      expect(1u == v.size() && 1 == mp::size_t(v.end() - v.begin()));
      expect(v.begin() != v.end());

      v.emplace_back(2);
      expect(2u == v.size() && 2u == mp::size_t(v.end() - v.begin()));
      expect(v.begin() != v.end());
    }

    {
      expect(mp::vector<int, 1>{1} == mp::vector<int, 1>{1});
      expect(mp::vector<int, 1>{1} == mp::vector<int, 2>{1});
      expect(mp::vector<int, 1>{1} != mp::vector<int, 1>{2});
      expect(mp::vector<int, 1>{1} != mp::vector<int, 2>{2});
    }
  }
}(), true));
#endif
#endif // MP
