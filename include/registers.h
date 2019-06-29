#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

struct registers {
  uint16_t pc;
  uint8_t sp;
  uint8_t p;
  uint8_t a;
  uint8_t x;
  uint8_t y;
};

uint8_t NMIGenerated;

void registerPowerup(struct registers*);

#endif
