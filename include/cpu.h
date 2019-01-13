#ifndef CPU_H
#define CPU_H

#include <sys/types.h>

typedef void (*FunctionExecute)(unsigned char, unsigned char);
struct opcode {
  unsigned char code[3];
  char mode[11];
  u_int8_t operands;
  //void (*opFunction)(unsigned char, unsigned char);
  void *functionExecute;
} extern const opcodes[256];

void step(void);

#endif
