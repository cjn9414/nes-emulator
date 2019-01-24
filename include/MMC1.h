#ifndef MMC1_H
#define MMC1_H

#include <stdint.h>

struct MMC1 {
  uint8_t mainControl;
  uint8_t chrBank0;
  uint8_t chrBank1;
  uint8_t prgBank;
  uint8_t shift;
};

uint8_t MMC1Setup(void);
void mmc1Reset(void);
void mmc1Write(uint16_t, uint8_t);
void loadChrBank0(void);
void loadChrBank1(void);
void loadProgramBank();

#endif
