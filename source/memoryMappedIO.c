#include <stdint.h>

#include "memoryMappedIO.h"

/**
  The following functions clear/set bits of the memory mapped
  register at $2000 (Controller register)
*/
void setBaseNameTableAddr(uint8_t bits) {
  uint8_t mask = 0b11111100;
  ppuRegisters.PPUControl &= mask;
  ppuRegisters.PPUControl |= bits;
}

void setVRAMIncrement(uint8_t set) {
  if (set) {
    ppuRegisters.PPUControl |= (1 << 2);  
  } else ppuRegisters.PPUControl &= ~(1 << 2);
}

void setSpritePatternAddress(uint8_t set) {
  if (set) {
    ppuRegisters.PPUControl |= (1 << 3);
  } else ppuRegisters.PPUControl &= ~(1 << 3);
}

void setBackgroundPatternAddress(uint8_t set) {
  if (set) {
    ppuRegisters.PPUControl |= (1 << 4);
  } else ppuRegisters.PPUControl &= ~(1 << 4);
}

void setSpriteSize(uint8_t set) {
  if (set) {
    ppuRegisters.PPUControl |= (1 << 5);
  } else ppuRegisters.PPUControl &= ~(1 << 5);
}

void setPPUMasterSlave(uint8_t set) {
  if (set) {
    ppuRegisters.PPUControl |= (1 << 6);
  } else ppuRegisters.PPUControl &= ~(1 << 6);
}

void setNMIGeneration(uint8_t set) {
  if (set) {
    ppuRegisters.PPUControl |= (1 << 7);
  } else ppuRegisters.PPUControl &= ~(1 << 7);
}


/**
  The following functions clear/set bits of the memory mapped
  register at $2001 (Mask register)
*/

void setGrayScale(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= 1;
  } else ppuRegisters.PPUMask &= 0b11111110;
}

void setBackgroundLeftEightPixelsActive(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= (1 << 1);
  } else ppuRegisters.PPUMask &= ~(1 << 1);
}

void setSpriteLeftEightPixelsActive(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= (1 << 2);
  } else ppuRegisters.PPUMask &= ~(1 << 2);
}

void setBackground(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= (1 << 3);
  } else ppuRegisters.PPUMask &= ~(1 << 3);
}

void setSprites(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= (1 << 4);
  } else ppuRegisters.PPUMask &= ~(1 << 4);
}

void setEmphasizeRed(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= (1 << 5);
  } else ppuRegisters.PPUMask &= ~(1 << 5);
}

void setEmphasizeGreen(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= (1 << 6);
  } else ppuRegisters.PPUMask &= ~(1 << 6);
}

void setEmphasizeBlue(uint8_t set) {
  if (set) {
    ppuRegisters.PPUMask |= (1 << 7);
  } else ppuRegisters.PPUMask &= ~(1 << 7);
}


/**
  The following functions clear/set bits of the memory mapped
  register at $2002 (Status register)
*/

//TODO: make function to set lower five bits of status register

void setSpriteOverflow(uint8_t set) {
  if (set) {
    ppuRegisters.PPUStatus |= (1 << 5);
  } else ppuRegisters.PPUStatus &= ~(1 << 5);
}

void setSpriteZeroHits(uint8_t set) {
  if (set) {
    ppuRegisters.PPUStatus |= (1 << 6);
  } else ppuRegisters.PPUStatus &= ~(1 << 6);
}

void setVerticalBlankStart(uint8_t set) {
  if (set) {
    ppuRegisters.PPUStatus |= (1 << 7);
  } else ppuRegisters.PPUStatus &= ~(1 << 7);
}


/**
  The following functions clear/set bits of the memory mapped
  register at $2003 (OAM address register)
*/

void OAMAddressWrite(uint8_t addr) {
  ppuRegisters.OAMAddress = addr;
}

/**
  The following functions clear/set bits of the memory mapped
  register at $2004 (OAM data register)
*/

void OAMDataWrite(uint8_t data) {
  ppuRegisters.OAMData = data;
}

/**
  The following functions clear/set bits of the memory mapped
  register at $2005 (Scroll register)
*/

void scrollWrite(uint8_t data) {
  ppuRegisters.PPUScroll = data;
  ppuRegisters.PPUWriteLatch <<= 8;
  ppuRegisters.PPUWriteLatch |= data;
}

/**
  The following functions clear/set bits of the memory mapped
  register at $2006 (Address register)
*/

void addressWrite(uint8_t data) {
  ppuRegisters.PPUAddress = data;
  ppuRegisters.PPUWriteLatch <<= 8;
  ppuRegisters.PPUWriteLatch |= data;
}

/**
  The following functions clear/set bits of the memory mapped
  register at $2007 (Data register)
*/

void dataWrite(uint8_t data) {
  ppuRegisters.PPUData = data;
  ppuRegisters.PPUWriteLatch <<= 8;
  ppuRegisters.PPUWriteLatch |= data;
}


