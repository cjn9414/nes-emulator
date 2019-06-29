#ifndef CPU_H
#define CPU_H

#include <sys/types.h>

enum AddressMode{ ZERO_PAGE, ZERO_PAGE_X, ZERO_PAGE_Y,
                   ABSOLUTE,  ABSOLUTE_X,  ABSOLUTE_Y,
                   INDIRECT,  INDIRECT_X,  INDIRECT_Y,
                   IMPLIED,   IMMEDIATE,   RELATIVE,
                   ACCUMULATOR,
                   INVALID };

typedef void (*FunctionExecute)(uint8_t, uint8_t);

struct opcode {
  uint8_t code[3];
  enum AddressMode addrMode;
  u_int8_t operands;
} extern const opcodes[256];

void step(uint8_t);

#endif
