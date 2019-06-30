#ifndef CPU_H
#define CPU_H

#include <sys/types.h>

typedef enum AddressMode{ ZERO_PAGE, ZERO_PAGE_X, ZERO_PAGE_Y,
                   ABSOLUTE,  ABSOLUTE_X,  ABSOLUTE_Y,
                   INDIRECT,  INDIRECT_X,  INDIRECT_Y,
                   IMPLIED,   IMMEDIATE,   RELATIVE,
                   ACCUMULATOR,
                   INVALID } AddressMode;

typedef struct FunctionExecute {
  union {
    void (*FunctionEx_0Arg)(void);
    void (*FunctionEx_1Arg)(AddressMode);
    void (*FunctionEx_2Arg)(uint8_t, AddressMode);
    void (*FunctionEx_3Arg)(uint8_t, uint8_t, AddressMode);
  }
} FunctionExecute;

struct opcode {
  uint8_t code[3];
  enum AddressMode addrMode;
  u_int8_t operands;
} extern const opcodes[256];

uint32_t step(void);

#endif
