/**
 * This function simply makes it easier to get and set all flags within
 * the PPU registers located in CPU memory at $2000-$2007.
 *
 * $2000 - PPUCTRL
 * $2001 - PPUMASK
 * $2002 - PPUSTATUS
 * $2003 - OAMADDR
 * $2004 - OAMDATA
 * $2005 - PPUSCROLL
 * $2006 - PPUADDR
 * $2007 - PPUDATA
 */

#include <stdint.h>

#include "memoryMappedIO.h"

/**
 * The following functions clear/set bits of the memory mapped
 * register at $2000 (Controller register)
 */

void setBaseNameTableAddr(uint8_t bits) {
  ppuRegisters.PPUControl &= ~PPUCTRL_NAME_TBL_MASK;
  ppuRegisters.PPUControl |= bits;
}

void setVRAMIncrement(uint8_t set) {
  if (set) {
    ppuRegisters.PPUControl |= PPUCTRL_VRAM_INC_MASK;  
  } else ppuRegisters.PPUControl &= ~PPUCTRL_VRAM_INC_MASK;
}

void setSpritePatternAddress(uint8_t set) {
  if (set) {
    ppuRegisters.PPUControl |= PPUCTRL_SPRITE_ADDR_MASK;
  } else ppuRegisters.PPUControl &= ~PPUCTRL_SPRITE_ADDR_MASK;
}

void setBackgroundPatternAddress(uint8_t set) {
  if (set) {
    ppuRegisters.PPUControl |= PPUCTRL_BACKGROUND_ADDR_MASK;
  } else ppuRegisters.PPUControl &= ~PPUCTRL_BACKGROUND_ADDR_MASK;
}

void setSpriteSize(uint8_t set) {
  if (set) {
    ppuRegisters.PPUControl |= PPUCTRL_SPRITE_SIZE_MASK;
  } else ppuRegisters.PPUControl &= ~PPUCTRL_SPRITE_SIZE_MASK;
}

void setPPUMasterSlave(uint8_t set) {
  if (set) {
    ppuRegisters.PPUControl |= PPUCTRL_PPU_MASTER_SLAVE_MASK;
  } else ppuRegisters.PPUControl &= ~PPUCTRL_PPU_MASTER_SLAVE_MASK;
}

void setNMIGeneration(uint8_t set) {
  if (set) {
    ppuRegisters.PPUControl |= PPUCTRL_NMI_GEN_MASK;
  } else ppuRegisters.PPUControl &= ~PPUCTRL_NMI_GEN_MASK;
}



/**
 * The following functions gets flags of the memory mapped register
 * at $2000 (Controller register)
 */

uint8_t getBaseNameTableAddress(void) {
  return ppuRegisters.PPUControl & PPUCTRL_NAME_TBL_MASK;
}

uint8_t getVRAMIncrement(void) {
  return ppuRegisters.PPUControl & PPUCTRL_VRAM_INC_MASK;
}

uint8_t getSpritePatternAddress(void) {
  return ppuRegisters.PPUControl & PPUCTRL_SPRITE_ADDR_MASK;	
}

uint8_t getBackgroundPatternAddress(void) {
  return ppuRegisters.PPUControl & PPUCTRL_BACKGROUND_ADDR_MASK;
}

uint8_t getSpriteSize(void) {
  return ppuRegisters.PPUControl & PPUCTRL_SPRITE_SIZE_MASK;
}

uint8_t getPPUMasterSlave(void) {
  return ppuRegisters.PPUControl & PPUCTRL_PPU_MASTER_SLAVE_MASK;
}

uint8_t getNMIGeneration(void) {
  return ppuRegisters.PPUControl & PPUCTRL_NMI_GEN_MASK;
}


/**
 * The following functions clear/set bits of the memory mapped
 * register at $2001 (Mask register)
 */

void setGrayScale(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= PPUMASK_GREYSCALE_MASK;
  } else ppuRegisters.PPUMask &= ~PPUMASK_GREYSCALE_MASK;
}

void setBackgroundLeftEightPixelsActive(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= PPUMASK_SHOW_BACKGROUND_LEFT_MASK;
  } else ppuRegisters.PPUMask &= ~PPUMASK_SHOW_BACKGROUND_LEFT_MASK;
}

void setSpriteLeftEightPixelsActive(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= PPUMASK_SHOW_SPRITES_LEFT_MASK;
  } else ppuRegisters.PPUMask &= ~PPUMASK_SHOW_SPRITES_LEFT_MASK;
}

void setBackground(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= PPUMASK_SHOW_BACKGROUND_MASK;
  } else ppuRegisters.PPUMask &= ~PPUMASK_SHOW_BACKGROUND_MASK;
}

void setSprites(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= PPUMASK_SHOW_SPRITES_MASK;
  } else ppuRegisters.PPUMask &= ~PPUMASK_SHOW_SPRITES_MASK;
}

