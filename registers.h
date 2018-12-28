#ifndef REGISTERS_H
#define REGISTERS_H

struct Registers {
  unsigned short pc;
  unsigned short sp;
  unsigned char p;
  unsigned char a;
  unsigned char x;
  unsigned char y;
};

void registerPowerUp(struct Registers);

#endif
