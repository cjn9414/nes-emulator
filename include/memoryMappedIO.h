#ifndef MEMORY_MAPPED_H
#define MEMORY_MAPPED_H

#define PPUCTRL_NAME_TBL_MASK 0b11
#define PPUCTRL_VRAM_INC_MASK (1 << 2)
#define PPUCTRL_SPRITE_ADDR_MASK (1 << 3)
#define PPUCTRL_BACKGROUND_ADDR_MASK (1 << 4)
#define PPUCTRL_SPRITE_SIZE_MASK (1 << 5)
#define PPUCTRL_PPU_MASTER_SLAVE_MASK (1 << 6)
#define PPUCTRL_NMI_GEN_MASK (1 << 7)

#define PPUMASK_GREYSCALE_MASK 1
#define PPUMASK_SHOW_BACKGROUND_LEFT_MASK (1 << 1)
#define PPUMASK_SHOW_SPRITES_LEFT_MASK (1 << 2)
#define PPUMASK_SHOW_BACKGROUND_MASK (1 << 3)
#define PPUMASK_SHOW_SPRITES_MASK (1 << 4)
#define PPUMASK_EMPHASIZE_RED_MASK (1 << 5)
#define PPUMASK_EMPHASIZE_GREEN_MASK (1 << 6)
#define PPUMASK_EMPHASIZE_BLUE_MASK (1 << 7)

#define PPUSTATUS_REG_WRITE_BITS_MASK 0b00011111
#define PPUSTATUS_SPRITE_OVERFLOW_MASK (1 << 5)
#define PPUSTATUS_SPRITE_ZERO_HIT_MASK (1 << 6)
#define PPUSTATUS_VBLANK_STARTED_MASK (1 << 7)



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
