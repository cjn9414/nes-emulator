#include <stdint.h>

#include "memoryMappedIO.h"

/**
  The following functions clear/set bits of the memory mapped
  register  at $2000 (Controller register)
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
  register  at $2001 (Controller register)
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
  register  at $2002 (Controller register)
*/

/**
  The following functions clear/set bits of the memory mapped
  register  at $2003 (Controller register)
*/

/**
  The following functions clear/set bits of the memory mapped
  register  at $2004 (Controller register)
*/

/**
  The following functions clear/set bits of the memory mapped
  register  at $2005 (Controller register)
*/

/**
  The following functions clear/set bits of the memory mapped
  register  at $2006 (Controller register)
*/

/**
  The following functions clear/set bits of the memory mapped
  register  at $2007 (Controller register)
*/




