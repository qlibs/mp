#include <boost/mp.hpp>
#include <ranges>

template <auto N>
auto nth_pack_element = []<class... Ts> {
  return boost::mp::list<Ts...>() | [] { return std::ranges::views::drop(N); } |
         [] { return std::ranges::views::take(1); } |
         []<class T> { return T{}; };
};

static_assert((boost::mp::list<int, double, float>() | nth_pack_element<0>) ==
              int{});
static_assert((boost::mp::list<int, double, float>() | nth_pack_element<1>) ==
              double{});
static_assert((boost::mp::list<int, double, float>() | nth_pack_element<2>) ==
              float{});

int main() {}
