#include <stdint.h>

#include "ppu.h"
#include "display.h"
#include "memoryMappedIO.h"


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


// 30 standard fetch cycles, two pre-render fetch cycles 
#define FETCH_CYCLES_PER_SCANLINE 32

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

// Stores the more recent nametable byte that was fetched.
uint8_t NTByte;

// Declares the image and sprite palettes in PPU memory.
uint8_t imagePalette[0x10];
uint8_t spritePalette[0x10];

/** Four bytes in a fetch cycle
 *  NT Byte, AT Byte, Low BG Tile Byte, High BG Tile Byte
 */
uint8_t pixelBuffer[3*FETCH_CYCLES_PER_SCANLINE];

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
 * Gets a byte at a specific index from the name table.
 * 
 * @param idx: offset in bytes from the start of the name table.
 */
uint8_t fetchNTByte(uint8_t idx) {
  //printf("%X--%X", idx, nTable0.tbl[idx]); 
  return nTable0.tbl[idx];
}


/**
 * Gets a byte at a specific index from the attribute table.
 * 
 * @param idx: offset in bytes from the start of the
 *             attribute table.
 */
void fetchATByte(uint8_t idx) {
  if (cycleType == PRE_FETCH) {
    idx -= FETCH_CYCLES_PER_SCANLINE-2;
    pixelBuffer[idx*3] = nTable0.attr[idx];
  } 
  else {
    pixelBuffer[(idx+2)*3] = nTable0.attr[(idx/4)%8 + 8*(idx/32)];
  }
}


/**
 * Fetches a low background tile byte on a given cycle of the PPU.
 */
void fetchLowBGTileByte(uint8_t idx) {
  if (cycleType == PRE_FETCH) {
    idx -= FETCH_CYCLES_PER_SCANLINE-2;
    pixelBuffer[idx*3+1] = pTable0[idx];
  } 
  else {
    pixelBuffer[(idx+2)*3 + 1] = pTable0[idx];
  }
}

/**
 * Fetches a high background tile byte on a given cycle of the PPU.
 */
void fetchHighBGTileByte(uint8_t idx) {
  pixelBuffer[(idx+2)*3 + 2] = pTable0[idx + 8];
}

/**
 * Send the pixel data that is in the buffer 
 * to the display which renders the scanline.
 */
void flushPixelBuffer(void) {
  renderScanline(pixelBuffer, scanCount);
  memset(pixelBuffer, 0, sizeof(pixelBuffer));
  //for (int i = 0; i < 0x10; i++) {
  //  printf("%X ", imagePalette[i]);
  //}
  //printf("\n");
}

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
    case 1:
      cycleType = STANDARD_FETCH;
      if (scanCount == 0) {
        lineType = VISIBLE;
        setVerticalBlankStart(0);
      }
      else if (scanCount == 240) lineType = POST_RENDER;
      else if (scanCount == 241) {
        lineType = V_BLANK;
        setVerticalBlankStart(1);
      }
      else if (scanCount == 261) lineType = PRE_RENDER;
      break;
    case 241://249:
      cycleType = UNUSED_FETCH;
      break;
    case 258:
      cycleType = H_BLANK;
      break;
    case 321:
      cycleType = PRE_FETCH;
      flushPixelBuffer();
      break;
    case 337:
      cycleType = UNUSED_FETCH;
      break;
    default:
      break;
  }

  if (cycleType == STANDARD_FETCH || cycleType == PRE_FETCH) {
    uint8_t adjust = (cycleType == PRE_FETCH ? 30 : 0);
    if (cycleCount % 8 == 1) {
      NTByte = fetchNTByte(cycleCount/8 + scanCount*32 - adjust);
    } else if (cycleCount % 8 == 3) {
      fetchATByte(cycleCount/8 + scanCount*32 - adjust);
    } else if (cycleCount % 8 == 5) {
      fetchLowBGTileByte(NTByte);
    } else if (cycleCount % 8 == 7) {
      fetchHighBGTileByte(NTByte);
    }
  }
  

  if (cycleCount == 256) { scanCount = (lineType == PRE_RENDER ? 0 : scanCount+1); }
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

void writePictureByte() {
  uint16_t addr = ppuRegisters.PPUWriteLatch;
  uint8_t data = ppuRegisters.PPUData;
  if (addr >= 0x4000) addr %= 0x4000; 

  if (addr >= 0x3F20) addr = 0x3F00 + addr % 0x20;

  if (addr >= 0x3000 && addr < 0x3F00) addr -= 0x1000;

  if (addr < 0x1000) {
    pTable0[addr] = data;
  } 
  else if (addr < 0x2000) {
    pTable1[addr-0x1000] = data;
  } 
  else if (addr < 0x23C0) {
    nTable0.tbl[addr-0x2000] = data;
  }
  else if (addr < 0x2400) {
    nTable0.attr[addr-0x23C0] = data;
  } 
  else if (addr < 0x27C0) {
    nTable1.tbl[addr-0x2400] = data;
  } 
  else if (addr < 0x2800) {
    nTable1.attr[addr-0x27C0] = data;
  } 
  else if (addr < 0x2BC0) {
    nTable2.tbl[addr-0x2800] = data;
  } 
  else if (addr < 0x2C00) {
    nTable2.attr[addr-0x2BC0] = data;
  } 
  else if (addr < 0x2FC0) {
    nTable3.tbl[addr-0x2C00] = data;
  } 
  else if (addr < 0x3000) {
    nTable3.attr[addr-0x2FC0] = data;
  } 
  else if (addr < 0x3F10) {
    imagePalette[addr-0x3F00] = data;
  } 
  else spritePalette[addr-0x3F10] = data;
  ppuRegisters.PPUWriteLatch += ((readPictureByte(0x2000) & 0x04) ? 1 : 32);

}

void devPrintPatternTable0() {
  uint8_t bits;
  for (int i = 0; i < 0x1000; i++) {
    bits =  (pTable0[(i/8)*2 + 8] & 1 << (7-(i%8))) >> (7-(i%8)-1);
    bits |= (pTable0[(i/8)*2    ] & 1 << (7-(i%8))) >> (7-(i%8)  ); 
    printf("%X ", bits); 
  }
}

void devPrintNameTable0() {
  for (int i = 0; i < 0x40; i++) {
    printf("%X ", nTable0.attr[i]);
  }
}

