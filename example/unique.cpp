#include <algorithm>
#include <bit>
#include <boost/mp.hpp>

template <auto Fn>
auto sort = []<class... Ts>(boost::mp::concepts::meta auto types) {
  std::sort(std::begin(types), std::end(types), Fn);
  return types;
};

auto by_size = [](auto lhs, auto rhs) { return lhs.size < rhs.size; };

auto unique = []<class... Ts>(boost::mp::concepts::meta auto types) {
  types.erase(std::unique(std::begin(types), std::end(types),
                          [](auto lhs, auto rhs) { return lhs == rhs; }),
              std::end(types));
  return types;
};

template <auto v>
auto unique_sort = v | sort<by_size> | unique;

static_assert(
    unique_sort<boost::mp::type_list<std::byte[1], std::byte[3], std::byte[2],
                                     std::byte[1], std::byte[3]>{}> ==
    boost::mp::type_list<std::byte[1], std::byte[2], std::byte[3]>{});

int main() {}
