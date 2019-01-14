#ifndef PPU_H
#define PPU_H

enum MirroringType { HORIZONTAL, VERTICAL, ONE_SCREEN, FOUR_SCREEN };

enum FrameStatus { VISIBLE, V_BLANK, POST_RENDER, PRE_RENDER };

enum ScanlineStatus { STANDARD_FETCH, UNUSED_FETCH, H_BLANK, PRE_FETCH };

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
