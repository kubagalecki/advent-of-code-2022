#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <vector>

#include "../common/common.hpp"

using i64 = std::int64_t;
using vec_t = std::array<i64, 3>;

auto operator+(vec_t a, vec_t b) {
  vec_t retval;
  std::ranges::transform(a, b, retval.begin(), std::plus{});
  return retval;
}
auto operator-(vec_t a, vec_t b) {
  vec_t retval;
  std::ranges::transform(a, b, retval.begin(), std::minus{});
  return retval;
}
constexpr auto ones = vec_t{1, 1, 1};
constexpr auto nbr_offsets = std::array<vec_t, 6>{
    {{-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1}}};

class Grid3D {
  i64 computeIndex(vec_t coords) const {
    auto [x, y, z] = coords;
    auto [x_min, y_min, z_min] = dim_min_;
    auto [xe, ye, ze] = extents_;
    return (z - z_min) * xe * ye + (y - y_min) * xe + (x - x_min);
  }

public:
  Grid3D(vec_t min, vec_t max)
      : dim_min_{min}, extents_{max - min + ones},
        filled_(std::reduce(extents_.begin(), extents_.end(), i64{1},
                            std::multiplies{})) {}
  bool test(vec_t coords) const { return filled_[computeIndex(coords)]; }
  void set(vec_t coords) { filled_[computeIndex(coords)] = true; }
  bool inGrid(vec_t coords) const {
    return std::ranges::all_of(std::views::iota(0, 3), [&](auto i) {
      return coords[i] >= dim_min_[i] and coords[i] < dim_min_[i] + extents_[i];
    });
  }

  auto getMin() const { return dim_min_; }
  auto getMax() const { return dim_min_ + extents_ - ones; }

private:
  vec_t dim_min_, extents_;
  std::vector<bool> filled_;
};

Grid3D parseInput(std::string_view input) {
  std::vector<vec_t> points;
  vec_t min, max;
  min.fill(std::numeric_limits<i64>::max());
  max.fill(std::numeric_limits<i64>::min());
  for (auto line : splitIntoLinesUntilEmpty(input)) {
    vec_t coords;
    for (size_t i = 0; auto num : line | std::views::split(',') |
                                      std::views::transform([](auto &&str) {
                                        return std::string_view{str};
                                      })) {
      coords[i] = toNumber<i64>(num);
      min[i] = std::min(min[i], coords[i]);
      max[i] = std::max(max[i], coords[i]);
      ++i;
    }
    points.push_back(coords);
  }
  Grid3D retval(min - ones, max + ones);
  for (auto coords : points)
    retval.set(coords);
  return retval;
}

size_t countFaces(const Grid3D &grid) {
  const auto [x_min, y_min, z_min] = grid.getMin() + ones;
  const auto [x_max, y_max, z_max] = grid.getMax() - ones;
  size_t retval{};
  for (auto z = z_min; z <= z_max; ++z)
    for (auto y = y_min; y <= y_max; ++y)
      for (auto x = x_min; x <= x_max; ++x) {
        const auto coords = vec_t{x, y, z};
        if (grid.test(coords))
          retval += std::ranges::count_if(nbr_offsets, [&](auto ofs) {
            return not grid.test(coords + ofs);
          });
      }
  return retval;
}

void part1(const Grid3D &grid) { std::cout << countFaces(grid) << '\n'; }

void part2(Grid3D &grid) {
  auto bfs_grid = grid;
  std::queue<vec_t> bfs_q;
  bfs_q.push(grid.getMin());
  bfs_grid.set(grid.getMin());
  // Use BFS to mark all nodes reachable from the edge of the grid
  while (not bfs_q.empty()) {
    const auto current = bfs_q.front();
    bfs_q.pop();
    std::ranges::for_each(nbr_offsets, [&](auto ofs) {
      const auto test_coord = current + ofs;
      if (bfs_grid.inGrid(test_coord) and not bfs_grid.test(test_coord)) {
        bfs_q.push(test_coord);
        bfs_grid.set(test_coord);
      }
    });
  }
  // Mark inclusions as rock
  const auto [x_min, y_min, z_min] = grid.getMin() + ones;
  const auto [x_max, y_max, z_max] = grid.getMax() - ones;
  for (auto z = z_min; z <= z_max; ++z)
    for (auto y = y_min; y <= y_max; ++y)
      for (auto x = x_min; x <= x_max; ++x) {
        const auto coords = vec_t{x, y, z};
        if (not bfs_grid.test(coords))
          grid.set(coords);
      }
  std::cout << countFaces(grid) << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  auto grid = parseInput(data);
  part1(grid);
  part2(grid);
}
