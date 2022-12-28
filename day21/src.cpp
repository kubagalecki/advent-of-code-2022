#include "../common/common.hpp"

#include <functional>
#include <unordered_map>
#include <variant>

using i64 = std::int64_t;
using fun_t = std::function<i64(i64, i64)>;
using arg_t = std::array<std::string_view, 2>;
using call_t = std::pair<arg_t, fun_t>;
using monkey_t = std::variant<i64, call_t>;
using monkey_map_t = std::unordered_map<std::string_view, monkey_t>;

template <typename... Os> struct OverloadSet : Os... {
  using Os::operator()...;
};
template <typename... Os>
OverloadSet(Os &&...) -> OverloadSet<std::decay_t<Os>...>;

auto parseInput(std::string_view input) {
  monkey_map_t retval;
  for (auto line : splitIntoLinesUntilEmpty(input)) {
    const auto colon_pos = line.find_first_of(':');
    auto &monkey = retval[line.substr(0, colon_pos)];
    line.remove_prefix(colon_pos + 2);
    auto arg_strings = line | std::views::split(' ');
    switch (std::ranges::distance(arg_strings)) {
    case 1:
      monkey = toNumber<i64>(*arg_strings.begin());
      break;
    case 3: {
      auto arg_it = arg_strings.begin();
      auto args = arg_t{};
      auto fun = fun_t{};
      auto &[a1, a2] = args;
      a1 = std::string_view{*arg_it++};
      if (std::string_view{*arg_it} == "+"sv)
        fun = std::plus<i64>{};
      else if (std::string_view{*arg_it} == "-"sv)
        fun = std::minus<i64>{};
      else if (std::string_view{*arg_it} == "*"sv)
        fun = std::multiplies<i64>{};
      else if (std::string_view{*arg_it} == "/"sv)
        fun = +[](i64 a, i64 b) {
          if (b != 0)
            return a / b;
          throw std::domain_error{"Dividing by 0"};
        };
      else
        throw std::runtime_error{"op parse error"};
      a2 = std::string_view{*++arg_it};
      monkey = std::make_pair(args, fun);
      break;
    }
    default:
      throw std::runtime_error{"parse error"};
    }
  }
  return retval;
}

i64 getValue(const monkey_t &monkey, const monkey_map_t &map) {
  return std::visit<i64>(OverloadSet{[](i64 v) { return v; },
                                     [&map](const call_t &call) {
                                       const auto &[args, op] = call;
                                       const auto [a1, a2] = args;
                                       return std::invoke(
                                           op, getValue(map.at(a1), map),
                                           getValue(map.at(a2), map));
                                     }},
                         monkey);
}

void part1(const monkey_map_t &map) {
  std::cout << getValue(map.at("root"sv), map) << '\n';
}

// This is in general a bad practice, don't use std::optional this way
auto maybeEval(auto &&fun, auto &&...args)
    -> std::optional<std::invoke_result_t<decltype(fun), decltype(args)...>> {
  try {
    return std::invoke(std::forward<decltype(fun)>(fun),
                       std::forward<decltype(args)>(args)...);
  } catch (...) {
    return {};
  }
}

auto bisection(auto &&fun, const i64 lo, const i64 hi, const i64 lo_val,
               const i64 hi_val) -> std::optional<i64>
  requires requires(i64 arg) {
             { fun(arg) } -> std::same_as<i64>;
           }
{
  // Failure condition
  if (hi - lo == 1)
    return {};

  auto mid = std::midpoint(lo, hi);
  auto mid_val = maybeEval(fun, mid);

  // Adjust mid if invalid
  constexpr i64 max_retries_base = 1 << 10;
  const auto max_retries =
      std::min(max_retries_base, std::min(mid - lo - 1, hi - mid - 1));
  int retry_counter = 0;
  while (not mid_val and retry_counter < max_retries) {
    mid_val = maybeEval(fun, ++mid);
    ++retry_counter;
  }
  if (retry_counter == max_retries) {
    retry_counter = 0;
    mid -= max_retries;
    while (not mid_val and retry_counter < max_retries) {
      mid_val = maybeEval(fun, --mid);
      ++retry_counter;
    }
    if (retry_counter == max_retries)
      return {};
  }

  if (*mid_val == 0)
    return mid;
  if (*mid_val * hi_val < 0)
    return bisection(fun, mid, hi, *mid_val, hi_val);
  else
    return bisection(fun, lo, mid, lo_val, *mid_val);
}

void part2(monkey_map_t &&map) {
  const auto [leaf1_name, leaf2_name] =
      std::get<call_t>(map.at("root"sv)).first;
  const auto &leaf1 = map.at(leaf1_name);
  const auto &leaf2 = map.at(leaf2_name);
  auto &human = std::get<i64>(map.at("humn"sv));
  const auto fun = [&](i64 arg) {
    human = arg;
    return getValue(leaf1, map) - getValue(leaf2, map);
  };

  // While there may be a more elegant solution based on the analysis of the
  // input DAG, we're going to solve the problem numerically. We treat `fun` as
  // a black box i64 -> i64 mapping. We first do a coarse-grained grid search
  // from lo to hi. Whenever neighboring grid points yield opposing signs for
  // fun, we bisect the interval they span. This approach not guaranteed to
  // work, YMMV. The program may even crash upon division by zero (depends
  // purely on the input graph).
  constexpr i64 lo = std::numeric_limits<i64>::min() / (1 << 20),
                hi = std::numeric_limits<i64>::max() / (1 << 20),
                grain_size = i64{1} << 31;

  const auto printResult = [&](i64 val) { std::cout << val << '\n'; };
  i64 prev_val = fun(lo), num_candidates{};
  if (prev_val == 0) {
    printResult(lo);
    return;
  }
  for (i64 i = lo + grain_size; i <= hi; i += grain_size) {
    const auto val = fun(i);
    if (val == 0) {
      printResult(i);
      return;
    }
    if (prev_val * val < 0) {
      ++num_candidates;
      const auto bisec_result =
          bisection(fun, i - grain_size, i, prev_val, val);
      if (bisec_result) {
        printResult(*bisec_result);
        return;
      }
    }
    prev_val = val;
  }
  std::puts("Failed to find solution, please adjust parameters and try again");
  std::cout << "The number of candidate intervals was " << num_candidates << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  auto monkey_map = parseInput(data);
  part1(monkey_map);
  part2(std::move(monkey_map));
}
