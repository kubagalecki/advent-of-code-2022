#include "../common/common.hpp"

using i64 = std::int64_t;
using vec_t = std::array<i64, 2>;

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

constexpr vec_t ones{1, 1}, down{0, -1}, left{-1, 0}, right{1, 0};

class Grid2D {
  i64 computeIndex(vec_t coords) const {
    auto [x, y] = coords;
    auto [x_min, y_min] = dim_min_;
    auto [xe, ye] = extents_;
    return (y - y_min) * xe + (x - x_min);
  }

public:
  Grid2D(vec_t min, vec_t max)
      : dim_min_{min}, extents_{max - min + ones},
        filled_(std::reduce(extents_.begin(), extents_.end(), i64{1},
                            std::multiplies{})) {}

  bool test(vec_t coords) const { return filled_[computeIndex(coords)]; }
  void set(vec_t coords) { filled_[computeIndex(coords)] = true; }
  void reset(vec_t coords) { filled_[computeIndex(coords)] = false; }
  bool inGrid(vec_t coords) const {
    return std::ranges::all_of(std::views::iota(0u, coords.size()),
                               [&](auto i) {
                                 return coords[i] >= dim_min_[i] and
                                        coords[i] < dim_min_[i] + extents_[i];
                               });
  }

  auto getMin() const { return dim_min_; }
  auto getMax() const { return dim_min_ + extents_ - ones; }

  void draw() const {
    for (i64 y = dim_min_[1] + extents_[1] - 1; y >= dim_min_[1]; --y) {
      for (i64 x = dim_min_[0]; x < dim_min_[0] + extents_[0]; ++x)
        std::cout << (test(vec_t{x, y}) ? '#' : '.');
      std::cout << '\n';
    }
  }

private:
  vec_t dim_min_, extents_;
  std::vector<bool> filled_;
};

class Simulation {
public:
  using shape_t = std::vector<vec_t>;

private:
  static constexpr i64 width = 7 + 2, height = 1 << 20;
  static constexpr i64 spawn_x_dist = 2, spawn_y_dist = 3;
  static const std::vector<shape_t> shapes;

  void spawnShape() {
    const auto spawn_pos =
        vec_t{spawn_x_dist + 1, peak_truncated_ + spawn_y_dist + 1};
    falling_shape_ =
        shapes[std::exchange(shape_ind_, (shape_ind_ + 1) % shapes.size())];
    for (auto &pixel : falling_shape_)
      pixel = pixel + spawn_pos;
    if (falling_shape_.back().back() >= height)
      throw std::runtime_error{"Exceeded tower allocation"};
  };
  bool tryMoveShape(vec_t dir) {
    static auto temp = shape_t{};
    temp = falling_shape_;
    for (auto &pixel : temp)
      pixel = pixel + dir;
    if (std::ranges::none_of(
            temp, [this](auto pixel) { return grid_.test(pixel); })) {
      falling_shape_ = temp;
      return true;
    } else
      return false;
  }
  void freezeShape() {
    for (auto pixel : falling_shape_)
      grid_.set(pixel);
    peak_truncated_ = std::max(falling_shape_.back().back(), peak_truncated_);
  };
  void truncate(i64 trunc_pos) {
    for (i64 y = 1; y <= trunc_pos; ++y)
      for (i64 x = 1; x < width - 1; ++x)
        grid_.reset(vec_t{x, y});
    for (i64 y = trunc_pos + 1; y <= peak_truncated_; ++y)
      for (i64 x = 1; x < width - 1; ++x) {
        const auto coords = vec_t{x, y};
        if (grid_.test(coords)) {
          grid_.reset(coords);
          grid_.set(vec_t{x, y - trunc_pos});
        }
      }
    floor_pos_ += trunc_pos;
    peak_truncated_ -= trunc_pos;
  }
  std::optional<i64> getTruncPos() const {
    for (i64 y : falling_shape_ | std::views::values) {
      if (std::ranges::all_of(std::views::iota(i64{1}) |
                                  std::views::take(width - 2),
                              [&](i64 x) {
                                return grid_.test(vec_t{x, y});
                              }))
        return y;
    }
    return {};
  }
  void tryTruncate() {
    const auto trunc_pos = getTruncPos();
    if (trunc_pos)
      truncate(*trunc_pos);
  }

public:
  Simulation(std::string_view input) : wind_{input} {
    while (wind_.back() == '\n')
      wind_.remove_suffix(1);
    // Floor and walls
    for (i64 x = 0; x < width; ++x)
      grid_.set(vec_t{x, 0});
    for (i64 y = 1; y < height; ++y) {
      grid_.set(vec_t{0, y});
      grid_.set(vec_t{width - 1, y});
    }
    spawnShape();
  }
  void tick() {
    if (wind_[wind_pos_++] == '<')
      tryMoveShape(left);
    else
      tryMoveShape(right);
    if (wind_pos_ == wind_.size())
      wind_pos_ = 0;

    if (not tryMoveShape(down)) {
      freezeShape();
      ++num_fallen_;
      tryTruncate();
      spawnShape();
    }
  }
  size_t numFallen() const { return num_fallen_; }
  size_t peak() const { return peak_truncated_ + floor_pos_; }

private:
  Grid2D grid_{vec_t{0, 0}, vec_t{width - 1, height - 1}};
  std::string_view wind_;
  shape_t falling_shape_;
  i64 floor_pos_{}, peak_truncated_{};
  size_t num_fallen_{}, wind_pos_{}, shape_ind_{};
};
const std::vector<Simulation::shape_t> Simulation::shapes{
    {{0, 0}, {1, 0}, {2, 0}, {3, 0}},
    {{1, 0}, {0, 1}, {1, 1}, {2, 1}, {1, 2}},
    {{0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}},
    {{0, 0}, {0, 1}, {0, 2}, {0, 3}},
    {{0, 0}, {1, 0}, {0, 1}, {1, 1}}};

void part1(std::string_view input) {
  Simulation sim{input};
  while (sim.numFallen() < 2022)
    sim.tick();
  std::cout << sim.peak() << '\n';
}

void part2(std::string_view input) {
  // There's definitely a way to avoid running the entire simulation by
  // detecting cycles. The nice thing about doing AoC in C++ is that problems
  // which would be computationally infeasible in Python can be solved in a long
  // - but not prohibitively so - time. Run this over a few days.
  Simulation sim{input};
  while (sim.numFallen() < 1'000'000'000'000ll) {
    sim.tick();
  }
  std::cout << sim.peak() << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  part1(data);
  part2(data);
}
