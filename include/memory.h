#ifndef MEMORY_H
#define MEMORY_H

typedef struct {
  uint8_t PPUControl;  // 0x2000
  uint8_t PPUMask;     // 0x2001
  uint8_t PPUStatus;   // 0x2002
  uint8_t OAMAddress;  // 0x2003
  uint8_t OAMData;     // 0x2004
  uint8_t PPUScroll;   // 0x2005
  uint8_t PPUAddress;  // 0x2006
  uint8_t PPUData;     // 0x2007
  unsigned short PPUWriteLatch;
} MemoryMappedRegisters;

MemoryMappedRegisters ppu_registers;
uint8_t readByte(unsigned short);
uint8_t readZeroPage(uint8_t);
void writeByte(unsigned short, uint8_t);
void writeZeroPage(uint8_t, uint8_t);
uint8_t popStack(void);
void pushStack(uint8_t);

#endif
