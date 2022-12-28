#include "../common/common.hpp"

#include <queue>
#include <unordered_map>
#include <unordered_set>

using i64 = std::int64_t;
using pos_t = std::array<i64, 3>;

auto operator+(pos_t a, pos_t b) {
  pos_t retval;
  std::ranges::transform(a, b, retval.begin(), std::plus{});
  return retval;
}
auto operator-(pos_t a, pos_t b) {
  pos_t retval;
  std::ranges::transform(a, b, retval.begin(), std::minus{});
  return retval;
}
constexpr pos_t left = {-1, 0, 1}, right = {1, 0, 1}, up = {0, 1, 1},
                down = {0, -1, 1}, stay = {0, 0, 1};
constexpr auto nbr_ofs = std::array{left, down, up, right, stay};

class Grid3D {
  i64 computeIndex(pos_t coords) const {
    auto [x, y, z] = coords;
    auto [xe, ye, ze] = extents_;
    return z * xe * ye + y * xe + x;
  }

public:
  Grid3D(pos_t size)
      : extents_{size}, filled_(std::reduce(extents_.begin(), extents_.end(),
                                            i64{1}, std::multiplies{})) {}
  bool test(pos_t coords) const { return filled_[computeIndex(coords)]; }
  void set(pos_t coords) { filled_[computeIndex(coords)] = true; }
  bool inGrid(pos_t x) const {
    return std::ranges::all_of(
        std::views::iota(0u, extents_.size()),
        [&](auto i) { return x[i] >= 0 and x[i] < extents_[i]; });
  }
  const auto &getExtents() const { return extents_; }

private:
  pos_t extents_;
  std::vector<bool> filled_;
};

std::optional<size_t> A_star(const Grid3D &grid, pos_t start, i64 x_finish,
                             i64 y_finish) {
  struct PosInfo {
    size_t cost, score;
  };
  static constexpr auto pos_info_default =
      PosInfo{std::numeric_limits<i64>::max(), std::numeric_limits<i64>::max()};
  constexpr auto hash = [](pos_t pos) {
    const auto h = std::hash<i64>{};
    return h(pos[0]) ^ h(pos[1]) ^ h(pos[2]);
  };
  auto pos_map = std::unordered_map<pos_t, PosInfo, decltype(hash)>{};
  auto cmp_points = [&pos_map](pos_t p1, pos_t p2) {
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
          std::move(cmp_points));
  const auto distance = [&](pos_t pos) -> size_t {
    return std::abs(pos[0] - x_finish) + std::abs(pos[1] - y_finish);
  };
  const auto isFinish = [&](pos_t x) {
    return x[0] == x_finish and x[1] == y_finish;
  };
  const auto isValidNbr = [&grid](pos_t nbr) {
    return grid.inGrid(nbr) and not grid.test(nbr);
  };
  // std::priority_queue does not provide element access, hold them separately
  std::unordered_set<pos_t, decltype(hash)> open_set_elems;

  open_set_elems.insert(start);
  open_set.push(start);
  pos_map[start] = PosInfo{0, distance(start)};

  while (not open_set.empty()) {
    const auto current = open_set.top();
    const auto [current_cost, current_score] = pos_map[current];
    if (isFinish(current)) {
      return current_cost;
    }
    open_set.pop();
    open_set_elems.erase(current);
    for (auto nbr_inc : nbr_ofs) {
      const auto nbr = current + nbr_inc;
      if (isValidNbr(nbr)) {
        const auto tentative_cost = current_cost + 1;
        const auto it =
            pos_map.insert(std::make_pair(nbr, pos_info_default)).first;
        if (tentative_cost < it->second.cost) {
          it->second.cost = tentative_cost;
          it->second.score = tentative_cost + distance(nbr);
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

Grid3D parseInput(std::string_view input, size_t z_param) {
  auto lines = splitIntoLinesUntilEmpty(input);
  const i64 ye = std::ranges::distance(lines);
  const i64 xe = (*std::ranges::begin(lines)).size();
  const i64 ze = std::max(ye, xe) * z_param;
  Grid3D retval{pos_t{xe, ye, ze}};
  const auto propagate3D = [&](pos_t dir, i64 x, i64 y) {
    pos_t pos{x, y, 0};
    for (i64 z = 0; z < ze; ++z) {
      retval.set(pos);
      pos = pos + dir;
      if (pos[0] == 0)
        pos[0] = xe - 2;
      else if (pos[0] == xe - 1)
        pos[0] = 1;
      if (pos[1] == 0)
        pos[1] = ye - 2;
      else if (pos[1] == ye - 1)
        pos[1] = 1;
    }
  };
  for (i64 y = ye - 1; auto line : lines) {
    for (i64 x = 0; auto c : line) {
      switch (c) {
      case '.':
        break;
      case '#':
        for (i64 z = 0; z < ze; ++z)
          retval.set(pos_t{x, y, z});
        break;
      case '<':
        propagate3D(left, x, y);
        break;
      case '>':
        propagate3D(right, x, y);
        break;
      case '^':
        propagate3D(up, x, y);
        break;
      case 'v':
        propagate3D(down, x, y);
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

void part1(std::string_view input) {
  size_t z_param = 4;
  auto grid = parseInput(input, z_param);
  const auto start = pos_t{1, grid.getExtents()[1] - 1, 0};
  const i64 x_finish = grid.getExtents()[0] - 2, y_finish = 0;
  auto num_turns = A_star(grid, start, x_finish, y_finish);
  while (not num_turns) {
    if (z_param >= 1024) {
      std::cerr << "Could not find path\n";
      return;
    }
    z_param *= 4;
    grid = parseInput(input, z_param);
    num_turns = A_star(grid, start, x_finish, y_finish);
  }
  std::cout << *num_turns << '\n';
}

void part2(std::string_view input) {
  size_t z_param = 16;
  auto grid = parseInput(input, z_param);
  const auto start = pos_t{1, grid.getExtents()[1] - 1, 0};
  const i64 x_finish = grid.getExtents()[0] - 2, y_finish = 0;

  const auto trySolve = [&] {
    size_t total{};
    const auto sumTotal = [&total](size_t len) { return total += len; };
    const auto goBack = [&](i64 len_there) {
      return A_star(grid, pos_t{x_finish, y_finish, len_there}, start[0],
                    start[1]);
    };
    const auto goBackAgain = [&](i64 len_back) {
      return A_star(grid, start + pos_t{0, 0, len_back}, x_finish, y_finish);
    };
    return A_star(grid, start, x_finish, y_finish)
        .transform(sumTotal)
        .and_then(goBack)
        .transform(sumTotal)
        .and_then(goBackAgain)
        .transform(sumTotal);
  };
  auto num_turns = trySolve();
  while (not num_turns) {
    if (z_param >= 4096) {
      std::cerr << "Could not find path\n";
      return;
    }
    z_param *= 4;
    grid = parseInput(input, z_param);
    num_turns = trySolve();
  }
  std::cout << *num_turns << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  part1(data);
  part2(data);
}
