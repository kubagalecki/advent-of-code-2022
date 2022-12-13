#include "../common/common.hpp"

#include <algorithm>
#include <concepts>
#include <functional>
#include <iostream>
#include <numeric>
#include <stack>
#include <variant>
#include <vector>

struct List {
  using entry_t = std::variant<List, unsigned>;
  std::vector<entry_t> content;

  void print() const {
    std::cout << '[';
    for (const auto &e : content)
      std::visit(
          [](const auto &val) {
            if constexpr (std::same_as<std::decay_t<decltype(val)>, unsigned>)
              std::cout << val << ',';
            else
              val.print();
          },
          e);
    std::cout << ']';
  }
};

bool operator<(const List::entry_t &e1, const List::entry_t &e2);
bool operator<(const List &e1, unsigned e2);
bool operator<(unsigned e1, const List &e2);
bool operator<(const List &e1, const List &e2) {
  return std::ranges::lexicographical_compare(e1.content, e2.content,
                                              std::less{});
}
bool operator<(const List::entry_t &e1, const List::entry_t &e2) {
  return std::visit(std::less{}, e1, e2);
}
bool operator<(const List &e1, unsigned e2) {
  List dummy;
  dummy.content.emplace_back(e2);
  return e1 < dummy;
}
bool operator<(unsigned e1, const List &e2) {
  List dummy;
  dummy.content.emplace_back(e1);
  return dummy < e2;
}

auto parseList(std::string_view line) -> List {
  List root;
  std::stack<List *> list_stack;
  list_stack.push(std::addressof(root));

  line.remove_prefix(1);
  while (not list_stack.empty()) {
    switch (line.front()) {
    case '[':
      list_stack.push(std::addressof(std::get<List>(
          list_stack.top()->content.emplace_back(std::in_place_type<List>))));
      line.remove_prefix(1);
      break;
    case ']':
      list_stack.pop();
      line.remove_prefix(1);
      break;
    case ',':
      line.remove_prefix(1);
      break;
    default: // Assume correct input, only remaining option is an integer
      const auto num_end = line.find_first_of(",]");
      const auto num = line.substr(0, num_end);
      list_stack.top()->content.emplace_back(toNumber<unsigned>(num));
      line.remove_prefix(num_end);
    }
  }
  return root;
}

void part1(std::string_view data) {
  size_t ind_sum = 0;
  for (size_t ind = 1; auto &&sec : splitIntoSections(data)) {
    auto lines = splitIntoLinesUntilEmpty(sec);
    auto lines_begin = std::ranges::begin(lines);
    const auto l1 = parseList(*lines_begin++);
    const auto l2 = parseList(*lines_begin);
    if (l1 < l2)
      ind_sum += ind;
    ++ind;
  }
  std::cout << ind_sum << '\n';
}

// Note: there is a much cleaner way to do this using the <=> operator for both
// sorting and finding the divider packets, but I couldn't get it to compile in
// a reasonable amount of time
void part2(std::string_view data) {
  std::vector<List> lists;
  std::ranges::transform(
      data | std::views::split("\n"sv) | std::views::transform([](auto &&line) {
        return std::string_view{line};
      }) | std::views::filter([](auto line) { return not line.empty(); }),
      std::back_inserter(lists), [](auto line) { return parseList(line); });

  // Add divider packets
  {
    List div_packet;
    auto &nested_list = std::get<List>(
        div_packet.content.emplace_back(std::in_place_type<List>));
    auto &val = nested_list.content.emplace_back(2u);
    lists.push_back(div_packet);
    val = 6u;
    lists.push_back(std::move(div_packet));
  }

  std::sort(lists.begin(), lists.end());

  // Returns true for l == [[value]]
  const auto findNestedVal = [](const List &l, unsigned value) {
    if (l.content.size() == 1)
      if (const auto nested =
              std::get_if<List>(std::addressof(l.content.front()));
          nested)
        if (nested->content.size() == 1)
          if (const auto val_ptr = std::get_if<unsigned>(
                  std::addressof(nested->content.front()));
              val_ptr)
            if (*val_ptr == value)
              return true;
    return false;
  };
  const auto it2 = std::ranges::find_if(
      lists, [&findNestedVal](const List &l) { return findNestedVal(l, 2); });
  const auto it6 = std::ranges::find_if(
      lists, [&findNestedVal](const List &l) { return findNestedVal(l, 6); });
  const auto index2 = std::distance(lists.begin(), it2) + 1;
  const auto index6 = std::distance(lists.begin(), it6) + 1;
  std::cout << index2 * index6 << '\n';
}

int main() {
  const auto [alloc, data] = getStdinView();
  part1(data);
  part2(data);
}
