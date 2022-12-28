#include "../common/common.hpp"

#include <unordered_set>

auto parseMoves(std::string_view data) {
  return splitIntoLinesUntilEmpty(data) |
         std::views::transform([](std::string_view line) {
           constexpr auto parseDir =
               [](std::string_view dir) -> std::array<ptrdiff_t, 2> {
             if (dir == "L"sv)
               return {-1, 0};
             else if (dir == "R"sv)
               return {1, 0};
             else if (dir == "D"sv)
               return {0, -1};
             else if (dir == "U"sv)
               return {0, 1};
             else
               throw std::runtime_error{"Incorrect direction"};
           };
           constexpr auto parseSteps = [](std::string_view steps) {
             return toNumber<size_t>(steps);
           };
           auto words = splitLineIntoWordsFilterEmpty(line);
           return std::make_pair(parseDir(*words.begin()),
                                 parseSteps(*std::next(words.begin())));
         });
}

ptrdiff_t sgn(ptrdiff_t val) { return (0 < val) - (val < 0); }

template <size_t num_knots = 2>
  requires(num_knots > 0)
class Chain {
public:
  using pos_t = std::array<ptrdiff_t, 2>;

  void moveHead(pos_t move) {
    pos_.front().front() += move.front();
    pos_.front().back() += move.back();
    updateRope();
  }
  size_t getNumTailPos() const { return tail_hist_.size(); }

private:
  static pos_t getDiff(pos_t lead, pos_t follow) {
    pos_t retval;
    std::ranges::transform(lead, follow, retval.begin(), std::minus{});
    return retval;
  }
  static void updateFollow(pos_t lead, pos_t &follow) {
    auto &[tx, ty] = follow;
    const auto [dx, dy] = getDiff(lead, follow);
    if (std::abs(dx) == 2 or std::abs(dy) == 2) {
      tx += sgn(dx);
      ty += sgn(dy);
    }
  }
  void saveTailPos() { tail_hist_.insert(pos_.back()); }
  void updateRope() {
    for (auto it :
         std::views::iota(pos_.begin()) | std::views::take(pos_.size() - 1))
      updateFollow(*it, *std::next(it));
    saveTailPos();
  }

  using hash_t = decltype([](pos_t key) {
    std::hash<ptrdiff_t> hash;
    return hash(key.front()) ^ hash(key.back());
  });

  std::array<pos_t, num_knots> pos_{};
  std::unordered_set<pos_t, hash_t> tail_hist_;
};

void part1(auto &&moves) {
  Chain<2> chain;
  for (const auto &[move, steps] : moves)
    for (size_t i = 0; i < steps; ++i)
      chain.moveHead(move);
  std::cout << chain.getNumTailPos() << '\n';
}

void part2(auto &&moves) {
  Chain<10> chain;
  for (const auto &[move, steps] : moves)
    for (size_t i = 0; i < steps; ++i)
      chain.moveHead(move);
  std::cout << chain.getNumTailPos() << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  auto moves_view = parseMoves(data);
  part1(moves_view);
  part2(moves_view);
}
