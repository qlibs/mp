#include <boost/mp.hpp>

template<class... Ts>
auto append = []<class... Tx>{
  return boost::mp::type_list<Tx..., Ts...>{};
};

template<auto v>
auto add = v | append<void>;

static_assert(add<boost::mp::type_list<int, double>{}> == boost::mp::type_list<int, double, void>{});

template <class... Tx>
constexpr auto push_back =
    []<class... Ts> { return mp::type_list<Ts..., Tx...>{}; };

template <auto... Tx>
constexpr auto push_back_v =
    []<auto... Ts> { return mp::value_list<Ts..., Tx...>{}; };
