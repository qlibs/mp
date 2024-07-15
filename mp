//
// Copyright (c) 2022-2024 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MP
#define MP 2'0'2 // SemVer
#if defined(__GNUC__)
#pragma GCC system_header
#endif

/**
 * ~~Template~~ Meta-Programming library (https://github.com/boost-ext/mp)
 */
namespace mp::inline v2_0_2 {
using size_t = decltype(sizeof(int));

namespace utility {
template<class T, T...> struct integer_sequence { };
template<size_t... Ns> using index_sequence = integer_sequence<size_t, Ns...>;
template<size_t Size> using make_index_sequence =
#if defined(__clang__) || defined(_MSC_VER)
  __make_integer_seq<integer_sequence, size_t, Size>;
#else
   index_sequence<__integer_pack(Size)...>;
#endif
template<class TRange>
concept range = requires (TRange range) { range.begin(); range.end(); };
} // namespace utility

/**
 * Minimal (not standard compliant) array optimized for fast compilation-times
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
  [[nodiscard]] constexpr auto end() const { return &data[0] + Size; }
  [[nodiscard]] constexpr auto end() { return &data[0] + Size; }
  [[nodiscard]] constexpr auto operator[](size_t I) const { return data[I]; }
  [[nodiscard]] constexpr auto& operator[](size_t I) { return data[I]; }
  [[nodiscard]] constexpr auto size() const { return Size; }
  [[nodiscard]] constexpr bool operator==(const array& other) const {
    for (auto i = 0u; i < Size; ++i) { if (data[i] != other.data[i]) { return false; } }
    return true;
  }
  [[nodiscard]] constexpr auto operator!=(const array& other) const -> bool {
    return not operator==(other);
  }
  T data[Size];
};
template<class T, class... Ts> array(T, Ts...) -> array<T, 1u + sizeof...(Ts)>;

/**
 * Minimal (not standard compliant) inplace vector optimized for fast compilation-times
 *
 * @code
 * inplace_vector v{meta<void>, meta<int>};
 * assert(2 == v.size());
 * assert(meta<void> == v[0]);
 * assert(meta<int> == v[1]);
 * @endcode
 */
template<class T, size_t Size>
struct inplace_vector {
  using value_type = T;
  constexpr inplace_vector() = default;
  template<class... Ts> constexpr explicit inplace_vector(const Ts&... ts) : data{ts...}, size_{sizeof...(ts)} { }
  constexpr inplace_vector(utility::range auto range) { for (const T& t : range) { data[size_++] = t; } }
  constexpr void push_back(const T& t) { data[size_++] = t; }
  constexpr void emplace_back(T&& t) { data[size_++] = static_cast<T&&>(t); }
  [[nodiscard]] constexpr auto begin() const { return &data[0]; }
  [[nodiscard]] constexpr auto begin() { return &data[0]; }
  [[nodiscard]] constexpr auto end() const { return &data[0] + size_; }
  [[nodiscard]] constexpr auto end() { return &data[0] + size_; }
  [[nodiscard]] constexpr auto operator[](size_t i) const { return data[i]; }
  [[nodiscard]] constexpr auto& operator[](size_t i) { return data[i]; }
  [[nodiscard]] constexpr auto size() const { return size_; }
  [[nodiscard]] constexpr auto resize(size_t size) { size_ = size; }
  [[nodiscard]] constexpr auto capacity() const { return Size; }
  template<size_t size> [[nodiscard]] constexpr auto operator==(const inplace_vector<T, size>& other) const -> bool {
    if (size_ != other.size_) { return false; }
    for (auto i = 0u; i < size_; ++i) { if (data[i] != other.data[i]) { return false; } }
    return true;
  }
  template<size_t size> [[nodiscard]] constexpr auto operator!=(const inplace_vector<T, size>& other) const -> bool {
    return not operator==(other);
  }
  constexpr void clear() { size_ = {}; }

