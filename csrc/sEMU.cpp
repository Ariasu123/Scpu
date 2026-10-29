#include "sEMU.h"

void SEMU::reset(const Program &program) {
  state_ = {};
  memory_ = program;
}

void SEMU::step(std::uint8_t switches, std::uint8_t buttons) {
  const std::uint8_t inst = memory_[state_.pc];
  const unsigned opcode = inst >> 6;
  const unsigned rd = (inst >> 4) & 3;
  const unsigned rs1 = (inst >> 2) & 3;
  const unsigned rs2 = inst & 3;
  std::uint8_t next_pc = static_cast<std::uint8_t>(state_.pc + 1);

  switch (opcode) {
    case 0:  // add rd, rs1, rs2
      state_.gpr[rd] = static_cast<std::uint8_t>(
          state_.gpr[rs1] + state_.gpr[rs2]);
      break;

    case 1: {  // io: bit3=0 input, bit3=1 output
      const unsigned idx = inst & 7;
      if (inst & 8) {
        if (idx == 0) state_.led = state_.gpr[rd];
        if (idx == 1) state_.display = state_.gpr[rd];
      } else {
        state_.gpr[rd] = idx == 0 ? switches : idx == 1 ? buttons : 0;
      }
      break;
    }

    case 2: {  // li rd, imm << (s << 1)
      const unsigned s = (inst >> 2) & 3;
      const unsigned imm = inst & 3;
      state_.gpr[rd] = static_cast<std::uint8_t>(imm << (s * 2));
      break;
    }

    case 3: {  // bner0: offset is relative to the CURRENT PC
      if (state_.gpr[0] != state_.gpr[rs2]) {
        const int raw = (inst >> 2) & 15;
        const int offset = raw < 8 ? raw : raw - 16;
        next_pc = static_cast<std::uint8_t>(state_.pc + offset);
      }
      break;
    }
  }

  state_.pc = next_pc;
}
