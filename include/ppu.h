#ifndef PPU_H
#define PPU_H

struct color {
  unsigned char entry;
  unsigned char value[3];
};

void loadPPU(unsigned char *);

unsigned char readPictureByte(unsigned short);

#endif
