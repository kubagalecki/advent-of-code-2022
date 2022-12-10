#include "../common/common.hpp"

#include <algorithm>
#include <array>
#include <iostream>

class CPU {
  enum struct State : short { Idle, Adding };

public:
  CPU(std::string_view instr) : instrctions_{instr} {}
  ptrdiff_t getRegister() const { return reg_; }
  void tick() {
    if (state_ == State::Idle)
      incInstrPtr();
    else {
      --counter_;
      if (counter_ == 0) {
        reg_ += val_;
        state_ = State::Idle;
        incInstrPtr();
      }
    }
  }

private:
  auto getNextInstr() -> std::string_view {
    const auto next_endl = instrctions_.find_first_of('\n');
    if (instrctions_.size() < next_endl + 1u)
      throw std::runtime_error{"terminated"};
    auto retval = instrctions_.substr(0, next_endl);
    instrctions_.remove_prefix(next_endl + 1u);
    return retval;
  }
  void incInstrPtr() {
    auto instr = splitLineIntoWordsFilterEmpty(getNextInstr());
    auto it = instr.begin();
    if (*it++ == "addx") {
      state_ = State::Adding;
      val_ = toNumber<short>(*it);
      counter_ = 2;
    }
  }

  std::string_view instrctions_;
  ptrdiff_t reg_{1};
  State state_{State::Idle};
  short counter_{}, val_{};
};

void part1(std::string_view data) {
  CPU cpu{data};
  constexpr auto obs_its = std::array{20, 60, 100, 140, 180, 220};
  ptrdiff_t sig_str = 0, tick = 0;
  for (auto obs_it : obs_its) {
    for (; tick != obs_it; ++tick)
      cpu.tick();
    sig_str += tick * cpu.getRegister();
  }
  std::cout << sig_str << '\n';
}

void part2(std::string_view data) {
  CPU cpu{data};
  const ptrdiff_t width = 40, height = 6;
  for (ptrdiff_t h = 0; h < height; ++h) {
    for (ptrdiff_t w = 0; w < width; ++w) {
      cpu.tick();
      std::cout << (std::abs(cpu.getRegister() - w) <= 1 ? '#' : '.');
    }
    std::cout << '\n';
  }
}

int main() {
  const auto [alloc, data] = getStdinView();
  part1(data);
  part2(data);
}
