#include "../common/common.hpp"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <ranges>

enum struct Shape { Rock, Paper, Scissors };
enum struct Outcome { Win, Lose, Draw };
constexpr auto shapes = std::array{Shape::Rock, Shape::Paper, Shape::Scissors};
constexpr auto beating_shapes =
    std::array{Shape::Paper, Shape::Scissors, Shape::Rock};
constexpr auto outcomes =
    std::array{Outcome::Lose, Outcome::Draw, Outcome::Win};

template <typename In, typename Out>
auto mapToVals(In in_val, const std::array<In, 3> &in,
               const std::array<Out, 3> &out) {
  const auto in_it = std::ranges::find(in, in_val);
  if (in_it == in.end())
    throw std::runtime_error{"invalid input"};
  return out[std::distance(in.begin(), in_it)];
}

template <typename R1, typename R2>
auto getStrategyView(std::string_view data, const std::array<R1, 3> &r1,
                     const std::array<R2, 3> &r2) {
  using namespace std::string_view_literals;
  return data | std::views::split("\n"sv) | std::views::filter([](auto &&subr) {
           return not std::ranges::empty(subr);
         }) |
         std::views::transform([&](auto &&subr) {
           const auto map1 = std::array{'A', 'B', 'C'};
           const auto map2 = std::array{'X', 'Y', 'Z'};
           return std::make_pair(
               mapToVals(*std::ranges::cbegin(subr), map1, r1),
               mapToVals(*std::ranges::crbegin(subr), map2, r2));
         });
}

int calculateScore(Shape opponent, Shape me) {
  const int base = mapToVals(me, shapes, std::array{1, 2, 3});
  int matchup{};
  if (opponent == me)
    matchup = 3;
  else if (opponent == mapToVals(me, beating_shapes, shapes))
    matchup = 6;
  return base + matchup;
}

void part1() {
  const auto [alloc, data] = getStdinView();
  auto score_view = getStrategyView(data, shapes, shapes) |
                    std::views::transform([](auto pair) {
                      const auto [oppo, me] = pair;
                      return calculateScore(oppo, me);
                    }) |
                    std::views::common;
  std::cout << std::reduce(std::ranges::begin(score_view),
                           std::ranges::end(score_view))
            << '\n';
}

Shape calculateChoice(Shape opponent, Outcome expected) {
  if (expected == Outcome::Draw)
    return opponent;
  else if (expected == Outcome::Win)
    return mapToVals(opponent, shapes, beating_shapes);
  else
    return mapToVals(opponent, beating_shapes, shapes);
}

void part2() {
  const auto [alloc, data] = getStdinView();
  auto score_view = getStrategyView(data, shapes, outcomes) |
                    std::views::transform([](auto pair) {
                      const auto [oppo, expected_result] = pair;
                      const auto me = calculateChoice(oppo, expected_result);
                      return calculateScore(oppo, me);
                    }) |
                    std::views::common;
  std::cout << std::reduce(std::ranges::begin(score_view),
                           std::ranges::end(score_view))
            << '\n';
}

int main() {
  part1();
  part2();
}
