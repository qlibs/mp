#include <boost/mp.hpp>
#include <algorithm>
#include <typeinfo>

template<auto Fn>
auto sort = []<class... Ts>(boost::mp::concepts::meta auto types) {
  std::sort(std::begin(types), std::end(types), Fn);
  return types;
};

auto by_size = [](auto lhs, auto rhs) { return lhs.size < rhs.size; };

auto unique = []<class... Ts>(boost::mp::concepts::meta auto types) {
  types.erase(std::unique(std::begin(types), std::end(types),
                          [](auto lhs, auto rhs) { return lhs.type == rhs.type; }),
              std::end(types));
  return types;
};

template<auto v>
auto unique_list = v
                 | sort<by_size>
                 | unique
;

static_assert(unique_list<boost::mp::type_list<int, double, float, int>{}> == boost::mp::type_list<float, int, double>{});
