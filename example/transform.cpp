#include <boost/mp.hpp>

template <auto Fn>
auto transform = []<class... Ts> {
  return boost::mp::type_list<decltype(Fn.template operator()<Ts>())...>{};
};

auto add_pointer = []<class T> -> T* { return {}; };

static_assert((boost::mp::type_list<int, double>{} | transform<add_pointer>) ==
              boost::mp::type_list<int*, double*>{});

int main() {}
