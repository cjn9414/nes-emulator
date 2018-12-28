#include "registers.h"

void registerPowerUp(struct Registers regs) {
  regs.p = 0x34;
  regs.sp = 0xFD;
  regs.accum = 0;
  regs.x = 0;
  regs.y = 0;
  regs.pc = 0; // not sure
}
