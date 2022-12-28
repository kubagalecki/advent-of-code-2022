#include "../common/common.hpp"

using i64 = std::int64_t;
using vec_t = std::vector<i64>;

auto parseInput(std::string_view input) -> vec_t {
  vec_t retval;
  std::ranges::transform(splitIntoLinesUntilEmpty(input),
                         std::back_inserter(retval),
                         [](auto line) { return toNumber<i64>(line); });
  retval.shrink_to_fit();
  return retval;
}

void rotateLeftBy1(auto begin, i64 len) {
  std::rotate(begin, std::next(begin), std::next(begin, len));
}

vec_t mix(const vec_t &nums, size_t n_mix_rounds) {
  vec_t inds(nums.size());
  std::iota(inds.begin(), inds.end(), 0);
  const auto rotate = [&](auto old_it, auto rot_val) {
    const auto old_pos = std::distance(inds.begin(), old_it);
    const auto num_slots = static_cast<i64>(inds.size()) - 1;

    auto new_pos = (old_pos + rot_val) % num_slots;
    if (new_pos <= 0) // Wrapped around to the left
      new_pos = num_slots + new_pos;

    const auto rot_len = std::abs(new_pos - old_pos) + 1;
    if (new_pos > old_pos)
      rotateLeftBy1(old_it, rot_len);
    else if (new_pos < old_pos)
      rotateLeftBy1(std::make_reverse_iterator(std::next(old_it)), rot_len);
  };
  for (; n_mix_rounds > 0; --n_mix_rounds)
    for (size_t i = 0; i < inds.size(); ++i) {
      const auto num = nums[i];
      const auto old_it = std::ranges::find(inds, i);
      rotate(old_it, num);
    }
  vec_t retval;
  retval.reserve(nums.size());
  std::ranges::transform(inds, std::back_inserter(retval),
                         [&](auto i) { return nums[i]; });
  return retval;
}

i64 groveCoords(const vec_t &mixed) {
  const auto pos0 = std::distance(mixed.begin(), std::ranges::find(mixed, 0));
  const auto getAt = [&](auto i) { return mixed[i % mixed.size()]; };
  return getAt(pos0 + 1000) + getAt(pos0 + 2000) + getAt(pos0 + 3000);
}

void part1(const vec_t &nums) {
  const auto mixed = mix(nums, 1);
  std::cout << groveCoords(mixed) << '\n';
}

void part2(vec_t &&nums) {
  for (auto &n : nums)
    n *= 811589153;
  const auto mixed = mix(nums, 10);
  std::cout << groveCoords(mixed) << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  auto nums = parseInput(data);
  part1(nums);
  part2(std::move(nums));
}
