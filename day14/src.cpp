#include "../common/common.hpp"

class Board {
  ptrdiff_t computeOffset(ptrdiff_t x, ptrdiff_t y) const {
    // Column major order
    auto off_x = x - x_min_;
    auto off_y = y - y_min_;
    return off_y + off_x * height_;
  }

public:
  enum struct Field : std::uint8_t { SandSource, Rock, Sand, Empty };
  enum struct SandStatus { FellDown, FellLeft, FellRight, Landed, FailedSpawn };

  Board(ptrdiff_t x_min, ptrdiff_t x_max, ptrdiff_t y_min, ptrdiff_t y_max)
      : x_min_{x_min}, width_{x_max - x_min + 1}, y_min_{y_min}, height_{y_max -
                                                                         y_min +
                                                                         1},
        fields_(width_ * height_, Field::Empty) {}

  Field operator()(ptrdiff_t x, ptrdiff_t y) const {
    return fields_[computeOffset(x, y)];
  }
  Field &operator()(ptrdiff_t x, ptrdiff_t y) {
    return fields_[computeOffset(x, y)];
  }
  SandStatus dropSand(ptrdiff_t x, ptrdiff_t y) {
    const auto getFieldAt = [&](ptrdiff_t px, ptrdiff_t py) -> Field & {
      return this->operator()(px, py);
    };
    auto &spawn = getFieldAt(x, y);
    if (spawn == Field::Rock)
      throw std::runtime_error{"Spawning sand inside a rock"};
    if (spawn == Field::Sand)
      return SandStatus::FailedSpawn;
    const auto x_max = x_min_ + width_ - 1, y_max = y_min_ + height_ - 1;
    for (;;) {
      if (y + 1 > y_max)
        return SandStatus::FellDown;
      else if (getFieldAt(x, y + 1) == Field::Empty)
        ++y;
      else if (x - 1 < x_min_)
        return SandStatus::FellLeft;
      else if (getFieldAt(x - 1, y + 1) == Field::Empty) {
        --x;
        ++y;
      } else if (x + 1 > x_max)
        return SandStatus::FellRight;
      else if (getFieldAt(x + 1, y + 1) == Field::Empty) {
        ++x;
        ++y;
      } else {
        getFieldAt(x, y) = Field::Sand;
        return SandStatus::Landed;
      }
    }
  }
  void draw() const {
    for (auto y = y_min_; y < y_min_ + height_; ++y) {
      for (auto x = x_min_; x < x_min_ + width_; ++x)
        switch (this->operator()(x, y)) {
        case Field::Empty:
          std::cout << '.';
          break;
        case Field::Sand:
          std::cout << 'o';
          break;
        case Field::Rock:
          std::cout << '#';
          break;
        case Field::SandSource:
          std::cout << '+';
          break;
        }
      std::cout << '\n';
    }
    std::cout << '\n';
  }

private:
  ptrdiff_t x_min_, width_, y_min_, height_;
  std::vector<Field> fields_;
};

auto parseChains(std::string_view data, ptrdiff_t x_src, ptrdiff_t y_src) {
  std::vector<std::vector<std::array<ptrdiff_t, 2>>> chains;
  ptrdiff_t x_min = x_src, x_max = x_src, y_min = y_src, y_max = y_src;
  for (auto line : splitIntoLinesUntilEmpty(data)) {
    auto &current_line = chains.emplace_back();
    for (auto pair : splitLineIntoWordsFilterEmpty(line) |
                         std::views::filter([](std::string_view word) {
                           return word != "->"sv;
                         })) {
      auto nums = pair | std::views::split(',');
      const auto x = toNumber<ptrdiff_t>(*nums.begin());
      const auto y = toNumber<ptrdiff_t>(*std::next(nums.begin()));
      x_min = std::min(x_min, x);
      x_max = std::max(x_max, x);
      y_min = std::min(y_min, y);
      y_max = std::max(y_max, y);
      current_line.push_back(std::array{x, y});
    }
  }
  return std::make_pair(std::move(chains),
                        std::array{x_min, x_max, y_min, y_max});
}

auto fillBoard(
    Board &board,
    const std::vector<std::vector<std::array<ptrdiff_t, 2>>> &chains) {
  for (const auto &chain : chains) {
    for (auto it = chain.begin(); it != std::prev(chain.end());) {
      const auto [x1, y1] = *it++;
      const auto [x2, y2] = *it;
      if (x1 == x2)
        for (auto y = std::min(y1, y2); y <= std::max(y1, y2); ++y)
          board(x1, y) = Board::Field::Rock;
      else if (y1 == y2)
        for (auto x = std::min(x1, x2); x <= std::max(x1, x2); ++x)
          board(x, y1) = Board::Field::Rock;
      else
        throw std::runtime_error{"Unaligned chain links"};
    }
  }
}

enum struct Draw { Yes, No };

void part1(const auto &parse_result, ptrdiff_t x_src, ptrdiff_t y_src,
           Draw do_draw = Draw::No) {
  const auto &[chains, limits] = parse_result;
  const auto [x_min, x_max, y_min, y_max] = limits;
  Board board{x_min, x_max, y_min, y_max};
  board(x_src, y_src) = Board::Field::SandSource;
  fillBoard(board, chains);

  if (do_draw == Draw::Yes)
    board.draw();

  size_t num_fallen{};
  while (board.dropSand(x_src, y_src) == Board::SandStatus::Landed)
    ++num_fallen;

  if (do_draw == Draw::Yes)
    board.draw();
  std::cout << num_fallen << '\n';
}

void part2(const auto &parse_result, ptrdiff_t x_src, ptrdiff_t y_src,
           Draw do_draw = Draw::No) {
  const auto &[chains, limits] = parse_result;
  const auto [x_min, x_max, y_min, y_max] = limits;
  const auto y_max_actual = y_max + 2;
  const auto fall_height = y_max_actual - y_src;
  const auto x_min_actual = std::min(x_min, x_src - fall_height - 1);
  const auto x_max_actual = std::max(x_max, x_src + fall_height + 1);

  Board board{x_min_actual, x_max_actual, y_min, y_max_actual};
  for (auto x = x_min_actual; x <= x_max_actual; ++x)
    board(x, y_max_actual) = Board::Field::Rock;
  board(x_src, y_src) = Board::Field::SandSource;
  fillBoard(board, chains);

  if (do_draw == Draw::Yes)
    board.draw();

  size_t num_fallen{};
  while (board.dropSand(x_src, y_src) != Board::SandStatus::FailedSpawn)
    ++num_fallen;

  if (do_draw == Draw::Yes)
    board.draw();
  std::cout << num_fallen << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  constexpr ptrdiff_t x_src = 500, y_src = 0;
  const auto parse_result = parseChains(data, x_src, y_src);
  part1(parse_result, x_src, y_src /*, Draw::Yes */);
  part2(parse_result, x_src, y_src /*, Draw::Yes */);
}
