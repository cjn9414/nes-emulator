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

typedef struct {
  uint8_t yPos;
  uint8_t tileIndex;
  uint8_t attributes;
  uint8_t xPos;
} Sprite;

struct color {
  uint8_t rgb[3];
} __attribute__((packed));

void loadPPU(uint8_t *);

uint8_t readPictureByte(uint16_t);

void devPrintPatternTable0(void);
void devPrintNameTable0(void);
uint8_t imagePalette[0x10];
uint8_t spritePalette[0x10];

#endif
