#include "../common/common.hpp"

auto getCaloryView(std::string_view data) {
  return splitIntoSections(data) |
         std::views::transform([](std::string_view sec) {
           auto line_sums = splitIntoLinesUntilEmpty(sec) |
                            std::views::transform([](std::string_view line) {
                              return toInt(line);
                            }) |
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
