#include "../common/common.hpp"

#include <algorithm>
#include <iostream>
#include <numeric>

auto getCaloryView(std::string_view data) {
  return data | std::views::split("\n\n"sv) |
         std::views::transform([](auto &&subr) {
           auto line_sums =
               subr | std::views::split("\n"sv) |
               std::views::transform([](auto &&line) { return toInt(line); }) |
               std::views::common;
           return std::reduce(line_sums.begin(), line_sums.end());
         });
}

void part1(std::string_view data) {
  const auto max_cal = std::ranges::max(getCaloryView(data));
  std::cout << max_cal << '\n';
}

void part2(std::string_view data) {
  std::array<int, 3> top3{};
  std::ranges::partial_sort_copy(getCaloryView(data), top3, std::greater{});
  std::cout << std::reduce(top3.begin(), top3.end()) << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  part1(data);
  part2(data);
}
