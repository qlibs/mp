
constexpr auto magic =
    mp::type_list<foo, bar<0>, bar<1>, literal>{} | []<class... Ts> {
      if constexpr ((requires(Ts ts) { ts.foo; } or ...)) {
        return mp::value_list<1, 2, 3>{};
      } else {
        return mp::type_list<int>{};
      }
    };
