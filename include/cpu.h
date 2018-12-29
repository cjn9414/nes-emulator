#ifndef CPU_H
#define CPU_H

#include <sys/types.h>

struct opcode {
  unsigned char code[3];
  char mode[11];
  u_int8_t operands;
};

#endif
