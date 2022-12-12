#include "../common/common.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <span>

bool isUnique(std::span<const char> input) {
  std::array<bool, 256> map{};
  return std::ranges::none_of(
      input, [&](char c) { return std::exchange(map[c], true); });
}

size_t firstUniqueAfter(std::string_view input, size_t window_size) {
  const auto span = std::span{input};
  for (size_t i = 0; i < span.size() - window_size; ++i)
    if (isUnique(span.subspan(i, window_size)))
      return i + window_size;
  return 0;
}

void part1(std::string_view data) {
  std::cout << firstUniqueAfter(data, 4) << '\n';
}

void part2(std::string_view data) {
  std::cout << firstUniqueAfter(data, 14) << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  part1(data);
  part2(data);
}
