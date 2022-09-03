#include <algorithm>
#include <boost/mp.hpp>
#include <boost/ut.hpp>

int main() {
  using namespace boost::ut;

  "value_list.<Vs...>"_test = [] {
    expect(constant<(boost::mp::value_list{} | []<auto... Vs> {
                      return boost::mp::value_list<Vs...>{};
                    }) == boost::mp::value_list{}>);
    expect(constant<($(1, 2) | []<auto... Vs> {
                      return boost::mp::value_list<Vs...>{};
                    }) == $(1, 2)>);
  };

  "value_list.(types)"_test = [] {
    expect(constant<(boost::mp::value_list{} |
                     [](boost::mp::concepts::meta auto types) {
                       return types;
                     }) == boost::mp::value_list{}>);
    expect(constant<($(1, 2) | [](boost::mp::concepts::meta auto types) {
                      return types;
                    }) == $(1, 2)>);
  };

  "value_list.<Vs...>(types)"_test = [] {
    expect(constant<(boost::mp::value_list{} |
                     []<auto... Vs>(boost::mp::concepts::meta auto types) {
                       return types;
                     }) == boost::mp::value_list{}>);
    expect(constant<($(1, 2) |
                     []<auto... Vs>(boost::mp::concepts::meta auto types) {
                       return types;
                     }) == $(1, 2)>);
  };

  "value_list.compund"_test = [] {
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
  };
}
