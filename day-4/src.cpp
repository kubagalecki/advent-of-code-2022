#include "../common/common.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>

auto parseLoHi(auto &&str) -> std::array<int, 2> {
  auto lohi_pairs = str | std::views::split("-"sv);
  auto it = std::ranges::begin(lohi_pairs);
  const auto lo = toInt(*it++);
  const auto hi = toInt(*it);
  return std::array{lo, hi};
}

auto getAssignmentsView(std::string_view data) {
  return data | std::views::split("\n"sv) |
         std::views::take_while(
             [](auto &&subr) { return not std::ranges::empty(subr); }) |
         std::views::transform([](auto &&line) {
           auto pairs = line | std::views::split(","sv);
           auto it = std::ranges::begin(pairs);
           const auto first_lohi = parseLoHi(*it++);
           const auto second_lohi = parseLoHi(*it);
           return std::make_pair(first_lohi, second_lohi);
         });
}

void part1(std::string_view data) {
  const auto num_overlapping =
      std::ranges::count_if(getAssignmentsView(data), [](const auto &pair) {
        const auto &[p1, p2] = pair;
        const auto [lo1, hi1] = p1;
        const auto [lo2, hi2] = p2;
        return (lo1 <= lo2 and hi1 >= hi2) or (lo2 <= lo1 and hi2 >= hi1);
      });
  std::cout << num_overlapping << '\n';
}

void part2(std::string_view data) {
  const auto num_subsuming =
      std::ranges::count_if(getAssignmentsView(data), [](const auto &pair) {
        const auto &[p1, p2] = pair;
        const auto [lo1, hi1] = p1;
        const auto [lo2, hi2] = p2;
        return (lo1 <= hi2 and hi1 >= lo2) or (lo2 <= hi1 and hi2 >= lo1);
      });
  std::cout << num_subsuming << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  part1(data);
  part2(data);
}
