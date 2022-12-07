#include "../common/common.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <numeric>
#include <vector>

struct File {
  std::string_view name;
  size_t size;
};

struct Directory {
  Directory *parent;
  std::map<std::string_view, std::unique_ptr<Directory>> children;
  std::vector<File> files;
};

enum struct Command { Ls, CdIn, CdOut, CdRoot };

auto parseCommand(auto &&line) -> std::pair<Command, std::string_view> {
  auto words =
      line | std::views::split(" "sv) |
      std::views::filter([](auto &&w) { return not std::ranges::empty(w); }) |
      std::views::transform([](auto &&w) { return std::string_view{w}; });
  auto word_it = words.begin();
  if (*word_it++ != "$"sv)
    throw std::runtime_error{"not a command"};

  if (*word_it == "ls"sv)
    return {Command::Ls, {}};
  else if (*word_it++ == "cd"sv) {
    if (*word_it == ".."sv)
      return {Command::CdOut, {}};
    else if (*word_it == "/"sv)
      return {Command::CdRoot, {}};
    else
      return {Command::CdIn, *word_it};
  } else
    throw std::runtime_error{"invalid command"};
}

auto parseFS(std::string_view data) -> Directory {
  auto lines =
      data | std::views::split("\n"sv) | std::views::take_while([](auto &&l) {
        return not std::ranges::empty(l);
      });
  auto line_it = lines.begin();
  Directory root{};
  Directory *wd{};

  const auto makeSubDir = [&](std::string_view name) { // mkdir -p name
    if (auto in_it = wd->children.find(name); in_it != wd->children.end())
      return in_it->second.get();
    auto &new_dir = wd->children[name];
    new_dir = std::make_unique<Directory>();
    new_dir->parent = wd;
    return new_dir.get();
  };
  const auto parseLs = [&] {
    while (line_it != lines.end()) {
      auto words =
          *line_it | std::views::split(" "sv) |
          std::views::filter(
              [](auto &&w) { return not std::ranges::empty(w); }) |
          std::views::transform([](auto &&w) { return std::string_view{w}; });
      auto word_it = words.begin();
      if (*word_it == "$"sv)
        return;
      ++line_it;
      if (*word_it == "dir"sv)
        makeSubDir(*std::next(word_it));
      else
        wd->files.emplace_back(*std::next(word_it), toNumber<size_t>(*word_it));
    }
  };

  while (line_it != lines.end()) {
    const auto [cmd, val] = parseCommand(*line_it++);
    switch (cmd) {
    case Command::Ls:
      parseLs();
      break;
    case Command::CdIn:
      wd = makeSubDir(val);
      break;
    case Command::CdOut:
      wd = wd->parent;
      break;
    case Command::CdRoot:
      wd = std::addressof(root);
    }
  }

  return root;
}

size_t getDirectoryFilesSize(const Directory &dir) {
  return std::transform_reduce(dir.files.begin(), dir.files.end(), 0ul,
                               std::plus{},
                               [](const File &f) { return f.size; });
}

size_t sumOfLessThan(size_t &sum, const Directory &dir, size_t cutoff) {
  size_t dir_files_sz = getDirectoryFilesSize(dir);
  if (dir.children.empty()) {
    if (dir_files_sz <= cutoff)
      sum += dir_files_sz;
    return dir_files_sz;
  }
  const size_t children_sizes =
      std::transform_reduce(dir.children.begin(), dir.children.end(), 0ul,
                            std::plus{}, [&](const auto &pair) {
                              const auto &child = *pair.second;
                              return sumOfLessThan(sum, child, cutoff);
                            });
  const size_t total_sz = dir_files_sz + children_sizes;
  if (total_sz <= cutoff)
    sum += total_sz;
  return total_sz;
}

void part1(const Directory &root) {
  size_t sum{};
  sumOfLessThan(sum, root, 100000);
  std::cout << sum << '\n';
}

// void part2(const Directory &root) {}

int main() {
  const auto [alloc, data] = getStdinView();
  const auto fs = parseFS(data);
  part1(fs);
  // part2(fs);
}
