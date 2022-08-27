#include <boost/mp.hpp>

template <class... Ts>
auto insert =
    boost::mp::type_list<Ts...>{}
  | [] { return std::ranges::views::drop(1); } | []<class... Rs> {
                 auto r2 = mp::type_list<Rs...>{} |
                           [] { return std::ranges::views::drop(1); };
                 return mp::type_list<Rs...>{} |
                        [] { return std::ranges::views::take(2); } |
                        push_back<float> | append(r2) |
                        []<class... Xs> {
                          auto rest = mp::type_list<Xs...>{} | [] {
                            return std::ranges::views::drop(sizeof...(Xs) - 2);
                          } | []<class X, class... Cs> {
                            return mp::type_list<X(), Cs...>{};
                          };

                          return mp::type_list<Xs...>{} | [] {
                            return std::ranges::views::take(sizeof...(Xs) - 2);
                          } | append(rest);
                        } |
                        append(mp::to_list<foo>) | push_back<void>;
               };
      else
        return mp::type_list<Us...>{};
    };


