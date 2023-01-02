#include "../common/common.hpp"

using u16 = std::uint16_t;

struct Blueprint {
  u16 ore_per_orebot, ore_per_claybot, ore_per_obsbot, clay_per_obsbot,
      ore_per_geodebot, obs_per_geodebot, max_ore_cost;
};

std::vector<Blueprint> parseBlueprints(std::string_view input) {
  std::vector<Blueprint> retval;
  for (auto line : splitIntoLinesUntilEmpty(input)) {
    Blueprint current;
    auto words = splitLineIntoWordsFilterEmpty(line);
    auto word_it = words.begin();
    std::advance(word_it, 6);
    current.ore_per_orebot = toNumber<u16>(*word_it);
    std::advance(word_it, 6);
    current.ore_per_claybot = toNumber<u16>(*word_it);
    std::advance(word_it, 6);
    current.ore_per_obsbot = toNumber<u16>(*word_it);
    std::advance(word_it, 3);
    current.clay_per_obsbot = toNumber<u16>(*word_it);
    std::advance(word_it, 6);
    current.ore_per_geodebot = toNumber<u16>(*word_it);
    std::advance(word_it, 3);
    current.obs_per_geodebot = toNumber<u16>(*word_it);
    current.max_ore_cost =
        std::max({current.ore_per_orebot, current.ore_per_claybot,
                  current.ore_per_obsbot, current.ore_per_geodebot});
    retval.push_back(current);
  }
  return retval;
}

struct GameState {
  std::array<u16, 4> resources, bots;
  u16 geodes_cracked, time_remaining;
};

void pushPossibleMoves(const GameState &state, const Blueprint &blueprint,
                       std::vector<GameState> &state_stack) {

  const auto &[ore, clay, obsidian, geodes] = state.resources;
  const auto &[ore_bots, clay_bots, obsidian_bots, geodes_bots] = state.bots;
  auto next_state = state;
  --next_state.time_remaining;
  std::ranges::transform(state.resources, state.bots,
                         next_state.resources.begin(), std::plus{});
  auto &[next_ore, next_clay, next_obsidian, next_geodes] =
      next_state.resources;
  auto &[next_ore_bots, next_clay_bots, next_obsidian_bots, next_geodes_bots] =
      next_state.bots;

  // Buy geode bot, if possible this is always the best move
  if (ore >= blueprint.ore_per_geodebot and
      obsidian >= blueprint.obs_per_geodebot) {
    next_ore -= blueprint.ore_per_geodebot;
    next_obsidian -= blueprint.obs_per_geodebot;
    ++next_geodes_bots;
    state_stack.push_back(next_state);
  } else {
    // Do nothing
    state_stack.push_back(next_state);

    // Buy ore bot
    if (ore >= blueprint.ore_per_orebot and ore_bots < blueprint.max_ore_cost) {
      next_ore -= blueprint.ore_per_orebot;
      ++next_ore_bots;
      state_stack.push_back(next_state);
      next_ore += blueprint.ore_per_orebot;
      --next_ore_bots;
    }

    // Buy obsidian bot
    if (ore >= blueprint.ore_per_obsbot and
        clay >= blueprint.clay_per_obsbot and
        obsidian_bots < blueprint.obs_per_geodebot) {
      next_ore -= blueprint.ore_per_obsbot;
      next_clay -= blueprint.clay_per_obsbot;
      ++next_obsidian_bots;
      state_stack.push_back(next_state);
    } else if // Buy clay bot; obsidian bots are always preferable to clay bots
        (ore >= blueprint.ore_per_claybot and
         clay_bots < blueprint.clay_per_obsbot and
         obsidian_bots < blueprint.obs_per_geodebot) {
      next_ore -= blueprint.ore_per_claybot;
      ++next_clay_bots;
      state_stack.push_back(next_state);
    }
  }
}

u16 getMaxGeodes(const Blueprint &blueprint, u16 time) {
  std::vector<GameState> state_stack;
  GameState initial_state{};
  initial_state.time_remaining = time;
  initial_state.bots.front() = 1; // 1 ore bot
  state_stack.push_back(initial_state);
  u16 max{};
  while (not state_stack.empty()) {
    const auto current = state_stack.back();
    state_stack.pop_back();
    if (current.time_remaining == 0)
      max = std::max(max, current.resources.back());
    else
      pushPossibleMoves(current, blueprint, state_stack);
  }
  return max;
}

void part1(const std::vector<Blueprint> &blueprints) {
  const auto index_range = std::views::iota(1) |
                           std::views::take(blueprints.size()) |
                           std::views::common;
  std::cout << std::transform_reduce(blueprints.begin(), blueprints.end(),
                                     index_range.begin(), 0, std::plus{},
                                     [](const Blueprint &bp, auto index) {
                                       return index * getMaxGeodes(bp, 24);
                                     })
            << '\n';
}

void part2(const std::vector<Blueprint> &blueprints) {
  const auto bp_range = blueprints | std::views::take(3) | std::views::common;
  std::cout << std::transform_reduce(
                   bp_range.begin(), bp_range.end(), 1, std::multiplies{},
                   [](const Blueprint &bp) { return getMaxGeodes(bp, 32); })
            << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  const auto blueprints = parseBlueprints(data);
  part1(blueprints);
  part2(blueprints);
}
