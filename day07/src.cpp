#include "../common/common.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <vector>

struct File {
  std::string_view name;
  size_t size;
};

struct Directory {
  bool isRoot() const { return parent == nullptr; }

  Directory *parent{};
  size_t size{};
  std::map<std::string_view, std::unique_ptr<Directory>> children;
  std::vector<File> files;
};

void visitPostOrder(auto &&visitor, Directory &dir,
                    std::string_view name = "/"sv)
  requires std::invocable<decltype(visitor), Directory &, std::string_view>
{
  for (auto &[child_name, child_ptr] : dir.children)
    visitPostOrder(visitor, *child_ptr, child_name);
  std::invoke(visitor, dir, name);
}

void visitPostOrder(auto &&visitor, const Directory &dir,
                    std::string_view name = "/"sv)
  requires std::invocable<decltype(visitor), const Directory &,
                          std::string_view>
{
  for (const auto &[child_name, child_ptr] : dir.children)
    visitPostOrder(visitor, *child_ptr, child_name);
  std::invoke(visitor, dir, "/"sv);
}

void visitPreOrder(auto &&visitor, const Directory &dir,
                   std::string_view name = "/"sv)
  requires std::invocable<decltype(visitor), const Directory &,
                          std::string_view>
{
  std::invoke(visitor, dir, name);
  for (const auto &[child_name, child_ptr] : dir.children)
    visitPreOrder(visitor, *child_ptr, child_name);
}

size_t getDirectoryFilesSize(const Directory &dir) {
  return std::transform_reduce(dir.files.begin(), dir.files.end(), 0ul,
                               std::plus{},
                               [](const File &f) { return f.size; });
}

size_t getDirectoryChildrenSize(const Directory &dir) {
  return std::transform_reduce(
      dir.children.begin(), dir.children.end(), 0ul, std::plus{},
      [&](const auto &pair) { return pair.second->size; });
}

void computeSizes(Directory &root) {
  visitPostOrder(
      [](Directory &dir, std::string_view) {
        dir.size = getDirectoryFilesSize(dir) + getDirectoryChildrenSize(dir);
      },
      root);
}

enum struct Command { Ls, CdIn, CdOut, CdRoot };

auto parseCommand(std::string_view line)
    -> std::pair<Command, std::string_view> {
  auto words = splitLineIntoWordsFilterEmpty(line);
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
  auto lines = splitIntoLinesUntilEmpty(data);
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
      auto words = splitLineIntoWordsFilterEmpty(*line_it);
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

  computeSizes(root);
  return root;
}

void print(const Directory &dir, size_t indent = 0) {
  if (dir.isRoot())
    std::cout << "- / (dir, size = " << dir.size << ")\n";
  indent += 2;
  for (const auto &[name, child_ptr] : dir.children) {
    for (size_t i = 0; i < indent; ++i)
      std::cout << ' ';
    std::cout << "- " << name << " (dir, size = " << child_ptr->size << ")\n";
    print(*child_ptr, indent);
  }
  for (const File &f : dir.files) {
    for (size_t i = 0; i < indent; ++i)
      std::cout << ' ';
    std::cout << "- " << f.name << " (file, size = " << f.size << ")\n";
  }
}

void part1(const Directory &root) {
  size_t sum{};
  visitPreOrder(
      [&](const Directory &dir, std::string_view) {
        if (dir.size <= 100'000)
          sum += dir.size;
      },
      root);
  std::cout << "Part 1 solution: " << sum << '\n';
}

void part2(const Directory &root) {
  constexpr size_t total_space = 70'000'000, space_needed = 30'000'000;
  const size_t free_space = total_space - root.size;
  const size_t space_to_free = space_needed - free_space;
  size_t answer = -1;
  std::string_view answer_name{};
  visitPreOrder(
      [&](const Directory &dir, std::string_view name) {
        if (dir.size >= space_to_free and dir.size < answer) {
          answer = dir.size;
          answer_name = name;
        }
      },
      root);
  std::cout << "Part 2 solution: " << answer << " (directory " << answer_name
            << ")\n";
}

int main() {
  const auto [alloc, data] = getStdinView();
  const auto fs = parseFS(data);
  print(fs);
  std::puts("");
  part1(fs);
  part2(fs);
}
