#ifndef MAIN_H
#define MAIN_H

#include <sys/types.h>
#include <stdio.h>

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
extern struct Header head;

signed char loadHeader(FILE*, struct Header*);
unsigned char * cpuStartup(void);
unsigned char * ppuStartup(void);

uint8_t logger;
FILE *logFile;

#endif
