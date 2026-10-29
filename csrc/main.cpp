#include "Vscpu.h"
#include "verilated.h"
#include "sEMU.h"

#include <cstdio>

static void cycle(Vscpu &dut) {
  dut.clk = 0;
  dut.eval();
  dut.clk = 1;
  dut.eval();
}

static bool check_state(const char *stage, unsigned step,
                        unsigned old_pc, unsigned inst,
                        const Vscpu &dut, const CPUState &ref) {
  bool same = true;
  auto check = [&](const char *name, unsigned actual, unsigned expected) {
    if (actual == expected) return;
    if (same) {
      std::fprintf(stderr, "%s FAIL: step=%u PC_before=0x%02x inst=0x%02x\n",
                   stage, step, old_pc, inst);
    }
    std::fprintf(stderr, "  %s: DUT=0x%02x REF=0x%02x\n",
                 name, actual, expected);
    same = false;
  };

  check("pc", dut.pc, ref.pc);
  const char *names[] = {"r0", "r1", "r2", "r3"};
  for (unsigned i = 0; i < 4; ++i) {
    check(names[i], (dut.gpr_flat >> (8 * i)) & 0xffu, ref.gpr[i]);
  }
  check("led", dut.led, ref.led);
  check("display", dut.display, ref.display);
  return same;
}

static bool run_case(const char *name, const Program &program,
                     unsigned steps, const CPUState &expected,
                     std::uint8_t switches = 0, std::uint8_t buttons = 0) {
  std::printf("[RUN] %s\n", name);
  Vscpu dut;
  SEMU ref;
  ref.reset(program);

  dut.clk = 0;
  dut.rst = 1;
  dut.inst = 0;
  dut.switches = switches;
  dut.buttons = buttons;
  cycle(dut);
  cycle(dut);
  dut.rst = 0;

  if (!check_state("reset", 0, 0, 0, dut, ref.state())) {
    dut.final();
    return false;
  }

  unsigned old_pc = 0;
  unsigned inst = 0;
  for (unsigned step = 1; step <= steps; ++step) {
    old_pc = dut.pc;
    inst = program[old_pc];
    dut.inst = static_cast<std::uint8_t>(inst);
    cycle(dut);
    ref.step(switches, buttons);

    if (!check_state("DiffTest", step, old_pc, inst, dut, ref.state())) {
      dut.final();
      return false;
    }
  }

  const bool ok = check_state("expected state", steps, old_pc, inst,
                              dut, expected);
  dut.final();
  if (ok) std::printf("[PASS] %s (%u instructions)\n", name, steps);
  return ok;
}

int main(int argc, char **argv) {
  Verilated::commandArgs(argc, argv);

  // F6 sISA v2.0. ROM is in C++; scpu fetches through pc/inst.
  const Program sum = {
      0x86,  //  0: li r0, 2 << 2       ; r0 = 8
      0x92,  //  1: li r1, 2
      0x01,  //  2: add r0, r0, r1     ; r0 = 10
      0x90,  //  3: li r1, 0           ; current term
      0xa0,  //  4: li r2, 0           ; sum
      0xb1,  //  5: li r3, 1
      0x17,  //  6: add r1, r1, r3     ; ++term
      0x29,  //  7: add r2, r2, r1     ; sum += term
      0xf9,  //  8: bner0 r1, -2       ; 8 + (-2) = 6
      0x68,  //  9: io out r2, device0 ; LED = 55
      0x69   // 10: io out r2, device1 ; display = 55
  };
  // Six setup instructions + 10*3 loop instructions + two outputs.
  if (!run_case("sum 1..10", sum, 38, {11, {10, 10, 55, 1}, 55, 55}))
    return 1;

  const Program arithmetic = {0x8f, 0x9b, 0xa7, 0xb3, 0x00, 0x00};
  if (!run_case("li shifts and overflow", arithmetic, 6,
                {6, {0, 48, 12, 3}, 0, 0}))
    return 1;

  Program branch{};
  branch[0] = 0x81;   // li r0, 1
  branch[1] = 0xc0;   // bner0 r0, 0: not taken
  branch[2] = 0x92;   // li r1, 2
  branch[3] = 0xdd;   // bner0 r1, +7: 3 -> 10
  branch[10] = 0xe1;  // bner0 r1, -8: 10 -> 2
  if (!run_case("branches", branch, 5, {2, {1, 2, 0, 0}, 0, 0}))
    return 1;

  const Program io = {0x40, 0x51, 0x48, 0x59, 0x80, 0x90,
                      0x62, 0x4a, 0x39, 0x77, 0x5f};
  if (!run_case("io and output retention", io, 11,
                {11, {0, 0, 0, 0}, 0x5a, 0xa5}, 0x5a, 0xa5))
    return 1;

  if (!run_case("pc wraparound", Program{}, 256, CPUState{}))
    return 1;

  std::puts("All tests passed. 1+...+10 = 55 (0x37).");
  return 0;
}
