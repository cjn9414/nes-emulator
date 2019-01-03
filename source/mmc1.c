#include "mmc1.h"
#include "cpu.h"
#include "main.h"

#define KB 1024

extern struct MMC1 mmc1;
extern struct Header head;
extern unsigned char prg_rom_lower[0x4000];
extern unsigned char prg_rom_upper[0x4000];

unsigned char * programData;
unsigned char * graphicData;

void loadMMC1Ptrs(unsigned char * p, unsigned char * g) {
  programData = p;
  graphicData = g;
}

void mmc1Reset(void) {
  mmc1.shift = 0x10;
}

void mmc1Write(unsigned short addr, unsigned char val) {
  if (getBit(val, 7)) mmc1Reset();
  if (getBit(mmc1.shift, 0)) {
    mmc1.shift = (mmc1.shift >> 1) | getBit(val, 0);
    if (addr > 0x8000 && addr < 0xA000) {
      mmc1.mainControl = mmc1.shift;
    } else if (addr >= 0xA000 && addr < 0xC000) {
      mmc1.chrBank0 = mmc1.shift;
      //loadChrBank0();
    } else if (addr >= 0xC000 && addr < 0xE000) {
      mmc1.chrBank1 = mmc1.shift;
      //loadChrBank1();
    } else if (addr >= 0xE000) {
      mmc1.prgBank = mmc1.shift;
      loadProgramBank();
    } else {
      printf("Unexpected address at mmc1Write.\n");
      exit(1);
    }
    mmc1Reset();
  } else {
    mmc1.shift = (mmc1.shift >> 1) | getBit(val, 0);
  }
}

void mmc1Powerup(void) {
  mmc1.mainControl = mmc1.mainControl | 0b00000100;
  mmc1Reset();
}

//void loadChrBank0(void) {
//    
//}

//void loadChrBank1(void) {
//
//}

void loadProgramBank() {
  unsigned char bankNumber = mmc1.prgBank & 0b00001111;
  unsigned int addrStart = 16*KB*bankNumber;
  if (getBit(mmc1.mainControl, 3)) {
    if (getBit(mmc1.mainControl, 2)) {
      memcpy(programData + addrStart, prg_rom_lower, 16*KB);
    } else memcpy(programData + addrStart, prg_rom_upper, 16*KB);
  } else {
    memcpy(programData + addrStart, prg_rom_lower, 16*KB);
    addrStart += 16*KB;
    memcpy(programData + addrStart, prg_rom_upper, 16*KB);
  }
}


