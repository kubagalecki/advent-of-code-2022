#include <sys/mman.h>
#include <unistd.h>

#include <charconv>
#include <concepts>
#include <memory>
#include <ranges>
#include <string_view>
#include <utility>

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

std::string_view toStringView(std::ranges::contiguous_range auto &&text)
  requires std::same_as<std::ranges::range_value_t<decltype(text)>, char>
{
  auto cv = text | std::views::common;
  return std::string_view{std::ranges::begin(cv), std::ranges::end(cv)};
}
