#ifndef SEMU_H
#define SEMU_H

#include <array>
#include <cstdint>

using Program = std::array<std::uint8_t, 256>;

struct CPUState {
  std::uint8_t pc = 0;
  std::array<std::uint8_t, 4> gpr{};
  std::uint8_t led = 0;
  std::uint8_t display = 0;
};

class SEMU {
 public:
  void reset(const Program &program);
  void step(std::uint8_t switches, std::uint8_t buttons);
  const CPUState &state() const { return state_; }

 private:
  CPUState state_{};
  Program memory_{};
};

#endif
