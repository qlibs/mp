#include <boost/mp.hpp>
#include <boost/ut.hpp>

int main() {
  using namespace boost::ut;

  "type_list"_test = [] {
    expect(boost::mp::type_list{} == boost::mp::type_list{});
    expect(boost::mp::type_list<void>{} == boost::mp::type_list<void>{});
    expect(boost::mp::type_list<int, double>{} ==
           boost::mp::type_list<int, double>{});
    expect(boost::mp::type_list<void>{} != boost::mp::type_list<int>{});
    expect(boost::mp::type_list<void, int>{} !=
           boost::mp::type_list<int, void>{});
  };

  struct structural {};

  "value_list"_test = [] {
    expect(boost::mp::value_list{} == boost::mp::value_list{});
    expect(boost::mp::value_list<1>{} == boost::mp::value_list<1>{});
    expect(boost::mp::value_list<1, 2>{} == boost::mp::value_list<1, 2>{});
    expect(boost::mp::value_list<2>{} != boost::mp::value_list<1, 2>{});
    expect(boost::mp::value_list<2, 1>{} != boost::mp::value_list<1, 2>{});
    expect(boost::mp::value_list<structural{}>{} ==
           boost::mp::value_list<structural{}>{});
    expect(boost::mp::value_list<structural{}>{} !=
           boost::mp::value_list<42>{});
  };

  "list"_test = [] {
    expect(boost::mp::list<int, void>() == boost::mp::type_list<int, void>{});
    expect(boost::mp::list<1, 2, 3>() == boost::mp::value_list<1, 2, 3>{});
    expect(boost::mp::list<structural{}, 1>() ==
           boost::mp::value_list<structural{}, 1>{});
  };
}
