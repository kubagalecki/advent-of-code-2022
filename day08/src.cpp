#include "../common/common.hpp"

auto getHeights(std::string_view data) {
  std::vector<char> heights;
  size_t row_size{};
  heights.reserve(data.size());
  std::ranges::for_each(
      splitIntoLinesUntilEmpty(data), [&](std::string_view line) {
        std::ranges::transform(line, std::back_inserter(heights),
                               [](char c) { return c - '0'; });
        row_size = line.size();
      });
  heights.shrink_to_fit();
  return std::make_pair(std::move(heights), row_size);
}

static constexpr auto min_val = [](auto a, auto b) { return std::min(a, b); };
static constexpr auto max_val = [](auto a, auto b) { return std::max(a, b); };

void computeRowThresholds(std::span<const char> row,
                          std::span<char> forward_max,
                          std::span<char> backward_max, auto out_it) {
  std::exclusive_scan(row.begin(), row.end(), forward_max.begin(), char{-1},
                      max_val);
  std::exclusive_scan(row.rbegin(), row.rend(), backward_max.rbegin(), char{-1},
                      max_val);
  std::ranges::transform(forward_max, backward_max, out_it, min_val);
}

// This is a clunky workaround since we don't yet have views::strided or mdspan
void computeColThresholds(auto col_begin, std::span<char> forward_max,
                          auto out_begin, size_t row_size, size_t n_rows) {
  char threshold = -1;
  for (size_t row = 0; row < n_rows; ++row) {
    forward_max[row] = threshold;
    threshold = std::max(threshold, col_begin[row * row_size]);
  }
  threshold = -1;
  for (size_t row = n_rows - 1; row < n_rows; --row) {
    const auto offs = row * row_size;
    const auto col_min = std::min(threshold, forward_max[row]);
    out_begin[offs] = std::min(col_min, out_begin[offs]);
    threshold = std::max(threshold, col_begin[offs]);
  }
}

void part1(const std::vector<char> &heights, size_t row_size) {
  std::vector<char> visibility_threshold(heights.size()), helpers(2 * row_size);
  const auto forward_max = std::span{helpers}.subspan(0, row_size);
  const auto backward_max = std::span{helpers}.subspan(row_size);
  size_t n_rows{};
  for (size_t row_offs = 0; row_offs < heights.size(); row_offs += row_size) {
    const auto row = std::span{heights}.subspan(row_offs, row_size);
    computeRowThresholds(row, forward_max, backward_max,
                         std::next(visibility_threshold.begin(), row_offs));
    ++n_rows;
  }
  for (size_t col = 0; col < row_size; ++col) {
    computeColThresholds(std::next(heights.cbegin(), col), forward_max,
                         std::next(visibility_threshold.begin(), col), row_size,
                         n_rows);
  }

  const size_t n_visible_trees = std::ranges::count_if(
      std::views::iota(0u) | std::views::take(heights.size()),
      [&](auto index) { return heights[index] > visibility_threshold[index]; });
  std::cout << n_visible_trees << '\n';
}

size_t computeDirScore(const std::vector<char> &heights, size_t pos,
                       ptrdiff_t stride, size_t max_steps) {
  const auto my_height = heights[pos];
  for (size_t step = 0; step < max_steps; ++step) {
    pos += stride;
    if (heights[pos] >= my_height)
      return step + 1;
  }
  return max_steps;
}

size_t computeViewScore(const std::vector<char> &heights, size_t row_size,
                        size_t n_rows, size_t row, size_t col) {
  const auto pos = row * row_size + col;
  const auto left = computeDirScore(heights, pos, -1, col);
  const auto right = computeDirScore(heights, pos, 1, row_size - col - 1);
  const auto top = computeDirScore(heights, pos, -1 * row_size, row);
  const auto bot = computeDirScore(heights, pos, row_size, n_rows - row - 1);
  return left * right * top * bot;
}

void part2(const std::vector<char> &heights, size_t row_size) {
  const size_t n_rows = heights.size() / row_size;
  size_t max_score{};
  for (size_t r = 0; r < n_rows; ++r)
    for (size_t c = 0; c < row_size; ++c)
      max_score = std::max(max_score,
                           computeViewScore(heights, row_size, n_rows, r, c));
  std::cout << max_score << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  const auto [heights, row_size] = getHeights(data);
  part1(heights, row_size);
  part2(heights, row_size);
}