  union { T data[Size]{}; }; // __cpp_trivial_union
  size_t size_{};
};
template<class T, class... Ts> inplace_vector(T, Ts...) -> inplace_vector<T, 1u + sizeof...(Ts)>;

/**
 * Meta info type
 */
enum class info : size_t { };

/**
 * Meta info vector
 */
#if not defined(MP_SIZE)
#define MP_SIZE 1024
#endif
#if defined(__cpp_deduction_guides) && (__cpp_deduction_guides >= 201907L)
template<class T = info, size_t Size = MP_SIZE>
using vector = inplace_vector<T, Size>;
#else
template<class T = info, size_t Size = MP_SIZE>
struct vector : inplace_vector<T, Size> {
  using inplace_vector<T, Size>::inplace_vector;
  constexpr vector() = default;
  constexpr vector(utility::range auto range) {
    for (const T& t : range) { inplace_vector<T, Size>::data[inplace_vector<T, Size>::size_++] = t; }
  }
};
template<class T, class... Ts> vector(T, Ts...) -> vector<T, 1u + sizeof...(Ts)>;
#endif

namespace detail {
template<info> struct meta_id { constexpr auto friend get(meta_id); };
template<class T> struct meta_info {
  using value_type = T;
  template<info N = info{}> static constexpr auto idx() -> info {
    if constexpr (requires { get(meta_id<N>{}); }) { return idx<info{size_t(N) + 1u}>(); } else { return N; }
  };
  static constexpr info id = idx();
  constexpr auto friend get(meta_id<id>) { return meta_info{}; }
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
inline constexpr info meta = detail::meta_info<T>::id;

/**
 * Returns underlying type from meta type
 *
 * @code
 * static_assert(typeid(type_of<meta<void>>) == typeid(void));
 * @endcode
 */
template<info meta>
using type_of = typename decltype(get(detail::meta_id<meta>{}))::value_type;

/**
 * Applies expression expr to `T<type_of<info>...>`
 *
 * @code
 * static_assert(typeid(variant<int>) == typeid(apply<variant>([] { return vector{meta<int>}; })));
 * @endcode
 */
template<template<class...> class T>
[[nodiscard]] constexpr auto apply(auto expr) {
  constexpr auto v = expr();
  return [v]<size_t... Ns>(utility::index_sequence<Ns...>) {
    return T<type_of<v[Ns]>...>{};
  }(utility::make_index_sequence<v.size()>{});
}

/**
 * Applies vector V to `T<type_of<info>...>`
 *
 * @code
 * static_assert(typeid(variant<int>) == typeid(apply<variant, vector{meta<int>}>));
 * @endcode
 */
template<template<class...> class T, auto V>
inline constexpr auto apply_v = []<size_t... Ns>(utility::index_sequence<Ns...>) {
  return T<type_of<V[Ns]>...>{};
}(utility::make_index_sequence<V.size()>{});

/**
 * Applies vector V to `T<type_of<info>...>`
 *
 * @code
 * static_assert(typeid(variant<int>) == typeid(apply_t<variant, [] { return vector{meta<int>}; }>));
 * @endcode
 */
#if defined(__clang__) || defined(_MSC_VER)
template<template<class...> class R, auto V>
using apply_t = decltype(apply<R>([] { return V; }));
#else
namespace detail {
template<template<class...> class R, auto V, size_t... Ns>
using apply_t = R<type_of<V[Ns]>...>;
} // namespace detail
template<template<class...> class R, auto V>
using apply_t = detail::apply_t<R, V, __integer_pack(V.size())...>;
#endif

/**
 * Invokes function with compile-time info based on run-time info
 *
 * @code
 * info i = meta<conts int>; // run-time
 * static_assert(invoke([]<info m> { return std::is_const_v<type_of<m>>; }, i));
 * @endcode
 */
template<class Fn, auto tag = []{}>
constexpr auto invoke(Fn fn, info meta) {
  constexpr auto dispatch = [&]<size_t... Ns>(utility::index_sequence<Ns...>) {
    return array{
      []<info N> {
        return +[](Fn fn) {
          if constexpr (requires { fn.template operator()<N>(); }) {
            return fn.template operator()<N>();
          }
        };
      }.template operator()<info{Ns}>()...
    };
  }(utility::make_index_sequence<size_t(detail::meta_info<decltype(tag)>::id)>{});
  return dispatch[size_t(meta)](fn);
}

/**
 * Invokes meta-function
 *
 * @code
 * static_assert(not invoke<std::is_const>(meta<int>));
 * static_assert(invoke<std::is_const>(meta<const int>));
 * @endcode
 */
template<template<class...> class T, class... Ts, auto = []{}>
constexpr auto invoke(info meta) {
  return invoke([]<info meta> {
    using type = type_of<meta>;
    if constexpr (requires { T<type, Ts...>::value; }) {
      return T<type>::value;
    } else {
      return mp::meta<typename T<type, Ts...>::type>;
    }
  }, meta);
}

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
template<auto V>
constexpr void for_each(auto fn) {
  [fn]<size_t... Ns>(utility::index_sequence<Ns...>) {
    (fn.template operator()<V[Ns]>(), ...);
  }(utility::make_index_sequence<V.size()>{});
}
} // namespace mp

#if not defined(NTEST)
namespace mp::test {
  template<class...> struct type_list { };
  template<class...> inline constexpr auto is_same_v = false;
  template<class T> inline constexpr auto is_same_v<T, T> = true;
  template<class T> struct is_const { static constexpr auto value = false; };
  template<class T> struct is_const<const T> { static constexpr auto value = true; };
  template<class T> struct add_const { using type = const T; };
  void failed(); constexpr void expect(bool cond) { if (not cond) { failed(); } }
} // namespace mp::test

static_assert(([] {
  using mp::test::expect;

  // mp::test
  {
    static_assert(not mp::test::is_same_v<int, void>);
    static_assert(not mp::test::is_same_v<void, int>);
    static_assert(not mp::test::is_same_v<void*, int>);
    static_assert(not mp::test::is_same_v<int, const int>);
    static_assert(mp::test::is_same_v<void, void>);
    static_assert(mp::test::is_same_v<int, int>);
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
      mp::vector<int, 3> v{1, 2, 3};
      expect(3u == v.size());
      v.resize(2);
      expect(2u == v.size());
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

  // mp::detail
  {
    static_assert(mp::detail::meta_info<int>::id == mp::detail::meta_info<int>::id);
    static_assert(mp::detail::meta_info<int>::id != mp::detail::meta_info<void>::id);
    static_assert(mp::detail::meta_info<void>::id == mp::detail::meta_info<void>::id);
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

  // mp::apply
  {
    static_assert([](mp::test::type_list<>){ return true; }(mp::apply<mp::test::type_list>([] { return mp::vector<mp::info, 1>{}; })));
    static_assert([](mp::test::type_list<int>){ return true; }(mp::apply<mp::test::type_list>([] { return mp::vector{mp::meta<int>}; })));
    static_assert([](mp::test::type_list<void, void>){ return true; }(mp::apply<mp::test::type_list>([] { return mp::vector{mp::meta<void>, mp::meta<void>}; })));
  }

  // mp::invoke
  {
    static_assert(mp::invoke([]<mp::info meta> { return meta == mp::meta<int>; }, mp::meta<int>));
    static_assert(not mp::invoke([]<mp::info meta> { return meta == mp::meta<void>; }, mp::meta<int>));

    static_assert(not mp::invoke<mp::test::is_const>(mp::meta<int>));
    static_assert(mp::invoke<mp::test::is_const>(mp::meta<const int>));

    static_assert(mp::test::is_same_v<const int, mp::type_of<mp::invoke<mp::test::add_const>(mp::meta<int>)>>);
    static_assert(mp::test::is_same_v<const int, mp::type_of<mp::invoke<mp::test::add_const>(mp::meta<const int>)>>);
  }

  // mp::for_each
  {
    {
      constexpr mp::vector<mp::info, 1> m{};
      mp::vector<mp::info, 42> v{};
      mp::for_each<m>([&]<auto meta> { v.push_back(meta); });
      expect(0u == v.size());
    }

    {
      mp::for_each<mp::vector{mp::meta<int>}>([&]<auto meta> { expect(meta == mp::meta<int>); });
    }

    {
      constexpr mp::vector m{mp::meta<int>, mp::meta<void>};
      mp::vector<mp::info, 42> v{};
      mp::for_each<m>([&]<mp::info meta> { v.push_back(meta); });
      expect(2u == v.size());
      expect(m[0] == v[0]);
      expect(m[1] == v[1]);
    }
  }
}(), true));
#endif
#endif // MP
