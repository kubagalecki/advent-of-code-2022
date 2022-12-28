#include "../common/common.hpp"

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

auto getWinningShape(Shape shape) -> Shape {
  return mapToVals(shape, shapes, beating_shapes);
}
auto getLosingShape(Shape shape) -> Shape {
  return mapToVals(shape, beating_shapes, shapes);
}

template <typename R1, typename R2>
auto getStrategyView(std::string_view data, const std::array<R1, 3> &r1,
                     const std::array<R2, 3> &r2) {
  static constexpr auto map1 = std::array{'A', 'B', 'C'};
  static constexpr auto map2 = std::array{'X', 'Y', 'Z'};
  return splitIntoLinesUntilEmpty(data) |
         std::views::transform([&](std::string_view line) {
           return std::make_pair(
               mapToVals(*std::ranges::cbegin(line), map1, r1),
               mapToVals(*std::ranges::crbegin(line), map2, r2));
         });
}

int calculateScore(Shape opponent, Shape me) {
  const int base = mapToVals(me, shapes, std::array{1, 2, 3});
  int matchup{};
  if (opponent == me)
    matchup = 3;
  else if (opponent == getLosingShape(me))
    matchup = 6;
  return base + matchup;
}

void part1(std::string_view data) {
  auto score_view = getStrategyView(data, shapes, shapes) |
                    std::views::transform([](auto pair) {
                      const auto [oppo, me] = pair;
                      return calculateScore(oppo, me);
                    }) |
                    std::views::common;
  std::cout << std::reduce(score_view.begin(), score_view.end()) << '\n';
}

Shape calculateChoice(Shape opponent, Outcome expected) {
  if (expected == Outcome::Draw)
    return opponent;
  else if (expected == Outcome::Win)
    return getWinningShape(opponent);
  else
    return getLosingShape(opponent);
}

void part2(std::string_view data) {
  auto score_view = getStrategyView(data, shapes, outcomes) |
                    std::views::transform([](auto pair) {
                      const auto [oppo, expected_result] = pair;
                      const auto me = calculateChoice(oppo, expected_result);
                      return calculateScore(oppo, me);
                    }) |
                    std::views::common;
  std::cout << std::reduce(score_view.begin(), score_view.end()) << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  part1(data);
  part2(data);
}
