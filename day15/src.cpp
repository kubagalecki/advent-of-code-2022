#include "../common/common.hpp"

using pos_t = std::array<std::int64_t, 2>;

auto parseInput(std::string_view data) {
  constexpr auto readNum = [](std::string_view &line) {
    line.remove_prefix(line.find_first_of('=') + 1);
    const auto num_end = line.find_first_not_of("-0123456789"sv);
    auto retval = toNumber<std::int64_t>(line.substr(0, num_end));
    line.remove_prefix(num_end);
    return retval;
  };

  std::vector<std::array<pos_t, 2>> retval;
  for (auto line : splitIntoLinesUntilEmpty(data)) {
    pos_t sensor, beacon;
    sensor.front() = readNum(line);
    sensor.back() = readNum(line);
    beacon.front() = readNum(line);
    beacon.back() = readNum(line);
    retval.push_back(std::array{sensor, beacon});
  }
  return retval;
}

void getBlockedRanges(const std::vector<std::array<pos_t, 2>> &input,
                      const std::int64_t y_level,
                      std::vector<pos_t> &blocked_ranges) {
  constexpr auto invalid = pos_t{1, 0};
  blocked_ranges.clear();
  std::ranges::copy_if(
      input | std::views::transform([&](const auto &in) -> pos_t {
        const auto [sensor, beacon] = in;
        const auto [sx, sy] = sensor;
        const auto [bx, by] = beacon;
        const auto row_dist = std::abs(sy - y_level);
        const auto manh_dist = std::abs(sx - bx) + std::abs(sy - by);
        if (manh_dist < row_dist)
          return invalid;
        const auto x_span = manh_dist - row_dist;
        return {sx - x_span, sx + x_span};
      }),
      std::back_inserter(blocked_ranges),
      [&](pos_t r) { return r != invalid; });
  std::ranges::sort(blocked_ranges);
}

auto getNumBeaconsInRow(const std::vector<std::array<pos_t, 2>> &input,
                        const std::int64_t y_level) {
  std::vector<pos_t> beacons_in_row;
  std::ranges::copy_if(input | std::views::values,
                       std::back_inserter(beacons_in_row),
                       [&y_level](pos_t b) { return b.back() == y_level; });
  std::ranges::sort(beacons_in_row);
  return beacons_in_row.size() - std::ranges::unique(beacons_in_row).size();
}

auto getNumBlocked(const std::vector<pos_t> &blocked_ranges,
                   pos_t limits = {std::numeric_limits<std::int64_t>::min(),
                                   std::numeric_limits<std::int64_t>::max()}) {
  const auto [lo, hi] = limits;
  std::int64_t sum = 0, running_max = std::numeric_limits<std::int64_t>::min();
  for (auto [min, max] : blocked_ranges) {
    min = std::clamp(min, lo, hi);
    max = std::clamp(max, lo, hi);
    if (running_max >= max)
      continue;
    if (min > running_max)
      sum += max - min + 1;
    else
      sum += max - running_max;
    running_max = max;
  }
  return sum;
}

void part1(const std::vector<std::array<pos_t, 2>> &input) {
  constexpr auto y_level = 2'000'000;
  std::vector<pos_t> blocked_ranges;
  getBlockedRanges(input, y_level, blocked_ranges);
  std::cout << getNumBlocked(blocked_ranges) -
                   getNumBeaconsInRow(input, y_level)
            << '\n';
};

auto findEmptyPos(const std::vector<pos_t> &blocked_ranges,
                  pos_t limits = {std::numeric_limits<std::int64_t>::min(),
                                  std::numeric_limits<std::int64_t>::max()}) {
  const auto it =
      std::ranges::adjacent_find(blocked_ranges, [](pos_t first, pos_t second) {
        return first.back() + 2 == second.front();
      });
  if (it != blocked_ranges.end())
    return it->back() + 1;
  else if (blocked_ranges.front().front() == limits.front() + 1)
    return limits.front();
  else
    return limits.back();
}

void part2(const std::vector<std::array<pos_t, 2>> &input) {
  constexpr pos_t limits{0, 4'000'000};
  const auto [lo, hi] = limits;
  std::vector<pos_t> blocked_ranges;
  for (auto y_level = lo; y_level <= hi; ++y_level) {
    getBlockedRanges(input, y_level, blocked_ranges);
    const auto num_blocked = getNumBlocked(blocked_ranges, limits);
    if (num_blocked == hi - lo + 1)
      continue;
    else if (num_blocked == hi - lo) {
      const auto x = findEmptyPos(blocked_ranges, limits);
      std::cout << x * 4'000'000 + y_level << '\n';
      return;
    } else
      throw std::logic_error{"Multiple locations possible"};
  }
}

int main() {
  const auto [alloc, data] = getStdinView();
  const auto input = parseInput(data);
  part1(input);
  part2(input);
}
