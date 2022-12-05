#include "../common/common.hpp"

#include <algorithm>
#include <iostream>
#include <numeric>

auto getCaloryView(std::string_view data) {
  using namespace std::string_view_literals;
  return data | std::views::split("\n\n"sv) |
         std::views::transform([](auto subr) {
           auto lines_sz =
               std::forward<decltype(subr)>(subr) | std::views::split("\n"sv) |
               std::views::transform([](auto &&line) {
                 return toInt(line);
               }) |
               std::views::common;
           return std::reduce(std::ranges::begin(lines_sz),
                              std::ranges::end(lines_sz));
         });
}

void part1() {
  const auto [alloc, data] = getStdinView();
  const auto max_cal = std::ranges::max(getCaloryView(data));
  std::cout << max_cal << '\n';
}

void part2() {
  const auto [alloc, data] = getStdinView();
  std::array<int, 3> top3{};
  std::ranges::partial_sort_copy(getCaloryView(data), top3, std::greater<>{});
  std::cout << std::reduce(top3.begin(), top3.end()) << '\n';
}

int main() {
  part1();
  part2();
}
