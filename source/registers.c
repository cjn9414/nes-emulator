#include "registers.h"

/**
 * Sets the CPU registers to their expected
 * power-on values.
 *
 * @param regs: Pointer to an instance of
 *              the struct registers.
 */
void registerPowerup(struct registers* regs) {
  regs->p = 0x34;
  regs->sp = 0xFD;
  regs->a = 0;
  regs->x = 0;
  regs->y = 0;
  regs->pc = 0;
}
