#ifndef MAIN_H
#define MAIN_H

struct Header {
  u_int8_t trainerBit;
  u_int8_t batteryRamBit;
  u_int8_t fourScreenBit;
  u_int8_t mapperNumber;
  u_int8_t mirror; // 0: horizontal, 1: vertical
  u_int8_t n_prg_banks; // 16 KB chunks
  u_int8_t n_chr_banks; // 8 KB chunks
  u_int8_t n_ram_banks; // 8 KB chunks
};

void loadHeader(FILE*, struct Header*);
unsigned char * cpuStartup(void);
unsigned char * ppuStartup(void);

#endif
