#include <boost/mp.hpp>
#include <boost/ut.hpp>
#include <type_traits>

int main() {
  using namespace boost::ut;

  "utility.nth_pack_element"_test = [] {
    expect(std::is_same_v<
           int, boost::mp::utility::nth_pack_element<0, int, void, double>>);
    expect(std::is_same_v<
           void, boost::mp::utility::nth_pack_element<1, int, void, double>>);
    expect(std::is_same_v<
           double, boost::mp::utility::nth_pack_element<2, int, void, double>>);
  };

  "utility.nth_pack_element_V"_test = [] {
    expect(constant<1 == boost::mp::utility::nth_pack_element_v<0, 1, 2, 3>>);
    expect(constant<2 == boost::mp::utility::nth_pack_element_v<1, 1, 2, 3>>);
    expect(constant<3 == boost::mp::utility::nth_pack_element_v<2, 1, 2, 3>>);
  };
}
