#ifndef MEMORY_H
#define MEMORY_H

uint8_t readByte(unsigned short);
uint8_t readZeroPage(uint8_t);
void writeByte(unsigned short, uint8_t);
void writeZeroPage(uint8_t, uint8_t);
uint8_t popStack(void);
void pushStack(uint8_t);

#define PPUCTRL   0x2000
#define PPUMASK   0x2001
#define PPUSTATUS 0x2002
#define OAMADDR   0x2003
#define OAMDATA   0x2004
#define PPUSCROLL 0x2005
#define PPUADDR   0x2006
#define PPUDATA   0x2007
#define OAMDMA    0x4014

#endif
