#ifndef MEMORY_MAPPED_H
#define MEMORY_MAPPED_H

typedef struct {
  uint8_t PPUControl;
  uint8_t PPUMask;
  uint8_t PPUStatus;
  uint8_t OAMAddress;
  uint8_t OAMData;
  uint8_t PPUScroll;
  uint8_t PPUAddress;
  uint8_t PPUData;
  uint16_t PPUWriteLatch; // Actually an 8-bit latch, for now will be ignored
} MemoryMappedRegisters;

MemoryMappedRegisters ppuRegisters;

#endif
