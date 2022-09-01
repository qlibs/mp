#include <algorithm>
#include <boost/mp.hpp>
#include <boost/ut.hpp>
#include <ranges>

int main() {
  using namespace boost::ut;

  "type_list.ranges"_test = [] {
    expect(constant<($(int, double) | std::ranges::views::reverse) ==
                    $(double, int)>);
  };
}
