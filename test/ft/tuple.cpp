#include <algorithm>
#include <boost/mp.hpp>
#include <boost/ut.hpp>
#include <ranges>

int main() {
  using namespace boost::ut;
  using boost::mp::operator|;
  using boost::mp::ct;

  "tuple.reverse"_test = [] {
    expect((std::tuple{1} | std::ranges::views::reverse) == std::tuple{1});
    expect((std::tuple{1, 2} | std::ranges::views::reverse) ==
           std::tuple{2, 1});
    expect((std::tuple{1, 2., "3"} | std::ranges::views::reverse) ==
           std::tuple{"3", 2., 1});
  };

  "tuple.take"_test = [] {
    expect((std::tuple{1, 2, 3} | std::ranges::views::take(ct<1>)) ==
           std::tuple{1});
    expect((std::tuple{1, 2, 3} | std::ranges::views::take(ct<2>)) ==
           std::tuple{1, 2});
  };

  "tuple.drop"_test = [] {
    expect((std::tuple{1, 2, 3} | std::ranges::views::drop(ct<1>)) ==
           std::tuple{2, 3});
    expect((std::tuple{1, 2, 3} | std::ranges::views::drop(ct<2>)) ==
           std::tuple{3});
  };

  "tuple.take.drop"_test = [] {
    expect((std::tuple{1, 2, 3} | std::ranges::views::drop(ct<1>) |
            std::ranges::views::take(ct<1>)) == std::tuple{2});
  };
}
