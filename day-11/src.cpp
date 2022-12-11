#include "../common/common.hpp"

#include <algorithm>
#include <deque>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

using item_t = size_t;

struct Monkey {
  std::deque<item_t> items;
  std::function<item_t(item_t)> op;
  item_t test_div, true_dest, false_dest, num_inspected{};
};

enum struct Ops { Plus, Mult };

auto parseMonkey(std::string_view section) -> Monkey {
  auto lines = splitIntoLinesUntilEmpty(section);
  Monkey retval;
  auto line_it = std::next(lines.begin());

  // Items
  {
    auto line = *line_it++;
    const auto items_start = line.find_first_of("123456789"sv);
    line.remove_prefix(items_start);
    std::ranges::copy(line | std::views::split(", "sv) |
                          std::views::transform(
                              [](auto &&num) { return toNumber<item_t>(num); }),
                      std::back_inserter(retval.items));
  }

  // Operation
  {
    auto line = *line_it++;
    auto words = splitLineIntoWordsFilterEmpty(line);
    auto word_it = std::next(words.begin(), 3);
    const auto arg1 = *word_it++;
    const auto op_str = *word_it++;
    const auto arg2 = *word_it;
    static constexpr auto old = std::numeric_limits<item_t>::max();
    retval.op = [al = (arg1 == "old"sv ? old : toNumber<item_t>(arg1)),
                 ar = (arg2 == "old"sv ? old : toNumber<item_t>(arg2)),
                 op = (op_str == "+"sv ? Ops::Plus : Ops::Mult)](auto arg) {
      const auto a1 = (al == old ? arg : al);
      const auto a2 = (ar == old ? arg : ar);
      if (op == Ops::Plus)
        return a1 + a2;
      else
        return a1 * a2;
    };
  }

  // Test + targets
  constexpr auto getLastNum = [](std::string_view line) {
    auto words = splitLineIntoWordsFilterEmpty(line);
    auto prev = words.begin();
    for (auto i = prev; i != words.end(); ++i)
      prev = i;
    return toNumber<item_t>(*prev);
  };
  retval.test_div = getLastNum(*line_it++);
  retval.true_dest = getLastNum(*line_it++);
  retval.false_dest = getLastNum(*line_it);

  return retval;
}

class Game {
public:
  Game(std::string_view data) {
    std::ranges::transform(splitIntoSections(data),
                           std::back_inserter(monkeys_), &parseMonkey);
    auto div_range =
        monkeys_ |
        std::views::transform([](const Monkey &m) { return m.test_div; }) |
        std::views::common;
    div_prod_ = std::reduce(div_range.begin(), div_range.end(), item_t{1},
                            std::multiplies{});
  }
  void roundDropWorry() {
    std::ranges::for_each(monkeys_, [&](Monkey &monkey) {
      for (auto item : monkey.items) {
        item = monkey.op(item);
        item /= 3;
        monkeys_[item % monkey.test_div == 0 ? monkey.true_dest
                                             : monkey.false_dest]
            .items.push_back(item);
        ++monkey.num_inspected;
      }
      monkey.items.clear();
    });
  }
  void roundConstWorry() {
    std::ranges::for_each(monkeys_, [&](Monkey &monkey) {
      for (auto item : monkey.items) {
        item = monkey.op(item);
        if (item > div_prod_)
          item %= div_prod_;
        monkeys_[item % monkey.test_div == 0 ? monkey.true_dest
                                             : monkey.false_dest]
            .items.push_back(item);
        ++monkey.num_inspected;
      }
      monkey.items.clear();
    });
  }
  size_t getMonkeyBusiness() const {
    // -Warray-bounds triggers for size == 2, super weird
    std::array<item_t, 3> top2{};
    std::ranges::partial_sort_copy(
        monkeys_ | std::views::transform(
                       [](const Monkey &m) { return m.num_inspected; }),
        top2, std::greater{});
    return top2[0] * top2[1];
  }

private:
  std::vector<Monkey> monkeys_;
  size_t div_prod_{};
};

void part1(std::string_view data) {
  Game game(data);
  for (int round = 0; round < 20; ++round)
    game.roundDropWorry();
  std::cout << game.getMonkeyBusiness() << '\n';
}

void part2(std::string_view data) {
  Game game(data);
  for (int round = 0; round < 10'000; ++round)
    game.roundConstWorry();
  std::cout << game.getMonkeyBusiness() << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  part1(data);
  part2(data);
}
