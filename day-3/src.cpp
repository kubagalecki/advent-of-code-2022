#include "../common/common.hpp"

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <numeric>
#include <ranges>

constexpr auto priority_map = std::invoke([] {
  std::array<char, 256> retval;
  for (char p = 1;
       char l : std::array{std::views::iota('a') | std::views::take(26),
                           std::views::iota('A') | std::views::take(26)} |
                    std::views::join)
    retval[l] = p++;
  return retval;
});

auto getRucksackView(std::string_view data) {
  using namespace std::string_view_literals;
  return data | std::views::split("\n"sv) | std::views::filter([](auto &&subr) {
           return not std::ranges::empty(subr);
         });
}

void part1() {
  const auto [alloc, data] = getStdinView();
  auto priority_view =
      getRucksackView(data) | std::views::transform([](auto &&ruck) {
        std::array<bool, 256> content_map{};
        const auto ruck_sz = std::ranges::size(ruck);
        for (auto c : ruck | std::views::take(ruck_sz / 2))
          content_map[c] = true;
        const auto repeated =
            *std::ranges::find_if(ruck | std::views::drop(ruck_sz / 2),
                                  [&](char c) { return content_map[c]; });
        return priority_map[repeated];
      }) |
      std::views::common;
  std::cout << std::reduce(std::ranges::begin(priority_view),
                           std::ranges::end(priority_view), size_t{})
            << '\n';
}

void part2() {
  const auto [alloc, data] = getStdinView();
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
  part1();
  part2();
}
