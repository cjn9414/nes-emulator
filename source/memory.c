#include "memory.h"
#include "registers.h"
#include "mmc1.h"
extern struct registers regs;

unsigned char ram[0x0800];
unsigned char ppu_reg[0x0008];
unsigned char apu_io_reg[0x0020];
unsigned char exp_rom[0x1FDF];
unsigned char sram[0x2000];
unsigned char prg_rom_lower[0x4000];
unsigned char prg_rom_upper[0x4000];

unsigned char readByte(unsigned short addr) {
  if (addr < 0x2000) {
    return ram[addr];
  }

  else if (addr < 0x4000) {
    return ppu_reg[(addr - 0x2000)];
  }

  else if (addr < 0x4020) {
    return apu_io_reg[addr - 0x4000];
  }

  else if (addr < 0x6000) {
    return exp_rom[addr - 0x4020];
  }

  else if (addr < 0x8000){
    return sram[addr - 0x6000];
  }

  else if (addr < 0xC000) {
    return prg_rom_lower[addr - 0x8000];
  }

  else {
    return prg_rom_upper[addr - 0xC000];
  }
}

unsigned char readZeroPage(unsigned char addr) {
  return ram[addr];
}

void writeByte (unsigned short addr, unsigned char val) {
  if (addr < 0x2000) {
    addr = addr % 0x0800;
    ram[addr] = val;
  }

  else if (addr < 0x4000) {
    addr = 0x2000 + (addr % 0x0008);
  }

  else if (addr < 0x4020) {
    apu_io_reg[addr - 0x4000] = val;
  }

  else if (addr < 0x6000) {
    exp_rom[addr - 0x4020] = val;
  }

  else if (addr < 0x8000){
    sram[addr - 0x6000] = val;
  }

  else if (addr < 0xC000) {
    prg_rom_lower[addr - 0x8000] = val;
    mmc1Write(addr, val);
  }

  else {
    prg_rom_upper[addr - 0xC000] = val;
    mmc1Write(addr, val);
  }
}

void writeZeroPage(unsigned char addr, unsigned char val) {
  ram[addr] = val;
}

unsigned char popStack(void) {
  return ram[--regs.sp];
}

void pushStack(unsigned char val) {
  ram[regs.sp++] = val;
}

