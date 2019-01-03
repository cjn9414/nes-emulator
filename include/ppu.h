#ifndef PPU_H
#define PPU_H

struct PPU {
  unsigned char patternTable0[0x1000];
  unsigned char patternTable1[0x1000];
  unsigned char nameTable0[0x03C0];
  unsigned char attrTable0[0x0040];
  unsigned char nameTable1[0x03C0];
  unsigned char attrTable1[0x0040];
  unsigned char nameTable2[0x03C0];
  unsigned char attrTable2[0x0040];
  unsigned char nameTable3[0x03C0];
  unsigned char attrTable3[0x0040];
  //unsigned char tableMirror[0x0F00];
  unsigned char imagePalette[0x10];
  unsigned char spritePalette[0x10];
  //unsigned char paletteMirror[0x00E0];
  //unsigned char dataMirror[0xB000];
};

struct color {
  unsigned char entry;
  unsigned char value[3];
};

unsigned char readPictureByte(unsigned short);

#endif
