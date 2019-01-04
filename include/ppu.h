#ifndef PPU_H
#define PPU_H

typedef struct {
  unsigned char tbl[0x20][0x1E];
  unsigned char attr[0x8][0x8];
} NameTable;

struct color {
  unsigned char rgb[3];
} __attribute__((packed));

void loadPPU(unsigned char *);

unsigned char readPictureByte(unsigned short);

#endif
