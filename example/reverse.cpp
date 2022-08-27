template <auto... Ns>
auto xx = mp::value_list<Ns...>{} | std::ranges::views::reverse;

