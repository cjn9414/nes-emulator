// NROM represents the absence of a memory mapper.
#include "mappers.h"
#include "main.h"
#include "ppu.h"

extern unsigned char prg_rom_lower[0x4000];
extern unsigned char prg_rom_upper[0x4000];

extern unsigned char pTable0[0x1000];
extern unsigned char pTable1[0x1000];
extern NameTable nTable0;
extern NameTable nTable1;
extern NameTable nTable2;
extern NameTable nTable3;

// Redeclare external variable.
extern struct Header head;
extern unsigned char * programData;
extern unsigned char * graphicData;

unsigned char NROMSetup(void) {
  if (head.n_chr_banks == 1) {
    memcpy(pTable0, graphicData, 0x1000);
    memcpy(pTable1, graphicData + 0x1000, 0x1000);
  }
  if (head.n_prg_banks == 1) {
    memcpy(prg_rom_lower, programData, 0x4000);
    memcpy(prg_rom_upper, programData, 0x4000);
  } else {
    memcpy(prg_rom_lower, programData, 0x4000);
    memcpy(prg_rom_upper, programData + 0x4000, 0x4000);
  }
}
