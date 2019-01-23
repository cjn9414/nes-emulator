// MMC1 memory mapper formally known as SxROM
#include <stdint.h>

#include "mappers.h"
#include "cpu.h"
#include "main.h"
#include "ppu.h"
#define KB 1024

extern struct MMC1 mmc1;
extern struct Header head;
extern uint8_t prg_rom_lower[0x4000];
extern uint8_t prg_rom_upper[0x4000];

extern uint8_t pTable0[0x1000];
extern uint8_t pTable1[0x1000];

extern NameTable nTable0;
extern NameTable nTable1;
extern NameTable nTable2;
extern NameTable nTable3;

extern struct PPU ppu;

// Declare pointers to the cartridge PRG ROM and CHR ROM.
extern uint8_t * programData;
extern uint8_t * graphicData;


/**
 * Resets the shift register for this memory mapper,
 * which happens upon a powerup or reset of the emulator,
 * and during a reset via a mmc1 write.
 */
void mmc1Reset(void) {
  mmc1.shift = 0x10;
}


/**
 * Performs a serial write to the this memory mapper.
 * Every fifth write to the memory mapper ($8000-$FFFF).
 * will result load a bank of memory, depending on the address
 * of the fifth write. Each write with bit 7 clear will  
 * take the first bit of the argument and copy it to the
 * fifth bit of the shift register, after shifting the
 * register contents to the right by one bit.
 *
 * @param addr: The address that the original write is to.
 *        This will determine the register in which the shift
 *        register's contents are being copied into.
 *
 * @param val: The value which is being inspected for a reset
 *        if bit seven is set, or a write/shift if bit
 *        seven is cleared and a desired value is in bit 0.
 */


void mmc1Write(unsigned short addr, uint8_t val) {
  if (getBit(val, 7)) mmc1Reset();
 
 // Fifth bit write, shift and then load shift register
 // into another register.
 if (getBit(mmc1.shift, 0)) {
    mmc1.shift = (mmc1.shift >> 1) | (getBit(val, 0) << 4);
    if (addr > 0x8000 && addr < 0xA000) {
      mmc1.mainControl = mmc1.shift;
    } else if (addr >= 0xA000 && addr < 0xC000) {
      mmc1.chrBank0 = mmc1.shift;
    } else if (addr >= 0xC000 && addr < 0xE000) {
      mmc1.chrBank1 = mmc1.shift;
    } else if (addr >= 0xE000) {
      mmc1.prgBank = mmc1.shift;
    } else {
      printf("Error: Unexpected address at mmc1Write.\n");
      exit(1);
    }
    mmc1Reset();
  } else {
    mmc1.shift = (mmc1.shift >> 1) | (getBit(val, 0) << 4);
  }
}


/**
 * Sets the mmc1 registers to their respective
 * power-on values, and loads this file's pointers
 * to the cartridge data.
 *
 * @param p: program data pointer.
 * @param g: graphical data pointer.
 */
uint8_t MMC1Setup(void) {
  mmc1.mainControl = mmc1.mainControl | 0b00000100;
  mmc1Reset();
  return 1;
}



/**
 * Loads a CHR ROM bank into the ppu memory. Can be loaded into
 * memory in two different ways, based on the value in the 
 * main control register.
 *
 * @param pTablePtr0: Pointer to the start of the first pattern table.
 * @param pTablePtr1: Pointer to the start of the second pattern table.
 */
void loadChrBanks(uint8_t * pTablePtr0, unsigned char * pTablePtr1) {
  // Declare and initialize variables to access memory.
  uint8_t bankNumber = mmc1.chrBank0 & 0b00011111;
  unsigned long addrStart = 8*KB*bankNumber;
  
  // This will load two separate 4 KB banks into memory.
  if (getBit(mmc1.mainControl, 4)) {
    memcpy(graphicData + addrStart, pTablePtr0, 4*KB);
    bankNumber = mmc1.chrBank1 & 0b00011111;
    addrStart = 8*KB*bankNumber;
    memcpy(graphicData + addrStart, pTablePtr1, 4*KB);
  } 
  // This will load one 8 KB bank into memory.
  else {
    memcpy(graphicData + addrStart, pTablePtr0, 4*KB);
    addrStart += 4*KB;
    memcpy(graphicData + addrStart, pTablePtr1, 4*KB);
  }
}


/**
 * Loads a PRG ROM bank into the cpu memory. The first
 * four bits indicate the address of the 16 KB bank(s) in
 * cartridge memory, while the fifth bit indicates
 * the PRG RAM chip is enabled (not yet implemented).
 */
void loadProgramBank() {
  // Declare and initialize variables to access memory.
  uint8_t bankNumber = mmc1.prgBank & 0b00001111;
  unsigned long addrStart = 16*KB*bankNumber;

  // One 16 KB bank is loaded into memory
  if (getBit(mmc1.mainControl, 3)) {
    // Bank is loaded into lower PRG ROM
    if (getBit(mmc1.mainControl, 2)) {
      memcpy(programData + addrStart, prg_rom_lower, 16*KB);
    } 
    // Bank is loaded into upper PRG ROM  
    else memcpy(programData + addrStart, prg_rom_upper, 16*KB);
  } 
  // Two 16 KB banks are loaded into memory.
  else {
    memcpy(programData + addrStart, prg_rom_lower, 16*KB);
    addrStart += 16*KB;
    memcpy(programData + addrStart, prg_rom_upper, 16*KB);
  }
}


