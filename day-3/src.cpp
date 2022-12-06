#include "../common/common.hpp"

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <numeric>
#include <ranges>

constexpr auto priority_map = std::invoke([] {
  std::array<char, 256> retval{};
  char p = 1;
  for (char l : std::views::iota('a') | std::views::take(26))
    retval[l] = p++;
  for (char l : std::views::iota('A') | std::views::take(26))
    retval[l] = p++;
  return retval;
});

auto getRucksackView(std::string_view data) {
  return data | std::views::split("\n"sv) |
         std::views::take_while(
             [](auto &&subr) { return not std::ranges::empty(subr); });
}

void part1(std::string_view data) {
  auto priority_view =
      getRucksackView(data) | std::views::transform([](auto &&ruck) {
        std::array<bool, 256> content_map{};
        const auto half_sz = std::ranges::size(ruck) / 2;
        for (auto c : ruck | std::views::take(half_sz))
          content_map[c] = true;
        const auto repeated =
            *std::ranges::find_if(ruck | std::views::drop(half_sz),
                                  [&](char c) { return content_map[c]; });
        return priority_map[repeated];
      }) |
      std::views::common;
  std::cout << std::reduce(priority_view.begin(), priority_view.end(), size_t{})
            << '\n';
}

void part2(std::string_view data) {
  std::vector<std::string_view> lines;
  std::ranges::transform(getRucksackView(data) | std::views::common,
                         std::back_inserter(lines), [](auto &&subr) {
                           return std::string_view{subr.begin(), subr.end()};
                         });
  int sum{};
  for (size_t i = 0; i < lines.size(); i += 3) {
    std::array<std::array<bool, 2>, 256> content_map{};
    for (size_t j = 0;
         auto line : lines | std::views::drop(i) | std::views::take(2)) {
      for (auto c : line)
        content_map[c][j] = true;
      ++j;
    }
    const auto repeated = *std::ranges::find_if(lines[i + 2], [&](char c) {
      return std::ranges::all_of(content_map[c], std::identity{});
    });
    sum += priority_map[repeated];
  }
  std::cout << sum << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  part1(data);
  part2(data);
}
