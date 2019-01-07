#ifndef PPU_H
#define PPU_H

typedef struct {
  unsigned char tbl[32*30];
  unsigned char attr[8*8];
} NameTable;

struct color {
  unsigned char rgb[3];
} __attribute__((packed));

void loadPPU(unsigned char *);

unsigned char readPictureByte(unsigned short);

#endif
