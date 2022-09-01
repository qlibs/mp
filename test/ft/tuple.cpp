#include <algorithm>
#include <boost/mp.hpp>
#include <boost/ut.hpp>
#include <ranges>

template <auto N>
static constexpr auto take = [] { return std::ranges::views::take(N); };

template <auto N>
static constexpr auto drop = [] { return std::ranges::views::drop(N); };

int main() {
  using namespace boost::ut;
  using boost::mp::operator|;

  "tuple.reverse"_test = [] {
    expect((std::tuple{1} | std::ranges::views::reverse) == std::tuple{1});
    expect((std::tuple{1, 2} | std::ranges::views::reverse) ==
           std::tuple{2, 1});
    expect((std::tuple{1, 2., "3"} | std::ranges::views::reverse) ==
           std::tuple{"3", 2., 1});
  };

  "tuple.take"_test = [] {
    expect((std::tuple{1, 2, 3} | take<1>) == std::tuple{1});
    expect((std::tuple{1, 2, 3} | take<2>) == std::tuple{1, 2});
  };

  "tuple.drop"_test = [] {
    expect((std::tuple{1, 2, 3} | drop<1>) == std::tuple{2, 3});
    expect((std::tuple{1, 2, 3} | drop<2>) == std::tuple{3});
  };

  "tuple.take.drop"_test = [] {
    expect((std::tuple{1, 2, 3} | drop<1> | take<1>) == std::tuple{2});
  };
}
