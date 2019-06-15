// NROM represents the absence of a memory mapper.
#include <stdint.h>

#include "mappers.h"
#include "main.h"
#include "ppu.h"

extern uint8_t prg_rom_lower[0x4000];
extern uint8_t prg_rom_upper[0x4000];

extern uint8_t pTable0[0x1000];
extern uint8_t pTable1[0x1000];

// Redeclare external variable.
extern struct Header head;
extern uint8_t * programData;
extern uint8_t * graphicData;

uint8_t NROMSetup(void) {
  if (head.n_chr_banks == 1) {
    memcpy(pTable0, graphicData + 0x1000, 0x1000);
    memcpy(pTable1, graphicData + 0x1000, 0x1000);
  }
  if (head.n_prg_banks == 1) {
    memcpy(prg_rom_lower, programData, 0x4000);
    memcpy(prg_rom_upper, programData, 0x4000);
  } else {
    memcpy(prg_rom_lower, programData, 0x4000);
    memcpy(prg_rom_upper, programData + 0x4000, 0x4000);
  }
  return 1;
}
