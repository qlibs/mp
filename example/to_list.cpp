#include <boost/mp.hpp>
#include <bit>

template<auto Fn>
auto sort = []<class... Ts>(boost::mp::concepts::meta auto types) {
  std::sort(std::begin(types), std::end(types), Fn);
  return types;
};

auto by_size = [](auto lhs, auto rhs) { return lhs.size < rhs.size; };

template<class T>
auto pack = boost::mp::to_list<T>
          | sort<by_size>
          | []<class... Ts> { return std::tuple<Ts...>{}; };
;

struct not_packed {
  char c{};
  int i{};
  std::byte b{};
};

static_assert(sizeof(not_packed) == 12uz);
static_assert(sizeof(pack<not_packed>) == 8uz);
