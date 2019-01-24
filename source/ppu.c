#include <stdint.h>

#include "ppu.h"
#include "visualTest.h"
#include "cpu.h"


#define KB 1024

/**
 * The PPU is idle for 63 PPU cycles (258-320).
 * The previous 16 cycles (249-257) represents an unused tile fetch.
 * The following 16 cycles (321-336) represents the first 
 * two tiles for the next scanline.
 * The remaining cycles (337-340) represent unused fetches.
 * Effective h-blank time for developer purposes is 79 PPU cycles.
 */
#define H_BLANK_START 258

// V-blank starts at scanline 241 (post-render line at 240).
// Pre-render line at scanline 261 (V-blank lasts 20 cycles).
#define V_BLANK_START 241

// Declares the two patten tables in PPU memory.
uint8_t pTable0[0x1000];
uint8_t pTable1[0x1000];

// Declares the four name tables in PPU memory.
NameTable nTable0;  // $2000
NameTable nTable1;  // $2400
NameTable nTable2;  // $2800
NameTable nTable3;  // $2C00

// Stores the mirroring type for the name tables.
// Stores frame and scanline status as an enumerated type.
enum MirroringType mirror;
enum FrameStatus lineType;
enum ScanlineStatus cycleType;

// Stores the current count for the scanline and each cycle within.
unsigned int scanCount, cycleCount; 

// Declares the image and sprite palettes in PPU memory.
uint8_t imagePalette[0x10];
uint8_t spritePalette[0x10];

// Defines the palette for the NES.
const struct color palette[48] = {
  {0x75, 0x75, 0x75},
  {0x27, 0x1B, 0x8F},
  {0x00, 0x00, 0xAB},
  {0x47, 0x00, 0x9F},
  {0x8F, 0x00, 0x77},
  {0xAB, 0x00, 0x13},
  {0xA7, 0x00, 0x00},
  {0x7F, 0x0B, 0x00},
  {0x43, 0x2F, 0x00},
  {0x00, 0x47, 0x00},
  {0x00, 0x51, 0x00},
  {0x00, 0x3F, 0x17},
  {0x1B, 0x3F, 0x5F},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
  {0xBC, 0xBC, 0xBC},
  {0x00, 0x73, 0xEF},
  {0x23, 0x3B, 0xEF},
  {0x83, 0x00, 0xF3},
  {0xBF, 0x00, 0xBF},
  {0xE7, 0x00, 0x5B},
  {0xDB, 0x2B, 0x00},
  {0xCB, 0x4F, 0x0F},
  {0x8B, 0x73, 0x00},
  {0x00, 0x97, 0x00},
  {0x00, 0xAB, 0x00},
  {0x00, 0x93, 0x3B},
  {0x00, 0x83, 0x8B},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
  {0xFF, 0xFF, 0xFF},
  {0x3F, 0xBF, 0xFF},
  {0x5F, 0x97, 0xFF},
  {0xA7, 0x8B, 0xFD},
  {0xF7, 0x7B, 0xFF},
  {0xFF, 0x77, 0xB7},
  {0xFF, 0x77, 0x63},
  {0xFF, 0x9B, 0x3B},
  {0xF3, 0xBF, 0x3F},
  {0x83, 0xD3, 0x13},
  {0x4F, 0xDF, 0x4B},
  {0x58, 0xF8, 0x98},
  {0x00, 0xEB, 0xDB},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
  {0xFF, 0xFF, 0xFF},
  {0xAB, 0xE7, 0xFF},
  {0xC7, 0xD7, 0xFF},
  {0xD7, 0xCB, 0xFF},
  {0xFF, 0xC7, 0xFF},
  {0xFF, 0xC7, 0xDB},
  {0xFF, 0xBF, 0xB3},
  {0xFF, 0xDB, 0xAB},
  {0xFF, 0xE7, 0xA3},
  {0xE3, 0xFF, 0xA3},
  {0xAB, 0xF3, 0xBF},
  {0xB3, 0xFF, 0xCF},
  {0x9F, 0xFF, 0xF3},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
};


