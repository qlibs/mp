
auto tup = [](auto t) {
  return t | std::ranges::views::reverse |
         [](auto... args) { return std::tuple{(args * 2)...}; };
};
