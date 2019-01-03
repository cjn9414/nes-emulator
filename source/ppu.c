#include "ppu.h"

struct PPU ppu;

unsigned char readPictureByte(unsigned short addr) {
  addr %= 0x4000;
  
  if (addr >= 0x3000 && addr < 0x3F00) addr -= 0x1000;
  if (addr >= 0x3F20 && addr < 0x4000) {
    addr = addr%0x0020 + 0x3F00;
  }

  if (addr < 0x1000) {
    return ppu.patternTable0[addr];
  } 
  
  else if (addr < 0x2000) {
    return ppu.patternTable1[addr - 0x1000];
  } 
  
  else if (addr < 0x23C0) {
    return ppu.nameTable0[addr - 0x2000];
  }

  else if (addr < 0x2400) {
    return ppu.attrTable0[addr - 0x23C0];
  }

  else if (addr < 0x27C0) {
    return ppu.nameTable1[addr - 0x2400];
  }

  else if (addr < 0x2800) { 
    return ppu.attrTable1[addr - 0x27C0];
  }

  else if (addr < 0x2BC0) { 
    return ppu.nameTable2[addr - 0x2800];
  }
    
  else if (addr < 0x2C00) { 
    return ppu.attrTable2[addr - 0x2BC0];
  }

  else if (addr < 0x2FC0) { 
    return ppu.nameTable3[addr - 0x2C00];
  }

  else {
    return ppu.attrTable3[addr - 0x2FC0];
  }
}
