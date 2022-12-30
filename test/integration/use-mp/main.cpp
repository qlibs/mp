#include <boost/mp.hpp>

int main() {
  constexpr auto correct = boost::mp::type_list{} == boost::mp::type_list{};
  return static_cast<int>(!correct);
}
