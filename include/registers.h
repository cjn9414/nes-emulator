#ifndef REGISTERS_H
#define REGISTERS_H

struct registers {
  unsigned short pc;
  unsigned char sp;
  unsigned char p;
  unsigned char a;
  unsigned char x;
  unsigned char y;
};

void registerPowerup(struct registers*);

#endif