void setEmphasizeRed(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= PPUMASK_EMPHASIZE_RED_MASK;
  } else ppuRegisters.PPUMask &= ~PPUMASK_EMPHASIZE_RED_MASK;
}

void setEmphasizeGreen(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= PPUMASK_EMPHASIZE_GREEN_MASK;
  } else ppuRegisters.PPUMask &= ~PPUMASK_EMPHASIZE_GREEN_MASK;
}

void setEmphasizeBlue(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= PPUMASK_EMPHASIZE_BLUE_MASK;
  } else ppuRegisters.PPUMask &= ~PPUMASK_EMPHASIZE_BLUE_MASK;
}



/**
 * The following functions get the flags of the memory mapped
 * register at $2001 (Mask register)
 */

uint8_t getGrayScale(void) {
  return ppuRegisters.PPUMask & PPUMASK_GREYSCALE_MASK;
}

uint8_t getBackgroundLeftEightPixelsActive(void) {
  return ppuRegisters.PPUMask & PPUMASK_SHOW_BACKGROUND_LEFT_MASK;
}

uint8_t getSpriteLeftEightPixelsActive(void) {
  return ppuRegisters.PPUMask & PPUMASK_SHOW_SPRITES_LEFT_MASK;
}

uint8_t getBackground(void) {
  return ppuRegisters.PPUMask & PPUMASK_SHOW_BACKGROUND_MASK;
}

uint8_t getSprites(void) {
  return ppuRegisters.PPUMask & PPUMASK_SHOW_SPRITES_MASK;
}

uint8_t getEmphasizeRed(void) {
  return ppuRegisters.PPUMask & PPUMASK_EMPHASIZE_RED_MASK;
}

uint8_t getEmphasizeGreen(void) {
  return ppuRegisters.PPUMask & PPUMASK_EMPHASIZE_GREEN_MASK;
}

uint8_t getEmphasizeBlue(void) {
  return ppuRegisters.PPUMask & PPUMASK_EMPHASIZE_BLUE_MASK;
}


/**
 * The following functions clear/set bits of the memory mapped
 * register at $2002 (Status register)
 */

void setSpriteOverflow(uint8_t set) {
  if (set) {
    ppuRegisters.PPUStatus |= PPUSTATUS_SPRITE_OVERFLOW_MASK;
  } else ppuRegisters.PPUStatus &= ~PPUSTATUS_SPRITE_OVERFLOW_MASK;
}

void setSpriteZeroHits(uint8_t set) {
  if (set) {
    ppuRegisters.PPUStatus |= PPUSTATUS_SPRITE_ZERO_HIT_MASK;
  } else ppuRegisters.PPUStatus &= ~PPUSTATUS_SPRITE_ZERO_HIT_MASK;
}

void setVerticalBlankStart(uint8_t set) {
  if (set) {
    ppuRegisters.PPUStatus |= PPUSTATUS_VBLANK_STARTED_MASK;
  } else ppuRegisters.PPUStatus &= ~PPUSTATUS_VBLANK_STARTED_MASK;
}


/**
 * The following functions get the flags of the memory mapped register
 * at $2002 (Status register)
 */

uint8_t getSpriteOverflow(void) {
  return ppuRegisters.PPUStatus & PPUSTATUS_SPRITE_OVERFLOW_MASK;
}

uint8_t getSpriteZeroHits(void) {
  return ppuRegisters.PPUStatus & PPUSTATUS_SPRITE_ZERO_HIT_MASK;
}

uint8_t getVerticalBlankStart() { 
  return ppuRegisters.PPUStatus & PPUSTATUS_VBLANK_STARTED_MASK;
}


/**
 * The following function writes to the memory mapped
 * register at $2003 (OAM address register)
 */

void OAMAddressWrite(uint8_t addr) {
  ppuRegisters.OAMAddress = addr;
}

/**
 * The following function writes to the memory mapped
 * register at $2004 (OAM data register)
 */

void OAMDataWrite(uint8_t data) {
  ppuRegisters.OAMData = data;
  ppuRegisters.OAMAddress++;

}

/**
 * The following function writes to the memory mapped
 * register at $2005 (Scroll register)
 */

void scrollWrite(uint8_t data) {
  ppuRegisters.PPUScroll = data;
  ppuRegisters.PPUWriteLatch <<= 8;
  ppuRegisters.PPUWriteLatch |= data;
}

/**
 * The following function writes to the memory mapped
 * register at $2006 (Address register)
 */

void addressWrite(uint8_t data) {
  ppuRegisters.PPUAddress = data;
  ppuRegisters.PPUWriteLatch <<= 8;
  ppuRegisters.PPUWriteLatch |= data;
}

/**
 * The following function writes to the memory mapped
 * register at $2007 (Data register)
 */

void dataWrite(uint8_t data) {
  ppuRegisters.PPUData = data;
  writePictureByte();
  ppuRegisters.PPUWriteLatch += (getVRAMIncrement() ? 32 : 1);
}

