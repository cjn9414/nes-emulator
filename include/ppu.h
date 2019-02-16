#ifndef PPU_H
#define PPU_H

enum MirroringType { HORIZONTAL, VERTICAL, ONE_SCREEN, FOUR_SCREEN };

enum FrameStatus { VISIBLE, V_BLANK, POST_RENDER, PRE_RENDER };

enum ScanlineStatus { STANDARD_FETCH, UNUSED_FETCH, H_BLANK, PRE_FETCH };

//enum InterruptType { IRQ, NMI, RESET, NONE };

typedef struct {
  uint8_t tbl[32*30];
  uint8_t attr[8*8];
} NameTable;

struct color {
  uint8_t rgb[3];
} __attribute__((packed));

void loadPPU(uint8_t *);

uint8_t readPictureByte(uint16_t);

#endif
