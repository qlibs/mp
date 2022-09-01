#include <algorithm>
#include <boost/mp.hpp>
#include <boost/ut.hpp>

int main() {
  using namespace boost::ut;

  auto sort = []<auto... Vs>(boost::mp::concepts::meta auto types) {
    std::sort(std::begin(types), std::end(types),
              [values = std::array{Vs...}](auto lhs, auto rhs) {
                return values[lhs] < values[rhs];
              });
    return types;
  };

  auto unique = []<auto... Vs>(boost::mp::concepts::meta auto types) {
    types.erase(std::unique(std::begin(types), std::end(types),
                            [values = std::array{Vs...}](auto lhs, auto rhs) {
                              return values[lhs] == values[rhs];
                            }),
                std::end(types));

    return types;
  };

  expect(constant<($(1, 2) | sort | unique) == $(1, 2)>);
  expect(constant<($(1, 2, 2) | sort | unique) == $(1, 2)>);
  expect(constant<($(1, 1, 2, 2) | sort | unique) == $(1, 2)>);
  expect(constant<($(1, 2, 1, 2) | sort | unique) == $(1, 2)>);
  expect(constant<($(1, 2, 3) | sort | unique) == $(1, 2, 3)>);
  expect(constant<($(1, 3, 2) | sort | unique) == $(1, 2, 3)>);
  expect(constant<($(2, 2, 1, 1, 3) | sort | unique) == $(1, 2, 3)>);
}
