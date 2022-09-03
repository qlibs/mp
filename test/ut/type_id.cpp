#include <boost/mp.hpp>
#include <boost/ut.hpp>

int main() {
  using namespace boost::ut;

  "type_id"_test = [] {
    expect(
        constant<boost::mp::type_id<const void> != boost::mp::type_id<void>>);
    expect(constant<boost::mp::type_id<void> != boost::mp::type_id<int>>);
    expect(constant<boost::mp::type_id<int> != boost::mp::type_id<int&>>);

    expect(constant<boost::mp::type_id<void> == boost::mp::type_id<void>>);
    expect(constant<boost::mp::type_id<const int&> ==
                    boost::mp::type_id<const int&>>);
  };
}
