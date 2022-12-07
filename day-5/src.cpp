#include "../common/common.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

struct Move {
  int from, to, count;
};

class Stacks {
public:
  Stacks(std::string_view section) {
    auto lines = splitIntoLinesUntilEmpty(section);
    const auto n_lines = std::ranges::distance(lines);
    for (auto &&line : lines | std::views::take(n_lines - 1)) {
      for (auto it = std::ranges::begin(line), end = std::ranges::end(line);;
           ++it) {
        it =
            std::find_if(it, end, [](char c) { return c >= 'A' and c <= 'Z'; });
        if (it == end)
          break;
        const auto pos = std::distance(std::ranges::begin(line), it);
        const auto stack_ind = (pos - 1) / 4;
        if (data_.size() <= static_cast<size_t>(stack_ind))
          data_.resize(stack_ind + 1);
        data_[stack_ind].push_back(*it);
      }
    }
    for (auto &stack : data_)
      std::ranges::reverse(stack);
  }

  auto getTops() const -> std::string {
    std::string retval;
    retval.reserve(data_.size());
    std::ranges::transform(data_, std::back_inserter(retval),
                           [](const auto &stack) { return stack.back(); });
    return retval;
  }
  void moveSeq(Move move) {
    auto &src = data_[move.from];
    auto &dest = data_[move.to];
    std::ranges::copy(src | std::views::reverse | std::views::take(move.count),
                      std::back_inserter(dest));
    src.erase(std::prev(src.end(), move.count), src.end());
  }
  void moveAll(Move move) {
    auto &src = data_[move.from];
    auto &dest = data_[move.to];
    std::ranges::reverse_copy(src | std::views::reverse |
                                  std::views::take(move.count),
                              std::back_inserter(dest));
    src.erase(std::prev(src.end(), move.count), src.end());
  }

private:
  std::vector<std::vector<char>> data_;
};

auto parseMove(auto &&line) -> Move {
  auto words = splitLineIntoWordsFilterEmpty(line);
  auto it = std::ranges::begin(words);
  Move retval;
  ++it;
  retval.count = toInt(*it++);
  ++it;
  retval.from = toInt(*it++) - 1;
  ++it;
  retval.to = toInt(*it) - 1;
  return retval;
}

void part1(std::string_view data) {
  auto parts = splitIntoSections(data);
  auto it = std::ranges::begin(parts);
  auto stacks = Stacks{*it++};
  std::ranges::for_each(splitIntoLinesUntilEmpty(*it) |
                            std::views::transform([](std::string_view line) {
                              return parseMove(line);
                            }),
                        [&stacks](Move m) { stacks.moveSeq(m); });
  std::cout << stacks.getTops() << '\n';
}

void part2(std::string_view data) {
  auto parts = splitIntoSections(data);
  auto it = std::ranges::begin(parts);
  auto stacks = Stacks{*it++};
  std::ranges::for_each(splitIntoLinesUntilEmpty(*it) |
                            std::views::transform([](std::string_view line) {
                              return parseMove(line);
                            }),
                        [&stacks](Move m) { stacks.moveAll(m); });
  std::cout << stacks.getTops() << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  part1(data);
  part2(data);
}
