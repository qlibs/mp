#include <boost/mp.hpp>

template <class... TRhs>
auto append =
    []<class... TLhs> { return boost::mp::type_list<TLhs..., TRhs...>{}; };

template <auto v>
auto add = v | append<void>;

static_assert(add<boost::mp::type_list<int, double>{}> ==
              boost::mp::type_list<int, double, void>{});

int main() {}
