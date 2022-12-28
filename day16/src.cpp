#include "../common/common.hpp"

#include <tuple>
#include <unordered_map>

using u32 = std::uint32_t;

auto parseInput(std::string_view data) {
  std::vector<std::vector<std::string_view>> adj_nodes;
  std::vector<u32> node_wgts;
  std::unordered_map<std::string_view, u32> name_map;
  u32 nodeAA{};
  for (u32 node = 0; auto line : splitIntoLinesUntilEmpty(data)) {
    auto words = splitLineIntoWordsFilterEmpty(line);
    auto word_it = std::ranges::begin(words);
    ++word_it;

    const auto name = *word_it;
    if (name == "AA"sv)
      nodeAA = node;
    name_map[name] = node++;
    std::advance(word_it, 3);

    std::string_view rate = *word_it;
    rate.remove_prefix(5);
    rate.remove_suffix(1);
    node_wgts.push_back(toNumber<u32>(rate));
    std::advance(word_it, 5);

    auto &my_adj_nodes = adj_nodes.emplace_back();
    while (std::next(word_it) != std::ranges::end(words)) {
      std::string_view adj = *word_it++;
      adj.remove_suffix(1);
      my_adj_nodes.push_back(adj);
    }
    my_adj_nodes.push_back(*word_it);
  }

  std::vector<std::vector<u32>> edges;
  edges.reserve(adj_nodes.size());
  for (const auto &my_adj_nodes : adj_nodes) {
    auto &e = edges.emplace_back();
    e.reserve(my_adj_nodes.size());
    std::ranges::transform(
        my_adj_nodes, std::back_inserter(e),
        [&](std::string_view adj_node) { return name_map[adj_node]; });
    std::ranges::sort(e);
  }
  return std::make_tuple(std::move(edges), std::move(node_wgts), nodeAA);
}

class SquareSymmetricMatrix {
public:
  SquareSymmetricMatrix(u32 n, u32 v0) : n_{n}, entries_(n * n, v0) {}
  u32 &operator()(u32 i, u32 j) {
    const auto [row, col] = std::minmax(i, j);
    return entries_[row + col * n_];
  }
  u32 operator()(u32 i, u32 j) const {
    const auto [row, col] = std::minmax(i, j);
    return entries_[row + col * n_];
  }

private:
  u32 n_;
  std::vector<u32> entries_;
};

// Floyd-Warshall
auto makeDistanceMatrix(const std::vector<std::vector<u32>> &edges)
    -> SquareSymmetricMatrix {
  SquareSymmetricMatrix retval{static_cast<u32>(edges.size()),
                               std::numeric_limits<u32>::max() /
                                   4}; // avoid overflow
  for (u32 node = 0; const auto &nbrs : edges) {
    for (auto nbr : nbrs)
      retval(node, nbr) = 1;
    retval(node, node) = 0;
    ++node;
  }
  for (u32 k = 0; k < edges.size(); ++k)
    for (u32 i = 0; i < edges.size(); ++i)
      for (u32 j = 0; j < edges.size(); ++j)
        retval(i, j) = std::min(retval(i, j), retval(i, k) + retval(k, j));
  return retval;
}

auto getOpenValves(const std::vector<std::vector<u32>> &edges,
                   const std::vector<u32> &wgts) {
  std::vector<u32> retval;
  std::ranges::copy_if(std::views::iota(0u, edges.size()),
                       std::back_inserter(retval),
                       [&](auto n) { return wgts[n] != 0; });
  return retval;
}

bool updatePath(std::vector<u32> &path, u32 nodes_reached) {
  std::ranges::sort(path | std::views::drop(nodes_reached + 1), std::greater{});
  return std::ranges::next_permutation(path).found;
}

auto runPath(const SquareSymmetricMatrix &dist_mat,
             const std::vector<u32> &wgts, const std::vector<u32> &open_valves,
             auto &&path, u32 start_node, u32 turns)
  requires std::convertible_to<std::ranges::range_value_t<decltype(path)>, u32>
{
  u32 volume{}, flow_rate{}, node = start_node, nodes_reached{};
  for (auto next_ind : path) {
    const auto next = open_valves[next_ind];
    const auto dist = dist_mat(node, next) + 1; // 1 turn to open
    if (dist >= turns)
      break;
    turns -= dist;
    node = next;
    volume += dist * flow_rate;
    flow_rate += wgts[node];
    ++nodes_reached;
  }
  volume += turns * flow_rate;
  return std::make_pair(volume, nodes_reached);
}

void part1(const SquareSymmetricMatrix &dist_mat,
           const std::vector<u32> &open_valves, const std::vector<u32> &wgts,
           u32 start_node) {
  std::vector<u32> path(open_valves.size());
  std::iota(path.begin(), path.end(), 0);
  u32 max_volume = 0;
  for (;;) {
    const auto [volume, nodes_reached] =
        runPath(dist_mat, wgts, open_valves, path, start_node, 30);
    max_volume = std::max(max_volume, volume);
    if (not updatePath(path, nodes_reached))
      break;
  }
  std::cout << max_volume << '\n';
}

auto getUnreachedNodes(const std::vector<u32> &path_humn, u32 reached_nodes) {
  std::vector<u32> retval;
  retval.reserve(path_humn.size() - reached_nodes);
  std::ranges::copy(path_humn | std::views::drop(reached_nodes),
                    std::back_inserter(retval));
  std::ranges::sort(retval);
  return retval;
}

void part2(const SquareSymmetricMatrix &dist_mat,
           const std::vector<u32> &open_valves, const std::vector<u32> &wgts,
           u32 start_node) {
  const auto num_valves = static_cast<u32>(open_valves.size());
  std::vector<u32> path_humn(num_valves);
  std::iota(path_humn.begin(), path_humn.end(), 0);
  u32 max_volume = 0;
  for (;;) {
    u32 num_humn_nodes = num_valves / 2;
    for (; num_humn_nodes <= num_valves; ++num_humn_nodes) {
      const auto path_humn_actual =
          path_humn | std::views::take(num_humn_nodes);
      const auto [volume_humn, nodes_reached_humn] = runPath(
          dist_mat, wgts, open_valves, path_humn_actual, start_node, 26);
      auto path_eleph =
          getUnreachedNodes(path_humn, num_valves - num_humn_nodes);
      for (;;) {
        const auto [volume_eleph, nodes_reached_eleph] =
            runPath(dist_mat, wgts, open_valves, path_eleph, start_node, 26);
        max_volume = std::max(max_volume, volume_humn + volume_eleph);
        if (not updatePath(path_eleph, nodes_reached_eleph))
          break;
      }
      if (nodes_reached_humn <= num_humn_nodes) {
        num_humn_nodes = nodes_reached_humn;
        break;
      }
    }
    if (not updatePath(path_humn, num_humn_nodes))
      break;
  }
  std::cout << max_volume << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  const auto [edges, wgts, start_node] = parseInput(data);
  const auto dist_mat = makeDistanceMatrix(edges);
  const auto open_valves = getOpenValves(edges, wgts);
  part1(dist_mat, open_valves, wgts, start_node);
  part2(dist_mat, open_valves, wgts, start_node);
}
