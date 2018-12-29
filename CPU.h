#ifndef CPU_H
#define CPU_H

#include <sys/types.h>

struct opcode {
  unsigned char code[3];
  char mode[11];
  u_int8_t hex;
  u_int8_t operands;
};

unsigned char cpu[0x10000];

#endif
