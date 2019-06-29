#include "registers.h"
#include "memory.h"
#include "main.h"
#include "memoryMappedIO.h"

/**
 * Sets the CPU registers to their expected
 * power-on values.
 *
 * @param regs: Pointer to an instance of
 *              the struct registers.
 */
void cpuRegisterPowerup(struct registers* regs) {
  regs->p = 0x24;
  regs->sp = 0xFD;
  regs->a = 0;
  regs->x = 0;
  regs->y = 0;
  //regs->pc = (readByte(0xFFFD) << 8) + readByte(0xFFFC);
  regs->pc = logger ? 0xC000 : (readByte(0xFFFD) << 8) + readByte(0xFFFC);
}


/**
 * Sets the PPU registers within the CPU to their
 * expected power-on values.
 */
void ppuRegisterPowerup(void) {
  ppuRegisters.PPUControl = 0;
  ppuRegisters.PPUMask = 0;
  ppuRegisters.PPUStatus = 0b10100000;
  ppuRegisters.OAMAddress = 0;
  ppuRegisters.OAMData = 0;
  ppuRegisters.PPUScroll = 0;
  ppuRegisters.PPUAddress = 0;
  ppuRegisters.PPUData = 0;
  ppuRegisters.PPUWriteLatch = 0;
  NMIGenerated = 0;
}
