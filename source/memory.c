#include "memory.h"
#include "registers.h"
#include "MMC1.h"

extern struct registers regs;

// Declaring components of CPU memory. 
unsigned char ram[0x0800];
extern MemoryMappedRegisters ppu_registers;
unsigned char apu_io_reg[0x0020];
unsigned char exp_rom[0x1FDF];
unsigned char sram[0x2000];
unsigned char prg_rom_lower[0x4000];
unsigned char prg_rom_upper[0x4000];


/**
 * Obtains a byte of data from the CPU memory.
 *
 * @param addr: Address of data in the CPU.
 *
 * @returns: Value at address in CPU memory.
 */
unsigned char readByte(unsigned short addr) {
  // Mirroring occurs from $2000-$2007 to $2008-$4000.
  if (addr >= 0x2008 && addr < 0x4000) {
    addr = 0x2000 + (addr % 0x0008);
  }

  // Mirroring occurs from $0000-$07FF to $0800-$1FFF.
  if (addr < 0x2000) {
    addr = addr % 0x800;
    // Addressing the RAM of CPU memory.
    return ram[addr];
  }
  // Addressing the PPU registers in CPU memory.
  else if (addr < 0x2008) {
    switch (addr) {
      case 0x2000:
        return ppu_registers.PPUControl;
        break;
      case 0x2001:
        return ppu_registers.PPUMask;
        break;
      case 0x2002:
        return ppu_registers.PPUStatus;
        break;
      case 0x2003:
        return ppu_registers.OAMAddress;
        break;
      case 0x2004:
        return ppu_registers.OAMData;
        break;
      case 0x2005:
        return ppu_registers.PPUScroll;
        break;
      case 0x2006:
        return ppu_registers.PPUAddress;
        break;
      case 0x2007:
        return ppu_registers.PPUData;
        break;
      default:
        printf("Error: Unexpected address to memory mapper I/O registers.\n");
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
unsigned char readZeroPage(unsigned char addr) {
  return ram[addr];
}


/**
 * Write a byte into CPU memory.
 *
 * @param addr: Desired address in CPU memory.
 * @param val: Desired value to write into CPU memory.
 */
void writeByte (unsigned short addr, unsigned char val) {
  // Mirroring occurs from $2000-$2007 to $2008-$4000.
  if (addr >= 0x2008 && addr < 0x4000) {
    addr = 0x2000 + (addr % 0x0008);
  }
  
  // Mirroring occurs from $0000-$07FF to $0800-$1FFF.
  if (addr < 0x2000) {
    addr = addr % 0x0800;
    // Write to CPU RAM.
    ram[addr] = val;
  }
  // Write to PPU registers in CPU memory.
  else if (addr < 0x2008) {
    switch(addr) {
      case 0x2000:
        ppu_registers.PPUControl = val;
        break;
      case 0x2001:
        ppu_registers.PPUMask = val;
        break;
      case 0x2002:
        ppu_registers.PPUStatus = val;
        break;
      case 0x2003:
        ppu_registers.OAMAddress = val;
        break;
      case 0x2004:
        ppu_registers.OAMData = val;
        break;
      case 0x2005:
        ppu_registers.PPUScroll = val;
        break;
      case 0x2006:
        ppu_registers.PPUAddress = val;
        break;
      case 0x2007:
        ppu_registers.PPUData = val;
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
void writeZeroPage(unsigned char addr, unsigned char val) {
  ram[addr] = val;
}


/**
 * Decrements the stack pointer and 
 * removes the top element from the CPU stack
 * 
 * @returns: Top element on the CPU stack.
 */
unsigned char popStack(void) {
  return ram[--regs.sp];
}


/**
 * Places a byte on top of the CPU stack and
 * increments the stack pointer.
 *
 * @param val: Value to place on top of the CPU stack.
 */
void pushStack(unsigned char val) {
  ram[regs.sp++] = val;
}

