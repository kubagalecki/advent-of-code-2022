#include "../common/common.hpp"

#include <unordered_map>

using i64 = std::int64_t;
using pos_t = std::array<i64, 2>;

pos_t operator+(pos_t a, pos_t b) {
  pos_t retval;
  retval[0] = a[0] + b[0];
  retval[1] = a[1] + b[1];
  return retval;
}
struct PosHasher {
  auto operator()(pos_t x) const {
    std::hash<i64> h;
    return h(x[0]) ^ h(x[1]);
  }
};
using pos_map_t = std::unordered_map<pos_t, pos_t, PosHasher>;

auto parseInput(std::string_view input) -> pos_map_t {
  pos_map_t retval;
  for (i64 y = 0; auto line : splitIntoLinesUntilEmpty(input)) {
    for (i64 x = 0; char c : line) {
      switch (c) {
      case '.':
        break;
      case '#':
        retval[pos_t{x, y}];
        break;
      default:
        throw std::runtime_error{"invalid input"};
      }
      ++x;
    }
    --y;
  }
  return retval;
}

class Simulation {
  static constexpr pos_t n = {0, 1}, e = {1, 0}, s = {0, -1}, w = {-1, 0};
  static constexpr auto dirs =
      std::array<pos_t, 8>{{{-1, 1}, n, {1, 1}, e, {1, -1}, s, {-1, -1}, w}};
  static constexpr auto dir_check_inds = std::array<std::array<size_t, 3>, 4>{
      {{0, 1, 2}, {6, 5, 4}, {0, 7, 6}, {2, 3, 4}}};
  static constexpr auto move_dirs = std::array{n, s, w, e};
  static constexpr pos_t stay = {std::numeric_limits<i64>::max(),
                                 std::numeric_limits<i64>::max()};

  static auto getMinMax(auto &&pos_range) -> std::array<i64, 4> {
    i64 x_min = std::numeric_limits<i64>::max(),
        x_max = std::numeric_limits<i64>::min(),
        y_min = std::numeric_limits<i64>::max(),
        y_max = std::numeric_limits<i64>::min();
    for (auto x : pos_range) {
      x_min = std::min(x_min, x[0]);
      x_max = std::max(x_max, x[0]);
      y_min = std::min(y_min, x[1]);
      y_max = std::max(y_max, x[1]);
    }
    return {x_min, x_max, y_min, y_max};
  }
  auto getNbrs(pos_t x) {
    std::array<bool, 8> retval;
    std::ranges::transform(dirs, retval.begin(), [&](pos_t dx) {
      return pos_to_cons_map_.contains(x + dx);
    });
    return retval;
  }

public:
  Simulation(pos_map_t map) : pos_to_cons_map_{std::move(map)} {
    std::iota(dir_check_order_.begin(), dir_check_order_.end(), 0u);
  }
  size_t run(size_t max_turns = std::numeric_limits<size_t>::max()) {
    std::unordered_map<pos_t, size_t, PosHasher> cons_map;
    size_t turn{};
    for (; turn < max_turns; ++turn) {
      for (auto &[x, x_cons] : pos_to_cons_map_) {
        const auto nbrs = getNbrs(x);
        if (std::ranges::any_of(nbrs, std::identity{})) {
          for (auto i : dir_check_order_)
            if (std::ranges::none_of(dir_check_inds[i],
                                     [&](auto dci) { return nbrs[dci]; })) {
              x_cons = x + move_dirs[i];
              ++cons_map[x_cons];
              goto OuterLoopEnd;
            }
        }
        x_cons = stay;
      OuterLoopEnd:;
      }

      bool moved{};
      pos_map_t new_pos;
      for (const auto &[x, x_cons] : pos_to_cons_map_)
        if (x_cons == stay or cons_map[x_cons] > 1)
          new_pos[x];
        else {
          new_pos[x_cons];
          moved = true;
        }
      if (not moved)
        break;
      cons_map.clear();
      pos_to_cons_map_ = std::move(new_pos);
      std::rotate(dir_check_order_.begin(), std::next(dir_check_order_.begin()),
                  dir_check_order_.end());
    }
    return turn;
  }
  i64 getNumEmptyFieldsInHull() const {
    const auto [x_min, x_max, y_min, y_max] =
        getMinMax(pos_to_cons_map_ | std::views::keys);
    return (x_max - x_min + 1) * (y_max - y_min + 1) - pos_to_cons_map_.size();
  }

private:
  pos_map_t pos_to_cons_map_;
  std::array<unsigned char, 4> dir_check_order_{};
};

void part1(Simulation &sim) {
  sim.run(10);
  std::cout << sim.getNumEmptyFieldsInHull() << '\n';
}

void part2(Simulation &sim) {
  std::cout << sim.run() + 11 /*10 from the first part of the problem + 1 since
                                 we want the first turn where no move occurs,
                                 not the last turn where it does */
            << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  auto sim = Simulation{parseInput(data)};
  part1(sim);
  part2(sim);
}
