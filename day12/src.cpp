#include "../common/common.hpp"

#include <algorithm>
#include <deque>
#include <functional>
#include <iostream>
#include <numeric>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>

using pos_t = std::array<ptrdiff_t, 2>;
enum struct DrawResult { yes, no };

class Board {
public:
  Board(std::string_view input) {
    data_.reserve(input.size());
    ptrdiff_t line = 0;
    for (ptrdiff_t pos = 0; auto &&line_str : splitIntoLinesUntilEmpty(input)) {
      std::ranges::transform(line_str, std::back_inserter(data_), [&](char c) {
        const auto current_pos = pos++;
        switch (c) {
        case 'S':
          start_ = {current_pos - line * width_, line};
          return 'a';
        case 'E':
          finish_ = {current_pos - line * width_, line};
          return 'z';
        default:
          return c;
        }
      });
      if (line++ == 0)
        width_ = pos;
    }
    height_ = line;
    data_.shrink_to_fit();
  }
  char operator()(pos_t pos) const {
    return data_[pos.back() * width_ + pos.front()];
  }
  char &operator()(pos_t pos) {
    return data_[pos.back() * width_ + pos.front()];
  }
  void setStart(pos_t pos) { start_ = pos; }

  auto start() const { return start_; }
  auto finish() const { return finish_; }
  auto width() const { return width_; }
  auto height() const { return height_; }

  void print() {
    for (ptrdiff_t y = 0; y < height_; ++y) {
      for (ptrdiff_t x = 0; x < width_; ++x)
        if (pos_t{x, y} == start_)
          std::cout << 'S';
        else if (pos_t{x, y} == finish_)
          std::cout << 'E';
        else
          std::cout << (*this)(pos_t{x, y});
      std::cout << '\n';
    }
    std::cout << std::endl;
  }

private:
  std::vector<char> data_;
  ptrdiff_t width_{}, height_{};
  pos_t start_{}, finish_{};
};

// See https://en.wikipedia.org/wiki/A*_search_algorithm
std::optional<size_t> A_star(const Board &board,
                             DrawResult do_draw = DrawResult::no) {
  struct PosInfo {
    size_t cost, score;
    pos_t from;
  };
  static constexpr auto pos_start =
      pos_t{std::numeric_limits<ptrdiff_t>::min(),
            std::numeric_limits<ptrdiff_t>::min()};
  static constexpr auto pos_info_default =
      PosInfo{std::numeric_limits<ptrdiff_t>::max(),
              std::numeric_limits<ptrdiff_t>::max(), pos_start};
  constexpr auto hash = [](pos_t pos) {
    const auto hasher = std::hash<ptrdiff_t>{};
    return hasher(pos.front()) ^ hasher(pos.back());
  };
  auto pos_map = std::unordered_map<pos_t, PosInfo, decltype(hash)>{};
  const auto cmp_points = [&pos_map](pos_t p1, pos_t p2) {
    const auto it1 = pos_map.find(p1);
    const auto it2 = pos_map.find(p2);
    if (it1 == pos_map.end())
      return true;
    if (it2 == pos_map.end())
      return false;
    return it1->second.score > it2->second.score;
  };
  auto open_set =
      std::priority_queue<pos_t, std::vector<pos_t>, decltype(cmp_points)>(
          cmp_points);
  const auto distance = [&board](pos_t pos) -> size_t {
    return std::abs(pos.front() - board.finish().front()) +
           std::abs(pos.back() - board.finish().back());
  };
  const auto isValidNbr = [&board](pos_t base, pos_t nbr) {
    const auto [x, y] = nbr;
    return x >= 0 and x < board.width() and y >= 0 and y < board.height() and
           board(base) + 1 >= board(nbr);
  };
  // std::priority_queue does not provide element access, hold them separately
  std::unordered_set<pos_t, decltype(hash)> open_set_elems;

  open_set_elems.insert(board.start());
  open_set.push(board.start());
  pos_map[board.start()] = PosInfo{0, distance(board.start()), pos_start};

  const auto &draw = [&] {
    Board path = board;
    auto current = open_set.top();
    do {
      const auto previous = pos_map[current].from;
      const auto diff = pos_t{current.front() - previous.front(),
                              current.back() - previous.back()};
      if (std::abs(diff.front()) == 1)
        path(current) = '-';
      else
        path(current) = '|';
      current = previous;
    } while (current != board.start());
    path.print();
  };

  while (not open_set.empty()) {
    const auto current = open_set.top();
    const auto [current_cost, current_score, from] = pos_map[current];
    if (current == board.finish()) {
      if (do_draw == DrawResult::yes)
        draw();
      return current_cost;
    }
    open_set.pop();
    open_set_elems.erase(current);
    for (auto nbr_inc :
         std::array<pos_t, 4>{{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}}) {
      const auto nbr = pos_t{current.front() + nbr_inc.front(),
                             current.back() + nbr_inc.back()};
      if (isValidNbr(current, nbr)) {
        const auto tentative_cost = current_cost + 1;
        const auto it =
            pos_map.insert(std::make_pair(nbr, pos_info_default)).first;
        if (tentative_cost < it->second.cost) {
          it->second.cost = tentative_cost;
          it->second.score = tentative_cost + distance(nbr);
          it->second.from = current;
          if (not open_set_elems.contains(nbr)) {
            open_set.push(nbr);
            open_set_elems.insert(nbr);
          }
        }
      }
    }
  }
  return {};
}

void part1(const Board &board, DrawResult draw) {
  std::cout << *A_star(board, draw) << '\n';
}

void part2(Board &board, DrawResult draw) {
  // Just brute force it, we have the cycles (but not the time to figure out a
  // smarter algo)
  size_t min_path = std::numeric_limits<size_t>::max();
  pos_t min_pos{};
  for (ptrdiff_t y = 0; y < board.height(); ++y)
    for (ptrdiff_t x = 0; x < board.width(); ++x) {
      const auto pos = pos_t{x, y};
      if (board(pos) == 'a') {
        board.setStart(pos);
        const auto shortest_path = A_star(board);
        if (shortest_path)
          if (min_path > *shortest_path) {
            min_path = *shortest_path;
            min_pos = pos;
          }
      }
    }
  if (draw == DrawResult::yes) {
    board.setStart(min_pos);
    *A_star(board, DrawResult::yes);
  }
  std::cout << min_path << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  Board board{data};

  constexpr auto draw_results = DrawResult::no;
  if constexpr (draw_results == DrawResult::yes)
    board.print();

  part1(board, draw_results);
  part2(board, draw_results);
}
