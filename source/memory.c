#include <stdint.h>

#include "memory.h"
#include "memoryMappedIO.h"
#include "registers.h"
#include "MMC1.h"

extern struct registers regs;

// Declaring components of CPU memory. 
uint8_t ram[0x0800];
extern MemoryMappedRegisters ppuRegisters;
uint8_t apu_io_reg[0x0020];
uint8_t exp_rom[0x1FDF];
uint8_t sram[PPUCTRL];
uint8_t prg_rom_lower[0x4000];
uint8_t prg_rom_upper[0x4000];

/**
 * Obtains a byte of data from the CPU memory.
 *
 * @param addr: Address of data in the CPU.
 *
 * @returns: Value at address in CPU memory.
 */
uint8_t readByte(uint16_t addr) {
  // Mirroring occurs from $2000-$2007 to $2008-$4000.
  if (addr >= 0x2008 && addr < 0x4000) {
    addr = PPUCTRL + (addr % 0x0008);
  }

  // Mirroring occurs from $0000-$07FF to $0800-$1FFF.
  if (addr < PPUCTRL) {
    addr = addr % 0x800;
   // Addressing the RAM of CPU memory.
    return ram[addr];
  }
  // Addressing the PPU registers in CPU memory.
  else if (addr < 0x2008) {
    switch (addr) {
      case PPUSTATUS:
	{
	uint8_t val = ppuRegisters.PPUStatus;
        ppuRegisters.PPUWriteLatch = 0;
	//ppuRegisters.PPUStatus &= ~(1 << 7);
	//ppuRegisters.PPUStatus |= (NMIGenerated << 7);
	NMIGenerated = 0;
        return val;
	}
      case OAMDATA:
        return ppuRegisters.OAMData;
      case PPUDATA:
        {
        uint8_t val = ppuRegisters.PPUData;
	ppuRegisters.PPUWriteLatch += getVRAMIncrement() ? 32 : 1;
        return val;
        }
      default:
        printf("Error: Unexpected address to memory mapper I/O registers: %X\n", addr);
        exit(1);
    }
  }
  // Addressing the Audio Processing registers in CPU memory.
  else if (addr < 0x4020) {
    return apu_io_reg[addr - 0x4000];
  }
  // Addressing the Expansion ROM registers in CPU memory.
  else if (addr < 0x6000) {
    return exp_rom[addr - 0x4020];
  }
  // Addressing the SRAM in CPU memory.
  else if (addr < 0x8000){
    return sram[addr - 0x6000];
  }
  // Addressing the lower bank of PRG ROM in CPU memory.
  else if (addr < 0xC000) {
    return prg_rom_lower[addr - 0x8000];
  }
  // Addressing the upper bank of PRG ROM in CPU memory.
  else {
    return prg_rom_upper[addr - 0xC000];
  }
}


/**
 * Quick read access to zero page memory in the CPU RAM.
 *
 * @param addr: Address to the desired value in CPU RAM.
 *
 * @returns: Value in CPU RAM based on given address.
 */
uint8_t readZeroPage(uint8_t addr) {
  return ram[addr];
}


/**
 * Write a byte into CPU memory.
 *
 * @param addr: Desired address in CPU memory.
 * @param val: Desired value to write into CPU memory.
 */
void writeByte (uint16_t addr, uint8_t val) {
  // Mirroring occurs from $2000-$2007 to $2008-$4000.
  if (addr >= 0x2008 && addr < 0x4000) {
    addr = PPUCTRL + (addr % 0x0008);
  }
  
  // Mirroring occurs from $0000-$07FF to $0800-$1FFF.
  if (addr < PPUCTRL) {
    addr = addr % 0x0800;
    // Write to CPU RAM.
    ram[addr] = val;
  }
  // Write to PPU registers in CPU memory.
  else if (addr < 0x2008) {
    switch(addr) {
      case PPUCTRL:
        ppuRegisters.PPUControl = val;
        break;
      case PPUMASK:
        ppuRegisters.PPUMask = val;
        break;
      case OAMADDR:
        OAMAddressWrite(val);
        break;
      case OAMDATA:
        OAMDataWrite(val);
        break;
      case PPUSCROLL:
        scrollWrite(val);
        break;
      case PPUADDR:
        addressWrite(val);
        break;
      case PPUDATA:
        dataWrite(val);
        break;
      default:
        printf("Error: Unexpected address to memory mapper I/O registers.\n");
        exit(1);
      }
  }
  // Write to Audio Processing registers in CPU memory.
  else if (addr < 0x4020) {
    apu_io_reg[addr - 0x4000] = val;
  }
  // Write to Expansion ROM in CPU memory.
  else if (addr < 0x6000) {
    exp_rom[addr - 0x4020] = val;
  }
  // Write to SRAM in CPU memory.
  else if (addr < 0x8000){
    sram[addr - 0x6000] = val;
  }
  // Write to PRG ROM bank(s).
  // This actually causes a serial write with MMC1.
  else {
    mmc1Write(addr, val);
  }
}


/**
 * Quick write access to zero page memory in the CPU RAM.
 *
 * @param addr: Address to the desired value in CPU RAM.
 * @param val: Value that should be written to the
 *             specified address in the CPU RAM.
 */
void writeZeroPage(uint8_t addr, uint8_t val) {
  ram[addr] = val;
}


/**
 * Decrements the stack pointer and 
 * removes the top element from the CPU stack
 * 
 * @returns: Top element on the CPU stack.
 */
uint8_t popStack(void) {
  return ram[++regs.sp + 0x100];
}


/**
 * Places a byte on top of the CPU stack and
 * increments the stack pointer.
 *
 * @param val: Value to place on top of the CPU stack.
 */
void pushStack(uint8_t val) {
  ram[regs.sp-- + 0x100] = val;
}

