#include <boost/mp.hpp>
#include <ranges>

auto reverse = []<class... Ts> {
  return boost::mp::list<Ts...>() | std::ranges::views::reverse;
};

static_assert((boost::mp::list<int, double, float>() | reverse) ==
              boost::mp::list<float, double, int>());

int main() {}