/**
 * HORIZONTAL:
 * Maps $2000 and $2400 of the ppu to the first physical name table.
 * Maps $2800 and $2C00 of the ppu to the second physical name table.
 *
 * VERTICAL:
 * Maps $2000 and $2800 of the ppu to the first physical name table.
 * Maps $2400 and $2C00 of the ppu to the second physical name table.
 *
 * ONE-SCREEN:
 * Maps all virtual name tables to the first physical name table.
 *
 * FOUR-SCREEN:
 * Maps each virtual name table to a physical name table using
 * 2KB of RAM in the game cartridge.
 */
void setMirroring(enum MirroringType newMirror) {
    mirror = newMirror;
}


/**
 * Takes the ppu through a single cycle. 
 * The cycle of the ppu takes one-third
 * of the time of a cpu cycle.
 * Uses NTSC timing.
 */
void ppuStep(void) {
  // Handle state of the current scanline and cycle.
  switch (cycleCount) {
    case 0:
      scanCount++;
      break;
    case 1:
      cycleType = VISIBLE;
      if (scanCount == 0) lineType = VISIBLE;
      else if (scanCount == 240) lineType = POST_RENDER;
      else if (scanCount == 241) lineType = V_BLANK;
      else if (scanCount == 261) lineType = PRE_RENDER;
      break;
    case 249:
      cycleType = UNUSED_FETCH;
      break;
    case 258:
      cycleType = H_BLANK;
      break;
    case 321:
      cycleType = PRE_FETCH;
      break;
    case 337:
      cycleType = UNUSED_FETCH;
      break;
    default:
      break;
  }
  
  //TODO: Handle events dependent on the scanline and cycle.

  // Increment the cycle and reset it if it equals 340.
  cycleCount = cycleCount == 340 ? 0 : cycleCount + 1;
}



/**
 * Reads a byte from PPU memory.
 * 
 * @param addr: Address to read byte of PPU memory from.
 *
 * @returns: Value of PPU memory at desired address.
 */
uint8_t readPictureByte(uint16_t addr) {
  // Mirroring occurs every 16 KB in PPU
  addr %= 0x4000;
  
  // Mirroring occurs from $2000-$2EFF at $3000-$3F00.
  if (addr >= 0x3000 && addr < 0x3F00) addr -= 0x1000;
  
  // Mirroring occurs from $3F00-$3F1F at $3F20-$3FFF.
  if (addr >= 0x3F20 && addr < 0x4000) {
    addr = addr%0x0020 + 0x3F00;
  }
  
  // Addressing first pattern table in PPU memory.
  if (addr < 0x1000) {
    return pTable0[addr];
  } 
  // Addressing second pattern table in PPU memory.
  else if (addr < 0x2000) {
    return pTable1[addr - 0x1000];
  } 
  // Addressing first name table in PPU memory.
  else if (addr < 0x23C0) {
    return nTable0.tbl[addr - 0x2000];
  }
  // Addressing first attribute table in PPU memory.
  else if (addr < 0x2400) {
    return nTable0.attr[addr - 0x23C0];
  }
  // Addressing second name table in PPU memory.
  else if (addr < 0x27C0) {
    return nTable1.tbl[addr - 0x2400];
  }
  // Addressing second attribute table in PPU memory.
  else if (addr < 0x2800) { 
    return nTable1.attr[addr - 0x27C0];
  }
  // Addressing third name table in PPU memory.
  else if (addr < 0x2BC0) { 
    return nTable2.tbl[addr - 0x2800];
  } 
  // Addressing third attribute table in PPU memory.
  else if (addr < 0x2C00) { 
    return nTable2.attr[addr - 0x2BC0];
  }
  // Addressing fourth name table in PPU memory.
  else if (addr < 0x2FC0) { 
    return nTable3.tbl[addr - 0x2C00];
  }
  // Addressing fourth attribute table in PPU memory.
  else {
    return nTable3.attr[addr - 0x2FC0];
  }
}


