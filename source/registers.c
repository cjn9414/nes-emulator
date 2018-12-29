#include "registers.h"

void registerPowerup(struct registers* regs) {
  regs->p = 0x34;
  regs->sp = 0xFD;
  regs->a = 0;
  regs->x = 0;
  regs->y = 0;
  regs->pc = 0; // not sure
}
