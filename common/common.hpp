#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <concepts>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <ranges>
#include <string_view>
#include <utility>
#include <vector>

using namespace std::string_view_literals;

inline auto getStdinView() {
  const auto size = lseek(STDIN_FILENO, 0, SEEK_END);
  const auto ptr = static_cast<char *>(
      mmap(STDIN_FILENO, size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, 0, 0));
  if (!ptr)
    throw std::runtime_error{"mmap failed"};
  const auto deleter = [size = size](char *p) { munmap(p, size); };
  auto unq_ptr = std::unique_ptr<char[], decltype(deleter)>{ptr, deleter};
  return std::make_pair(std::move(unq_ptr), std::string_view(ptr, size));
}

inline auto splitIntoSections(std::string_view in) {
  return in | std::views::split("\n\n"sv) |
         std::views::transform([](auto &&r) { return std::string_view{r}; });
}

inline auto splitIntoLinesUntilEmpty(std::string_view in) {
  return in | std::views::split("\n"sv) |
         std::views::transform([](auto &&r) { return std::string_view{r}; }) |
         std::views::take_while(
             [](std::string_view line) { return not line.empty(); });
}

inline auto splitLineIntoWordsFilterEmpty(std::string_view line) {
  return line | std::views::split(" "sv) |
         std::views::transform([](auto &&r) { return std::string_view{r}; }) |
         std::views::filter(
             [](std::string_view word) { return not word.empty(); });
}

template <typename T>
T toNumber(auto &&str_range)
  requires std::integral<T> or std::floating_point<T>
{
  const auto str = std::string_view{str_range};
  T retval{};
  std::from_chars(str.begin(), str.end(), retval);
  return retval;
}

int toInt(auto &&str) { return toNumber<int>(str); }
