#ifndef MEMORY_H
#define MEMORY_H

typedef struct {
  unsigned char PPUControl;  // 0x2000
  unsigned char PPUMask;     // 0x2001
  unsigned char PPUStatus;   // 0x2002
  unsigned char OAMAddress;  // 0x2003
  unsigned char OAMData;     // 0x2004
  unsigned char PPUScroll;   // 0x2005
  unsigned char PPUAddress;  // 0x2006
  unsigned char PPUData;     // 0x2007
} MemoryMappedRegisters;

MemoryMappedRegisters ppu_registers;
unsigned char readByte(unsigned short);
unsigned char readZeroPage(unsigned char);
void writeByte(unsigned short, unsigned char);
void writeZeroPage(unsigned char, unsigned char);
unsigned char popStack(void);
void pushStack(unsigned char);

#endif
