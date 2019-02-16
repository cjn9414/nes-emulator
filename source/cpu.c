#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "cpu.h"
#include "memory.h"
#include "registers.h"
#include "main.h"

#define KB 1024

extern struct registers regs;
extern uint8_t prg_rom_lower[0x4000];
extern uint8_t prg_rom_upper[0x4000];

void nan(uint8_t, uint8_t);

/**
 * OPCODES WITH ADDITIONAL CYCLE FOR PAGE BOUNDARY CROSSING
 * $11, $19, $1D, $31, $39, $3D, $51, $59, $5D, $71, $79, $7D
 * $B1, $B9, $BD, $D1, $D9, $DD, $F1, $F9, $FD 
 * $BC, $BE
 *
 * OTHER OPCODES WITH ADDITIONAL CYCLES
 * (May also have additional cycle for page boundary)
 * $10, $30, $50, $70, $90, $B0, $D0, $F0
 *
 */

const uint8_t cycles[256] = {
  7, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0,  // 0x0F
  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,  // 0x1F
  6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0,  // 0x2F
  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,  // 0x3F
  6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0,  // 0x4F
  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,  // 0x5F
  6, 6, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0,  // 0x6F
  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,  // 0x7F
  0, 6, 0, 0, 3, 3, 3, 0, 2, 0, 2, 0, 4, 4, 4, 0,  // 0x8F
  2, 6, 0, 0, 4, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0,  // 0x9F
  2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,  // 0xAF
  2, 5, 0, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0,  // 0xBF
  2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,  // 0xCF
  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,  // 0xDF
  2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,  // 0xEF
  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0   // 0xFF
};

/**
 * The next set of functions will either set or clear a flag
 * in the status register. For each function:
 *
 * @param bit: 0 to clear a flag, anything else to set a flag.
 *
 * @returns: Nothing.
 */

void setFlagCarry(uint8_t bit)  {
  regs.p = bit ? regs.p | 0b00000001 : regs.p & 0b11111110;
}
void setFlagZero(uint8_t bit) { 
  regs.p = bit ? regs.p | 0b00000010 : regs.p & 0b11111101; 
}

void setFlagInterrupt(uint8_t bit) {
  regs.p = bit ? regs.p | 0b00000100 : regs.p & 0b11111011; 
}

void setFlagDecimal(uint8_t bit) {
  regs.p = bit ? regs.p | 0b00001000 : regs.p & 0b11110111; 
}

void setFlagBreak(uint8_t bit) { 
  regs.p = bit ? regs.p | 0b00010000 : regs.p & 0b11101111;
}

void setFlagOverflow(uint8_t bit) { 
  regs.p = bit ? regs.p | 0b01000000 : regs.p & 0b10111111;
}

void setFlagNegative(uint8_t bit) {
  regs.p = bit ? regs.p | 0b10000000 : regs.p & 0b01111111;
}


/**
 * Retrieves a selected bit from a given byte.
 *
 * @param byte: Byte in which a bit is retrieved from.
 * @param bit: Bit (7...0) that is retrieved from the byte.
 *
 * @returns: 1 or 0, if the selected bit is set or not.
 */
uint8_t getBit(uint8_t byte, uint8_t bit) {
  bit = (byte & (1 << bit)) >> bit;
  return bit;
}


/**
 * The next set of functions will retrieve a flag
 * from the status register. For each function:
 *
 * @returns: Flag value. Returns 1 for set or 0 for clear.
 */

uint8_t getFlagCarry(void) { return getBit(regs.p, 0); }

uint8_t getFlagZero(void) { return getBit(regs.p, 1); }

uint8_t getFlagInterrupt(void) { return getBit(regs.p, 2); }

uint8_t getFlagDecimal(void) { return getBit(regs.p, 3); }

uint8_t getFlagBreak(void) { return getBit(regs.p, 4); }

uint8_t getFlagOverflow(void) { return getBit(regs.p, 6); }

uint8_t getFlagNegative(void) { return getBit(regs.p, 7); }

/**
 * Helper function to set flags with all compare functions.
 * 
 * @param a: primary value of comparison to compare function.
 * @param b: operand to compare function.
 */
void flagCompare(uint8_t a, uint8_t b) {
  SZFlags(a - b);
  if (a >= b) {
    setFlagCarry(1);
  } else { setFlagCarry(0); }
}

/**
 * Determines if the overflag should be set after an instruction.
 *
 * @param a: First argument to the instruction.
 * @param b: Second argument to the instruction. 
 * @param c: Resolution to the instruction.
 */
void VFlag(uint8_t a, uint8_t b, uint8_t c) {
  if ((!getBit(a, 7) && !getBit(b, 7) && getBit(c, 7)) ||
      (getBit(a, 7) && getBit(b, 7) && !getBit(c, 7))) {
    setFlagOverflow(1);
  } else setFlagOverflow(0);
}

/**
 * Determines if the Zero and Negative flags should be
 * set after an instruction.
 *
 * @param val: Resolution to an instruction.
 */
void SZFlags(uint8_t val) {
  if (val == 0) {
    setFlagZero(1);
    setFlagNegative(0);
  }
  else if (getBit(val, 7)) {
    setFlagZero(0);
    setFlagNegative(1);
  }
  else {
    setFlagZero(0);
    setFlagNegative(0);
  }
}

/**
 * Helper function to branch instructions.
 * Handles two's complement argument for the
 * branch instruction.
 *
 * @param val: Signed program counter offset.
 *             Negative numbers move the
 *             program counter backwards.
 */
void branchJump(uint8_t val) {
  if (!getBit(val, 7)) {
    regs.pc += val;
  } else {
    val = ~val + 1;
    regs.pc -= val;
  }
}


/*************************************/
/* START OF OFFICIAL OPCODE FUNCTIONS*/
/*************************************/

void nan(uint8_t garb0, uint8_t garb1) {
  printf("Error: Invalid opcode!.\n");
  exit(1);
}

void brk(uint8_t garb0, uint8_t garb1) { setFlagBreak(1); }  //0x00

void adc_imm(uint8_t val, uint8_t res) {
  res = val + regs.a + getFlagCarry();
  VFlag(val, regs.a, res);
  res < regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a);
}

void adc_zp(uint8_t val, uint8_t res) {
  val = readZeroPage(val);
  res = val + regs.a + getFlagCarry();
  VFlag(val, regs.a, res);
  res < regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a);
}

void adc_zp_x(uint8_t val, uint8_t res) {
  val = readZeroPage(val + regs.x);
  res = val + regs.a + getFlagCarry();
  VFlag(val, regs.a, res);
  res < regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a);
}

void adc_ind_x(uint8_t val, uint8_t res) {
  uint16_t addr = readZeroPage(val + regs.x) + (readZeroPage(val + regs.x + 1) << 8);
  val = readByte(addr);
  res = val + regs.a + getFlagCarry();
  VFlag(val, regs.a, res);
  res < regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a);
}

void adc_ind_y(uint8_t val, uint8_t res) {
  uint16_t addr = readZeroPage(val) + (readZeroPage(val + 1) << 8);
  val = readByte(addr + regs.y);
  res = val + regs.a + getFlagCarry();
  VFlag(val, regs.a, res);
  res < regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a);
}

void adc_abs(uint8_t lower, uint8_t upper) {
  uint8_t res;
  uint16_t addr = (upper << 8) + lower;
  lower = readByte(addr);
  res = lower + regs.a + getFlagCarry();
  VFlag(lower, regs.a, res);
  res < regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a); 
}

void adc_abs_x(uint8_t lower, uint8_t upper) {
  uint8_t res;
  uint16_t addr = (upper << 8) + lower + regs.x;
  lower = readByte(addr);
  res = lower + regs.a + getFlagCarry();
  VFlag(lower, regs.a, res);
  res < regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a); 
}

void adc_abs_y(uint8_t lower, uint8_t upper) {
  uint8_t res;
  uint16_t addr = (upper << 8) + lower + regs.y;
  lower = readByte(addr);
  res = lower + regs.a + getFlagCarry();
  VFlag(lower, regs.a, res);
  res < regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a); 
}

void and_ind_x(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(regs.x + val + 1) << 8) + readZeroPage(regs.x + val); 
  val = regs.a & readByte(addr);
  SZFlags(val);
  regs.a = val;
};

void and_ind_y(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(val + 1) << 8) + readZeroPage(val); 
  val = regs.a & readByte(addr + regs.y);
  SZFlags(val);
  regs.a = val;
}

void and_imm(uint8_t val, uint8_t garb) {
  val = regs.a & val;
  SZFlags(val);
  regs.a = val;
}

void and_zp(uint8_t val, uint8_t garb) {
  val = readZeroPage(val) & regs.a;
  SZFlags(val);
  regs.a = val;
}

void and_zp_x(uint8_t val, uint8_t garb) {
  val = readZeroPage(val+regs.x) & regs.a;
  SZFlags(val);
  regs.a = val;
}

void and_abs(uint8_t lower, uint8_t upper) {  
  uint16_t addr = (upper << 8) + lower;
  lower = readByte(addr) & regs.a;
  SZFlags(lower);
  regs.a = lower;
}

void and_abs_x(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.x;
  lower = readByte(addr) & regs.a;
  SZFlags(lower);
  regs.a = lower;
}

void and_abs_y(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.y;
  lower = readByte(addr) & regs.a;
  SZFlags(lower);
  regs.a = lower;
}

void asl_acc(uint8_t garb0, uint8_t garb1) { 
  setFlagCarry(getBit(regs.a, 7));
  regs.a = regs.a << 1;
  SZFlags(regs.a);
}

void asl_zp(uint8_t addr, uint8_t val) {
  val = readZeroPage(addr);
  setFlagCarry(getBit(val, 7));
  val = val << 1;
  writeZeroPage(addr, val); 
  SZFlags(val);
}

void asl_zp_x(uint8_t addr, uint8_t val) {
  addr += regs.x;
  val = readZeroPage(addr);
  setFlagCarry(getBit(val, 7));
  val = val << 1;
  writeZeroPage(addr, val);
  SZFlags(val);
}

void asl_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  lower = readByte(addr);
  setFlagCarry(getBit(lower, 7));
  lower = lower << 1;
  writeByte(addr, lower);
}

void asl_abs_x(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.x;
  lower = readByte(addr);
  setFlagCarry(getBit(lower, 7));
  lower = lower << 1;
  writeByte(addr, lower);
}

void bit_zp(uint8_t val, uint8_t garb) {
  val = readZeroPage(val);
  setFlagNegative(getBit(val, 7));
  setFlagOverflow(getBit(val, 6));
  setFlagZero((val & regs.a) != 0 ? 0 : 1);
}

void bit_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  lower = readByte(addr);
  setFlagNegative(getBit(lower, 7));
  setFlagOverflow(getBit(lower, 6));
  setFlagZero((lower & regs.a) != 0 ? 0 : 1);
}

void bpl(uint8_t val, uint8_t garb) {  
  if (!getFlagNegative()) {
    branchJump(val);
  }
}

void bmi(uint8_t val, uint8_t garb) {  
  if (getFlagNegative()) {
    branchJump(val);
  }
}

void bvc(uint8_t val, uint8_t garb) {  
  if (!getFlagOverflow()) {
    branchJump(val);
  }
}

void bvs(uint8_t val, uint8_t garb) {  
  if (getFlagOverflow()) {
    branchJump(val);
  }
}

void bcc(uint8_t val, uint8_t garb) {  
  if (!getFlagCarry()) {
    branchJump(val);
  }
}

void bcs(uint8_t val, uint8_t garb) {  
  if (getFlagCarry()) {
    branchJump(val);
  }
}

void bne(uint8_t val, uint8_t garb) {  
  if (!getFlagZero()) {
    branchJump(val);
  }
}

void beq(uint8_t val, uint8_t garb) {  
  if (getFlagZero()) {
    branchJump(val);
  }
}

void cmp_imm(uint8_t val, uint8_t garb) {
  flagCompare(regs.a, val);
}

void cmp_zp(uint8_t val, uint8_t garb) {
  val = readZeroPage(val);
  flagCompare(regs.a, val);
}

void cmp_zp_x(uint8_t val, uint8_t garb) {
  val = readZeroPage(val + regs.x);
  flagCompare(regs.a, val);
}

void cmp_ind_x(uint8_t val, uint8_t garb) {
  uint16_t addr;
  addr = (readZeroPage(val + regs.x + 1) << 8) + readZeroPage(val + regs.x);
  val = readByte(addr);
  flagCompare(regs.a, val);
}

void cmp_ind_y(uint8_t val, uint8_t garb) {
  uint16_t addr;
  addr = (readZeroPage(val + 1) << 8) + readZeroPage(val);
  val = readByte(addr + regs.y);
  flagCompare(regs.a, val);
}

void cmp_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  lower = readByte(addr);
  flagCompare(regs.a, lower);
}

void cmp_abs_x(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.x;
  lower = readByte(addr);
  flagCompare(regs.a, lower);
}

void cmp_abs_y(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.y;
  lower = readByte(addr);
  flagCompare(regs.a, lower);
}

void cpx_imm(uint8_t val, uint8_t garb) {
  flagCompare(regs.x, val);
}

void cpx_zp(uint8_t val, uint8_t garb) {
  val = readZeroPage(val);
  flagCompare(regs.x, val);
}

void cpx_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  lower = readByte(addr);
  flagCompare(regs.x, lower);
}

void cpy_imm(uint8_t val, uint8_t garb) {
  flagCompare(regs.y, val);
}

void cpy_zp(uint8_t val, uint8_t garb) {
  val = readZeroPage(val);
  flagCompare(regs.y, val);
}

void cpy_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  lower = readByte(addr);
  flagCompare(regs.y, lower);
}



void dec_zp(uint8_t addr, uint8_t garb) {
  garb = readZeroPage(addr) - 1;
  writeZeroPage(addr, garb);
  SZFlags(garb);
}

void dec_zp_x(uint8_t addr, uint8_t garb) {
  addr += regs.x;
  garb = readZeroPage(addr) - 1;
  writeZeroPage(addr, garb);
  SZFlags(garb);
}

void dec_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  lower = readByte(addr) - 1;
  writeByte(addr, lower);
  SZFlags(lower);
}

void dec_abs_x(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.x;
  lower = readByte(addr) - 1;
  writeByte(addr, lower);
  SZFlags(lower);
}

void eor_imm(uint8_t val, uint8_t garb) {
  regs.a = regs.a ^ val;
  SZFlags(regs.a);
}

void eor_zp(uint8_t val, uint8_t garb) {
  regs.a = regs.a ^ readZeroPage(val);
  SZFlags(regs.a);
}

void eor_zp_x(uint8_t val, uint8_t garb) {
  regs.a = regs.a ^ readZeroPage(val + regs.x);
  SZFlags(regs.a);
}

void eor_ind_x(uint8_t val, uint8_t garb) {
  uint16_t addr;
  addr = (readZeroPage(val + regs.x + 1) << 8) + readZeroPage(val + regs.x);
  regs.a = regs.a ^ readByte(addr);
  SZFlags(regs.a);
}

void eor_ind_y(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(val + 1) << 8) + readZeroPage(val);
  regs.a = regs.a ^ readByte(addr + regs.y);
  SZFlags(regs.a);
}

void eor_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  regs.a = regs.a ^ readByte(addr);
  SZFlags(regs.a);
}

void eor_abs_x(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.x;
  regs.a = regs.a ^ readByte(addr);
  SZFlags(regs.a);
}

void eor_abs_y(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.y;
  regs.a = regs.a ^ readByte(addr);
  SZFlags(regs.a);
}

void clc(uint8_t garb0, uint8_t garb1) { setFlagCarry(0); }

void sec(uint8_t garb0, uint8_t garb1) { setFlagCarry(1); }

void cli(uint8_t garb0, uint8_t garb1) { setFlagInterrupt(0); }

void sei(uint8_t garb0, uint8_t garb1) { setFlagInterrupt(1); }

void clv(uint8_t garb0, uint8_t garb1) { setFlagOverflow(0); }

void cld(uint8_t garb0, uint8_t garb1) { setFlagDecimal(0); }

void sed(uint8_t garb0, uint8_t garb1) { setFlagDecimal(1); }

void inc_zp(uint8_t addr, uint8_t garb) {
  garb =  readZeroPage(addr) + 1;
  writeZeroPage(addr, garb);
  SZFlags(garb);
}

void inc_zp_x(uint8_t addr, uint8_t garb) {
  addr += regs.x;
  garb = readZeroPage(addr) + 1;
  writeZeroPage(addr, garb);
  SZFlags(garb);
}

void inc_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  lower = readByte(addr) + 1;
  writeByte(addr, lower);
  SZFlags(lower);
}

void inc_abs_x(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.x;
  lower = readByte(addr) + 1;
  writeByte(addr, lower);
  SZFlags(lower);
}

void jmp_abs(uint8_t lower, uint8_t upper) {
  regs.pc = (upper << 8) + lower;
}

void jmp_ind(uint8_t lower, uint8_t upper) {
  uint16_t addr = lower + (upper << 8);
  regs.pc = readByte(addr) + (readByte(addr + 
  (addr % 0x100 == 0xFF ? -0xFF : 1)) << 8);
}

void jsr(uint8_t lower, uint8_t upper) {
  uint16_t val = regs.pc + 3 - 1;
  pushStack(val >> 8);
  pushStack(val & 0x00FF);
  val = (upper << 8) + lower;
  regs.pc = val;
}

void ldx_imm(uint8_t addr, uint8_t garb) {
  regs.x = addr;
  SZFlags(regs.x);
}

void ldx_zp(uint8_t addr, uint8_t garb) {
  regs.x = readZeroPage(addr);
  SZFlags(regs.x);
}

void ldx_zp_y(uint8_t addr, uint8_t garb) {
  regs.x = readZeroPage(addr + regs.y);
  SZFlags(regs.x);
}

void ldx_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  regs.x = readByte(addr);
  SZFlags(regs.x);
}

void ldx_abs_y(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.y;
  regs.x = readByte(addr);
  SZFlags(regs.x);
}

void ldy_imm(uint8_t addr, uint8_t garb) {
  regs.y = addr;
  SZFlags(regs.y);
}

void ldy_zp(uint8_t addr, uint8_t garb) {
  regs.y = readZeroPage(addr);
  SZFlags(regs.y);
}

void ldy_zp_x(uint8_t addr, uint8_t garb) {
  regs.y = readZeroPage(addr + regs.x);
  SZFlags(regs.y);
}

void ldy_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  regs.y = readByte(addr);
  SZFlags(regs.y);
}

void ldy_abs_x(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.x;
  regs.y = readByte(addr);
  SZFlags(regs.y);
}


void lda_imm(uint8_t addr, uint8_t garb) {
  regs.a = addr;
  SZFlags(regs.a);
}

void lda_zp(uint8_t addr, uint8_t garb) {
  regs.a = readZeroPage(addr);
  SZFlags(regs.a);
}

void lda_zp_x(uint8_t addr, uint8_t garb) {
  regs.a = readZeroPage(addr + regs.x);
  SZFlags(regs.a);
}

void lda_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  regs.a = readByte(addr);
  SZFlags(regs.a);
}

void lda_abs_x(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.x;
  regs.a = readByte(addr);
  SZFlags(regs.a);
}

void lda_abs_y(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.y;
  regs.a = readByte(addr);
  SZFlags(regs.a);
}

void lda_ind_x(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(val + regs.x + 1) << 8) + readZeroPage(val + regs.x);
  regs.a = readByte(addr);
  SZFlags(regs.a);
}

void lda_ind_y(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(val + 1) << 8) + readZeroPage(val);
  regs.a = readByte(addr + regs.y);
  SZFlags(regs.a);
}

void lsr_acc(uint8_t garb0, uint8_t garb1) { 
  setFlagCarry(getBit(regs.a, 0));
  regs.a = regs.a >> 1;
  SZFlags(regs.a);
}

void lsr_zp(uint8_t addr, uint8_t garb) {
  uint8_t val = readZeroPage(addr);
  setFlagCarry(getBit(val, 0));
  val = val >> 1;
  writeZeroPage(addr, val); 
  SZFlags(val);
}

void lsr_zp_x(uint8_t addr, uint8_t garb) {
  addr += regs.x;
  uint8_t val = readZeroPage(addr);
  setFlagCarry(getBit(val, 0));
  val = val >> 1;
  writeZeroPage(addr, val);
  SZFlags(val);
}

void lsr_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  lower = readByte(addr);
  setFlagCarry(getBit(lower, 0));
  lower = lower >> 1;
  writeByte(addr, lower);
}

void lsr_abs_x(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.x;
  lower = readByte(addr);
  setFlagCarry(getBit(lower, 0));
  lower = lower >> 1;
  writeByte(addr, lower);
}

void nop(uint8_t garb0, uint8_t garb1) { return; }

void ora_ind_x(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(regs.x + val + 1) << 8) + readZeroPage(regs.x + val); 
  val = regs.a | readByte(addr);
  SZFlags(val);
  regs.a = val;
};

void ora_ind_y(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(val + 1) << 8) + readZeroPage(val); 
  val = regs.a | readByte(addr + regs.y);
  SZFlags(val);
  regs.a = val;
}

void ora_imm(uint8_t val, uint8_t garb) {   //0x09
  val = regs.a | val;
  SZFlags(val);
  regs.a = val;
}

void ora_zp(uint8_t val, uint8_t garb) {  //0x05
  val = readZeroPage(val) | regs.a;
  SZFlags(val);
  regs.a = val;
}

void ora_zp_x(uint8_t val, uint8_t garb) {  // 0x15
  val = readZeroPage(val+regs.x) | regs.a;
  SZFlags(val);
  regs.a = val;
}

void ora_abs(uint8_t lower, uint8_t upper) {  
  uint16_t addr = (upper << 8) + lower;
  lower = readByte(addr) | regs.a;
  SZFlags(lower);
  regs.a = lower;
}

void ora_abs_x(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.x;
  lower = readByte(addr) | regs.a;
  SZFlags(lower);
  regs.a = lower;
}

void ora_abs_y(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.y;
  lower = readByte(addr) | regs.a;
  SZFlags(lower);
  regs.a = lower;
}

void tax(uint8_t garb0, uint8_t garb1) {
  regs.x = regs.a;
  SZFlags(regs.x);  
}

void txa(uint8_t garb0, uint8_t garb1) {
  regs.a = regs.x;
  SZFlags(regs.a);
}

void dex(uint8_t garb0, uint8_t garb1) {
  regs.x--;
  SZFlags(regs.x);
}

void inx(uint8_t garb0, uint8_t garb1) {
  regs.x++;
  SZFlags(regs.x);
}

void tay(uint8_t garb0, uint8_t garb1) {
  regs.y = regs.a;
  SZFlags(regs.y);
}

void tya(uint8_t garb0, uint8_t garb1) { 
  regs.a = regs.y;
  SZFlags(regs.a);
}

void dey(uint8_t garb0, uint8_t garb1) { 
  regs.y--;
  SZFlags(regs.y);
}

void iny(uint8_t garb0, uint8_t garb1) { 
  regs.y++;
  SZFlags(regs.y);
}

void rol_acc(uint8_t garb0, uint8_t garb1) { 
  uint8_t msb = getFlagCarry();
  setFlagCarry(regs.a >> 7);
  regs.a = msb | (regs.a << 1);
  SZFlags(regs.a);
}

void rol_zp(uint8_t addr, uint8_t garb) {
  uint8_t msb = getFlagCarry();
  uint8_t val = readZeroPage(addr);
  setFlagCarry(val >> 7);
  val = msb | (val << 1);
  writeByte(addr, val);
  SZFlags(val);
}

void rol_zp_x(uint8_t addr, uint8_t garb) {
  uint8_t msb = getFlagCarry();
  addr = regs.x + addr;
  uint8_t val = readZeroPage(addr);
  setFlagCarry(val >> 7);
  val = msb | (val << 1);
  writeByte(addr, val);
  SZFlags(val);
}

void rol_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  uint8_t msb = getFlagCarry();
  lower = readByte(addr);
  setFlagCarry(lower >> 7);
  lower = msb | (lower << 1);
  writeByte(addr, lower);
  SZFlags(lower);
}

void rol_abs_x(uint8_t lower, uint8_t upper) {
  uint16_t addr = regs.x + (upper << 8) + lower;
  uint8_t msb = getFlagCarry();
  lower = readByte(addr);
  setFlagCarry(lower >> 7);
  lower = msb | (lower << 1);
  writeByte(addr, lower);
  SZFlags(lower);
}

void ror_acc(uint8_t garb0, uint8_t garb1) { 
  uint8_t msb = getFlagCarry() << 7;
  setFlagCarry(regs.a << 7);
  regs.a = msb | (regs.a >> 1);
  SZFlags(regs.a);
}

void ror_zp(uint8_t addr, uint8_t garb) {
  uint8_t msb = getFlagCarry() << 7;
  uint8_t val = readZeroPage(addr);
  setFlagCarry(val << 7);
  val = msb | (val >> 1);
  writeZeroPage(addr, val);
  SZFlags(val);
}

void ror_zp_x(uint8_t addr, uint8_t garb) {
  uint8_t msb = getFlagCarry() << 7;
  addr = regs.x + addr;
  uint8_t val = readZeroPage(addr);
  setFlagCarry(val << 7);
  val = msb | (val >> 1);
  writeZeroPage(addr, val);
  SZFlags(val);
}

void ror_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  uint8_t msb = getFlagCarry() << 7;
  lower = readByte(addr);
  setFlagCarry(lower << 7);
  lower = msb | (lower >> 1);
  writeByte(addr, lower);
  SZFlags(lower);
}

void ror_abs_x(uint8_t lower, uint8_t upper) {
  uint16_t addr = regs.x + (upper << 8) + lower;
  uint8_t msb = getFlagCarry() << 7;
  lower = readByte(addr);
  setFlagCarry(lower << 7);
  lower = msb | (lower >> 1);
  writeByte(addr, lower);
  SZFlags(lower);
}

void rti(uint8_t garb0, uint8_t garb1) {
  regs.p = (popStack() & 0xEF) | 0x20;
  regs.pc = popStack();
  regs.pc += (uint16_t) (popStack() << 8);
}

void rts(uint8_t garb0, uint8_t garb1) {
  uint16_t addr = popStack();
  addr += (uint16_t) (popStack() << 8);
  regs.pc = addr;
}

void sbc_imm(uint8_t val, uint8_t garbage) {
  val = ~val + 1 - (getFlagCarry() ? 0 : 1);
  garbage = val + regs.a;
  VFlag(val, regs.a, garbage);
  regs.a = garbage;
  SZFlags(regs.a);
  (garbage >= 0 && garbage < 0x80) ? setFlagCarry(1) : setFlagCarry(0);
}

void sbc_zp(uint8_t val, uint8_t garbage) {
  val = ~readZeroPage(val) + 1 - (getFlagCarry() ? 0 : 1);
  garbage = val + regs.a;
  VFlag(val, regs.a, garbage);
  regs.a = garbage;
  SZFlags(regs.a);
  (garbage >= 0 && garbage < 0x80) ? setFlagCarry(1) : setFlagCarry(0);
}

void sbc_zp_x(uint8_t val, uint8_t garbage) {
  val = ~readZeroPage(regs.x + val) + 1 - (getFlagCarry() ? 0 : 1);  
  garbage = val + regs.a;
  VFlag(val, regs.a, garbage);
  regs.a = garbage;
  SZFlags(regs.a);
  (garbage >= 0 && garbage < 0x80) ? setFlagCarry(1) : setFlagCarry(0);
}

void sbc_ind_x(uint8_t val, uint8_t garbage) {
  uint16_t addr;
  addr = readZeroPage(val + regs.x) + (readZeroPage(val + regs.x + 1) << 8);
  val = ~readByte(addr) + 1 - (getFlagCarry() ? 0 : 1);
  garbage = val + regs.a;
  VFlag(val, regs.a, garbage);
  regs.a = garbage;
  SZFlags(regs.a);
  (garbage >= 0 && garbage < 0x80) ? setFlagCarry(1) : setFlagCarry(0);
}

void sbc_ind_y(uint8_t val, uint8_t garbage) {
  uint16_t addr;
  addr = readZeroPage(val) + (readZeroPage(val + 1) << 8);
  val = ~readByte(addr + regs.y) + 1 - (getFlagCarry() ? 0 : 1);
  garbage = val + regs.a;
  VFlag(val, regs.a, garbage);
  regs.a = garbage;
  SZFlags(regs.a);
  (garbage >= 0 && garbage < 0x80) ? setFlagCarry(1) : setFlagCarry(0);
}

void sbc_abs(uint8_t lower, uint8_t upper) {
  uint8_t res;
  uint16_t addr;
  addr = (upper << 8) + lower;
  lower = ~readByte(addr) + 1 - (getFlagCarry() ? 0 : 1);
  res = lower + regs.a;
  VFlag(lower, regs.a, res);
  regs.a = res;
  SZFlags(regs.a);
  (res >= 0 && res < 0x80) ? setFlagCarry(1) : setFlagCarry(0);
}

void sbc_abs_x(uint8_t lower, uint8_t upper) {
  uint8_t res;
  uint16_t addr;
  addr = (upper << 8) + lower + regs.x;
  lower = ~readByte(addr) + 1 - (getFlagCarry() ? 0 : 1);
  res = lower + regs.a;
  VFlag(lower, regs.a, res);
  regs.a = res;
  SZFlags(regs.a);
  (res >= 0 && res < 0x80) ? setFlagCarry(1) : setFlagCarry(0);
}


void sbc_abs_y(uint8_t lower, uint8_t upper) {
  uint8_t res;
  uint16_t addr;
  addr = (upper << 8) + lower + regs.y;
  lower = ~readByte(addr) + 1 - (getFlagCarry() ? 0 : 1);
  res = lower + regs.a;
  VFlag(lower, regs.a, res);
  regs.a = res;
  SZFlags(regs.a);
  (res >= 0 && res < 0x80) ? setFlagCarry(1) : setFlagCarry(0);
}


void sta_zp(uint8_t val, uint8_t garbage) {
  writeZeroPage(val, regs.a);
}

void sta_zp_x(uint8_t val, uint8_t garbage) {
  writeZeroPage(val + regs.x, regs.a);
}

void sta_ind_x(uint8_t val, uint8_t garbage) {
  uint16_t addr = readZeroPage(val + regs.x) + (readZeroPage(val + regs.x + 1) << 8);
  writeByte(addr, regs.a);
}

void sta_ind_y(uint8_t val, uint8_t garbage) {
  uint16_t addr = readZeroPage(val) + (readZeroPage(val + 1) << 8);
  writeByte(addr + regs.y, regs.a);
}

void sta_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  writeByte(addr, regs.a);
}

void sta_abs_x(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.x;
  writeByte(addr, regs.a);
}

void sta_abs_y(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.y;
  writeByte(addr, regs.a);
}

void txs(uint8_t garb0, uint8_t garb1) { 
  regs.sp = regs.x;
}

void tsx(uint8_t garb0, uint8_t garb1) {
  regs.x = regs.sp;
  SZFlags(regs.x);
}

void pha(uint8_t garb0, uint8_t garb1) { pushStack(regs.a); }

void pla(uint8_t garb0, uint8_t garb1) { 
  regs.a = popStack();
  SZFlags(regs.a);
}

void php(uint8_t garb0, uint8_t garb1) { pushStack(regs.p | 0x10); }

void plp(uint8_t garb0, uint8_t garb1) { 
  regs.p = (popStack() & 0xEF) | 0x20;
}

void stx_zp(uint8_t val, uint8_t garb) { writeZeroPage(val, regs.x); }

void stx_zp_y(uint8_t val, uint8_t garb) { writeZeroPage(val+regs.y, regs.x); }

void stx_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  writeByte(addr, regs.x);
}

void sty_zp(uint8_t val, uint8_t garb) { writeZeroPage(val, regs.y); }

void sty_zp_x(uint8_t val, uint8_t garb) { writeZeroPage(val+regs.x, regs.y); }

void sty_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  writeByte(addr, regs.y);
}

/*************************************/
/* START OF UNOFFICIAL OPCODE FUNCTIONS*/
/*************************************/

void lax_abs(uint8_t lower, uint8_t upper) {
  lda_abs(lower, upper);
  ldx_abs(lower, upper);
}

void lax_abs_y(uint8_t lower, uint8_t upper) {
  lda_abs_y(lower, upper);
  ldx_abs_y(lower, upper);
}

void lax_zp(uint8_t val, uint8_t garb) {
  lda_zp(val, garb);
  ldx_zp(val, garb);
}

void lax_zp_y(uint8_t val, uint8_t garb) {
  val = readZeroPage(val + regs.y);
  regs.a = val;
  regs.x = val;
  SZFlags(regs.a);
}

void lax_ind_x(uint8_t val, uint8_t garb) {
  
  uint16_t addr = (readZeroPage(val + regs.x + 1) << 8) + readZeroPage(val + regs.x);
  val = readByte(addr);
  regs.a = val;
  regs.x = val;
  SZFlags(regs.a);
}

void lax_ind_y(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(val + 1) << 8) + readZeroPage(val);
  val = readByte(addr + regs.y);
  regs.a = val;
  regs.x = val;
  SZFlags(regs.a);
} 

void sax(uint8_t val, uint8_t garb) {
  val = (regs.a & regs.x) - val;
  setFlagCarry((val > (regs.a & regs.x)) ? 1 : 0);
  SZFlags(val);
}

void axs_abs(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  writeByte(addr, regs.a & regs.x);
}

void axs_zp(uint8_t val, uint8_t garb) {
  writeZeroPage(val, regs.a & regs.x);
}

void axs_zp_y(uint8_t val, uint8_t garb) {
  writeZeroPage(val + regs.y, regs.a & regs.x);
}

void axs_ind_x(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(val + 1 + regs.x) << 8) + readZeroPage(val + regs.x);
  writeByte(addr, regs.a & regs.x);
}

void dcm_abs(uint8_t lower, uint8_t upper) {
  dec_abs(lower, upper);
  cmp_abs(lower, upper);
}

void dcm_abs_x(uint8_t lower, uint8_t upper) {
  dec_abs_x(lower, upper);
  cmp_abs_x(lower, upper);
}

void dcm_abs_y(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  writeByte(addr, readByte(addr + regs.y) - 1);
  cmp_abs_y(lower, upper);
}

void dcm_zp(uint8_t val, uint8_t garb) {
  dec_zp(val, garb);
  cmp_zp(val, garb);
}

void dcm_zp_x(uint8_t val, uint8_t garb) {
  dec_zp_x(val, garb);
  cmp_zp_x(val, garb);
}

void dcm_ind_x(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(val + 1 + regs.x) << 8) + readZeroPage(val + regs.x);
  writeByte(addr, readByte(addr) - 1);
  cmp_ind_x(val, garb);
}

void dcm_ind_y(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(val + 1) << 8) + readZeroPage(val);
  writeByte(addr + regs.y, readByte(addr + regs.y) - 1);
  cmp_ind_y(val, garb);
}

void isb_abs(uint8_t lower, uint8_t upper) {
  uint8_t carry = getFlagCarry();
  inc_abs(lower, upper);
  sbc_abs(lower, upper);
  setFlagCarry(carry);
}

void isb_abs_x(uint8_t lower, uint8_t upper) {
  uint8_t carry = getFlagCarry();
  inc_abs_x(lower, upper);
  sbc_abs_x(lower, upper);
  setFlagCarry(carry);
}

void isb_abs_y(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower;
  uint8_t carry = getFlagCarry();
  writeByte(addr + regs.y, readByte(addr + regs.y) + 1); 
  SZFlags(readByte(addr));
  sbc_abs_y(lower, upper);
  setFlagCarry(carry);
}

void isb_zp(uint8_t val, uint8_t garb) {
  inc_zp(val, garb);
  garb = getFlagCarry();
  sbc_zp(val, garb);
  setFlagCarry(garb);
}

void isb_zp_x(uint8_t val, uint8_t garb) {
  inc_zp_x(val, garb);
  garb = getFlagCarry();
  sbc_zp_x(val, garb);
  setFlagCarry(garb);
}

void isb_ind_x(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(val + 1 + regs.x) << 8) + readZeroPage(val + regs.x);
  writeByte(addr, readByte(addr) + 1);
  SZFlags(readByte(addr));
  garb = getFlagCarry();
  sbc_ind_x(val, garb);
  setFlagCarry(garb);
}

void isb_ind_y(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(val + 1) << 8) + readZeroPage(val);
  writeByte(addr + regs.y, readByte(addr + regs.y) + 1);
  SZFlags(readByte(addr));
  garb = getFlagCarry();
  sbc_ind_y(val, garb);
  setFlagCarry(garb);
}

void slo_abs(uint8_t lower, uint8_t upper) {
  asl_abs(lower, upper);
  ora_abs(lower, upper);
}

void slo_abs_x(uint8_t lower, uint8_t upper) {
  asl_abs_x(lower, upper);
  ora_abs_x(lower, upper);
}

void slo_abs_y(uint8_t lower, uint8_t upper) {
  uint16_t addr = (upper << 8) + lower + regs.y;
  lower = readByte(addr);
  setFlagCarry(getBit(lower, 7));
  writeByte(addr, lower << 1);
  ora_abs_y(lower, upper);
}

void slo_zp(uint8_t val, uint8_t garb) {
  asl_zp(val, garb);
  ora_zp(val, garb);
}

void slo_zp_x(uint8_t val, uint8_t garb) {
  asl_zp_x(val, garb);
  ora_zp_x(val, garb);
}

void slo_ind_x(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(val + regs.x + 1) << 8) + readZeroPage(val + regs.x);
  garb = readByte(addr);
  setFlagCarry(getBit(garb, 7));
  writeByte(addr, (garb << 1));
  ora_ind_x(val, garb);
}

void slo_ind_y(uint8_t val, uint8_t garb) {
  uint16_t addr = (readZeroPage(val + 1) << 8) + readZeroPage(val);
  garb = readByte(addr + regs.y);
  setFlagCarry(getBit(garb, 7));
  writeByte(addr + regs.y, garb << 1);
  ora_ind_y(val, garb);
}

/**
 * Contains all information on CPU opcodes and addressing modes.
 * There are 56 unique opcodes and 13 different addressing modes,
 * which end up yielding a total of 151 unique instructions for 
 * the 6502 processor. The other 105 elements in this array 
 * represent invalid instructions, and will return an error if
 * one is used. All instructions are ordered.
 *
 */
const struct opcode opcodes[256] = {
  {"BRK", IMPLIED, 1},    // 0x00
  {"ORA", INDIRECT_X, 2},
  {"NAN", INVALID, 0},
  {"SLO", INDIRECT_X, 2},
  {"NOP", INVALID, 2},
  {"ORA", ZERO_PAGE, 2},
  {"ASL", ZERO_PAGE, 2},
  {"SLO", ZERO_PAGE, 2},
  {"PHP", IMPLIED, 1},
  {"ORA", IMMEDIATE, 2},
  {"ASL", ACCUMULATOR, 1},
  {"NAN", INVALID, 0},
  {"NOP", INVALID, 3},
  {"ORA", ABSOLUTE, 3},
  {"ASL", ABSOLUTE, 3},
  {"SLO", ABSOLUTE, 3},    // 0x0F
  {"BPL", RELATIVE, 2},
  {"ORA", INDIRECT_Y, 2},
  {"NAN", INVALID, 0},
  {"SLO", INDIRECT_Y, 2},
  {"NOP", INVALID, 2},
  {"ORA", ZERO_PAGE_X, 2},
  {"ASL", ZERO_PAGE_X, 2},
  {"SLO", INDIRECT_X, 2},
  {"CLC", IMPLIED, 1},
  {"ORA", ABSOLUTE_Y, 3},
  {"NOP", INVALID, 1},
  {"SLO", ABSOLUTE_Y, 3},
  {"NOP", INVALID, 3},
  {"ORA", ABSOLUTE_X, 3},
  {"ASL", ABSOLUTE_X, 3},
  {"SLO", ABSOLUTE_X, 3},    // 0x1F
  {"JSR", ABSOLUTE, 3},
  {"AND", INDIRECT_X, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"BIT", ZERO_PAGE, 2},
  {"AND", ZERO_PAGE, 2},
  {"ROL", ZERO_PAGE, 2},
  {"NAN", INVALID, 0},
  {"PLP", IMPLIED, 1},
  {"AND", IMMEDIATE, 2},
  {"ROL", ACCUMULATOR, 1},
  {"NAN", INVALID, 0},
  {"BIT", ABSOLUTE, 3},
  {"AND", ABSOLUTE, 3},
  {"ROL", ABSOLUTE, 3},
  {"NAN", INVALID, 0},   // 0x2F
  {"BMI", RELATIVE, 2},
  {"AND", INDIRECT_Y, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NOP", INVALID, 2},
  {"AND", ZERO_PAGE_X, 2},
  {"ROL", ZERO_PAGE_X, 2},
  {"NAN", INVALID, 0},
  {"SEC", IMPLIED, 1},
  {"AND", ABSOLUTE_Y, 3},
  {"NOP", INVALID, 1},
  {"NAN", INVALID, 0},
  {"NOP", INVALID, 3},
  {"AND", ABSOLUTE_X, 3},
  {"ROL", ABSOLUTE_X, 3},
  {"NAN", INVALID, 0x02, 0},  // 0x3F
  {"RTI", IMPLIED, 1},
  {"EOR", INDIRECT_X, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NOP", INVALID, 2},
  {"EOR", ZERO_PAGE, 2},
  {"LSR", ZERO_PAGE, 2},
  {"NAN", INVALID, 0},
  {"PHA", IMPLIED, 1},
  {"EOR", IMMEDIATE, 2},
  {"LSR", ACCUMULATOR, 1},
  {"NAN", INVALID, 0},
  {"JMP", ABSOLUTE, 3},
  {"EOR", ABSOLUTE, 3},
  {"LSR", ABSOLUTE, 3},
  {"NAN", INVALID, 0},    // 0x4F
  {"BVC", RELATIVE, 2},
  {"EOR", INDIRECT_Y, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NOP", INVALID, 2},
  {"EOR", ZERO_PAGE_X, 2},
  {"LSR", ZERO_PAGE_X, 2},
  {"NAN", INVALID, 0},
  {"CLI", IMPLIED, 1},
  {"EOR", ABSOLUTE_Y, 3},
  {"NOP", INVALID, 1},
  {"NAN", INVALID, 0},
  {"NOP", INVALID, 3},
  {"EOR", ABSOLUTE_X, 3},
  {"LSR", ABSOLUTE_X, 3},
  {"NAN", INVALID, 0},    // 0x5F
  {"RTS", IMPLIED, 1},
  {"ADC", INDIRECT_X, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NOP", INVALID, 2},
  {"ADC", ZERO_PAGE, 2},
  {"ROR", ZERO_PAGE, 2},
  {"NAN", INVALID, 0},
  {"PLA", IMPLIED, 1},
  {"ADC", IMMEDIATE, 2},
  {"ROR", ACCUMULATOR, 1},
  {"NAN", INVALID, 0},
  {"JMP", INDIRECT, 3},
  {"ADC", ABSOLUTE, 3},
  {"ROR", ABSOLUTE, 3},
  {"NAN", INVALID, 0},    // 0x6F
  {"BVS", RELATIVE, 2},
  {"ADC", INDIRECT_Y, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NOP", INVALID, 2},
  {"ADC", ZERO_PAGE_X, 2},
  {"ROR", ZERO_PAGE_X, 2},
  {"NAN", INVALID, 0},
  {"SEI", IMPLIED, 1},
  {"ADC", ABSOLUTE_Y, 3},
  {"NOP", INVALID, 1},
  {"NAN", INVALID, 0},
  {"NOP", INVALID, 3},
  {"ADC", ABSOLUTE_X, 3},
  {"ROR", ABSOLUTE_X, 3},
  {"NAN", INVALID, 0},    // 0x7F
  {"NOP", INVALID, 2},
  {"STA", INDIRECT_X, 2},
  {"NOP", INVALID, 2},
  {"AXS", INDIRECT_Y, 2},
  {"STY", ZERO_PAGE, 2},
  {"STA", ZERO_PAGE, 2},
  {"STX", ZERO_PAGE, 2},
  {"AXS", ZERO_PAGE, 2},
  {"DEY", IMPLIED, 1},
  {"NAN", INVALID, 0},
  {"TXA", IMPLIED, 1},
  {"NAN", INVALID, 0},
  {"STY", ABSOLUTE, 3},
  {"STA", ABSOLUTE, 3},
  {"STX", ABSOLUTE, 3},
  {"AXS", ABSOLUTE, 3},    // 0x8F
  {"BCC", RELATIVE, 2},
  {"STA", INDIRECT_Y, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"STY", ZERO_PAGE_X, 2},
  {"STA", ZERO_PAGE_X, 2},
  {"STX", ZERO_PAGE_X, 2},
  {"AXS", ZERO_PAGE_Y, 2},
  {"TYA", IMPLIED, 1},
  {"STA", ABSOLUTE_Y, 3},
  {"TXS", IMPLIED, 1},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"STA", ABSOLUTE_X, 3},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},  // 0x9F
  {"LDY", IMMEDIATE, 2},
  {"LDA", INDIRECT_X, 2},
  {"LDX", IMMEDIATE, 2},
  {"LAX", INDIRECT_X, 2},
  {"LDY", ZERO_PAGE, 2},
  {"LDA", ZERO_PAGE, 2},
  {"LDX", ZERO_PAGE, 2},
  {"LAX", ZERO_PAGE, 2},
  {"TAY", IMPLIED, 1},
  {"LDA", IMMEDIATE, 2},
  {"TAX", IMPLIED, 1},
  {"NAN", INVALID, 0},
  {"LDY", ABSOLUTE, 3},
  {"LDA", ABSOLUTE, 3},
  {"LDX", ABSOLUTE, 3},
  {"LAX", ABSOLUTE, 3},    // 0xAF
  {"BCS", RELATIVE, 2}, 
  {"LDA", INDIRECT_Y, 2},
  {"NAN", INVALID, 0},
  {"LAX", INDIRECT_Y, 2},
  {"LDY", ZERO_PAGE_X, 2},
  {"LDA", ZERO_PAGE_X, 2},
  {"LDX", ZERO_PAGE_Y, 2},
  {"LAX", ZERO_PAGE_Y, 2},
  {"CLV", IMPLIED, 1},
  {"LDA", ABSOLUTE_Y, 3},
  {"TSX", IMPLIED, 1},
  {"NAN", INVALID, 0},
  {"LDY", ABSOLUTE_X, 3},
  {"LDA", ABSOLUTE_X, 3},
  {"LDX", ABSOLUTE_Y, 3},
  {"LAX", ABSOLUTE_Y, 3},    // 0xBF
  {"CPY", IMMEDIATE, 2},
  {"CMP", INDIRECT_X, 2},
  {"NOP", INVALID, 2},
  {"DCM", INDIRECT_X, 2},
  {"CPY", ZERO_PAGE, 2},
  {"CMP", ZERO_PAGE, 2},
  {"DEC", ZERO_PAGE, 2},
  {"DCM", ZERO_PAGE, 2},
  {"INY", IMPLIED, 1},
  {"CMP", IMMEDIATE, 2},
  {"DEX", IMPLIED, 1},
  {"SAX", IMMEDIATE, 2},
  {"CPY", ABSOLUTE, 3},
  {"CMP", ABSOLUTE, 3},
  {"DEC", ABSOLUTE, 3},
  {"DCM", ABSOLUTE, 3},    // 0xCF     
  {"BNE", RELATIVE, 2},
  {"CMP", INDIRECT_Y, 2},
  {"NAN", INVALID, 0},
  {"DCM", INDIRECT_Y, 2},
  {"NOP", INVALID, 2},
  {"CMP", ZERO_PAGE_X, 2},
  {"DEC", ZERO_PAGE_X, 2},
  {"DCM", ZERO_PAGE_X, 2},
  {"CLD", IMPLIED, 1},
  {"CMP", ABSOLUTE_Y, 3},
  {"NOP", INVALID, 1},
  {"DCM", ABSOLUTE_Y, 3},
  {"NOP", INVALID, 3},
  {"CMP", ABSOLUTE_X, 3},
  {"DEC", ABSOLUTE_X, 3},
  {"DCM", ABSOLUTE_X, 3},      // 0xDF
  {"CPX", IMMEDIATE, 2},
  {"SBC", INDIRECT_X, 2},
  {"NOP", INVALID, 2},
  {"ISB", INDIRECT_X, 2},
  {"CPX", ZERO_PAGE, 2},
  {"SBC", ZERO_PAGE, 2},
  {"INC", ZERO_PAGE, 2},
  {"ISB", ZERO_PAGE, 2},
  {"INX", IMPLIED, 1},
  {"SBC", IMMEDIATE, 2},
  {"NOP", IMPLIED, 1},
  {"SBC", IMMEDIATE, 2},
  {"CPX", ABSOLUTE, 3},
  {"SBC", ABSOLUTE, 3},
  {"INC", ABSOLUTE, 3},
  {"ISB", ABSOLUTE, 3},  // 0xEF
  {"BEQ", RELATIVE, 2},
  {"SBC", INDIRECT_Y, 2},
  {"NAN", INVALID, 0},
  {"ISB", INDIRECT_Y, 2},
  {"NOP", INVALID, 2},
  {"SBC", ZERO_PAGE_X, 2},
  {"INC", ZERO_PAGE_X, 2},
  {"ISB", ZERO_PAGE_X, 2},
  {"SED", IMPLIED, 1},
  {"SBC", ABSOLUTE_Y, 3},
  {"NOP", INVALID, 1},      
  {"ISB", ABSOLUTE_Y, 3},      
  {"NOP", INVALID, 3},      
  {"SBC", ABSOLUTE_X, 3},  
  {"INC", ABSOLUTE_X, 3},  
  {"ISB", ABSOLUTE_X, 3}      // 0xFF
};

FunctionExecute functions[0x100] = {
  brk, ora_ind_x, nan, slo_ind_x, nop, ora_zp, asl_zp, slo_zp,
  php, ora_imm, asl_acc, nan, nop, ora_abs, asl_abs, slo_abs,         // 0x0F
  bpl, ora_ind_y, nan, slo_ind_y, nop, ora_zp_x, asl_zp_x, slo_zp_x,
  clc, ora_abs_y, nop, slo_abs_y, nop, ora_abs_x, asl_abs_x, slo_abs_x,       // 0x1F
  jsr, and_ind_x, nan, nan, bit_zp, and_zp, rol_zp, nan,
  plp, and_imm, rol_acc, nan, bit_abs, and_abs, rol_abs, nan,     // 0x2F
  bmi, and_ind_y, nan, nan, nop, and_zp_x, rol_zp_x, nan,
  sec, and_abs_y, nop, nan, nop, and_abs_x, rol_abs_x, nan,       // 0x3F
  rti, eor_ind_x, nan, nan, nop, eor_zp, lsr_zp, nan,
  pha, eor_imm, lsr_acc, nan, jmp_abs, eor_abs, lsr_abs, nan,     // 0x4F
  bvc, eor_ind_y, nan, nan, nop, eor_zp_x, lsr_zp_x, nan,
  cli, eor_abs_y, nop, nan, nop, eor_abs_x, lsr_abs_x, nan,       // 0x5F
  rts, adc_ind_x, nan, nan, nop, adc_zp, ror_zp, nan,
  pla, adc_imm, ror_acc, nan, jmp_ind, adc_abs, ror_abs, nan,     // 0x6F
  bvs, adc_ind_y, nan, nan, nop, adc_zp_x, ror_zp_x, nan, 
  sei, adc_abs_y, nop, nan, nop, adc_abs_x, ror_abs_x, nan,       // 0x7F
  nop, sta_ind_x, nop, axs_ind_x, sty_zp, sta_zp, stx_zp, axs_zp,
  dey, nan, txa, nan, sty_abs, sta_abs, stx_abs, axs_abs,             // 0x8F
  bcc, sta_ind_y, nan, nan, sty_zp_x, sta_zp_x, stx_zp_y, axs_zp_y,
  tya, sta_abs_y, txs, nan, nan, sta_abs_x, nan, nan,             // 0x9F
  ldy_imm, lda_ind_x, ldx_imm, lax_ind_x, ldy_zp, lda_zp, ldx_zp, lax_zp,
  tay, lda_imm, tax, nan, ldy_abs, lda_abs, ldx_abs, lax_abs,         // 0xAF
  bcs, lda_ind_y, nan, lax_ind_y, ldy_zp_x, lda_zp_x, ldx_zp_y, lax_zp_y,
  clv, lda_abs_y, tsx, nan, ldy_abs_x, lda_abs_x, ldx_abs_y, lax_abs_y, // 0xBF
  cpy_imm, cmp_ind_x, nop, dcm_ind_x, cpy_zp, cmp_zp, dec_zp, dcm_zp,
  iny, cmp_imm, dex, sax, cpy_abs, cmp_abs, dec_abs, dcm_abs,         // 0xCF
  bne, cmp_ind_y, nan, dcm_ind_y, nop, cmp_zp_x, dec_zp_x, dcm_zp_x, 
  cld, cmp_abs_y, nop, dcm_abs_y, nop, cmp_abs_x, dec_abs_x, dcm_abs_x,       // 0xDF
  cpx_imm, sbc_ind_x, nop, isb_ind_x, cpx_zp, sbc_zp, inc_zp, isb_zp, 
  inx, sbc_imm, nop, sbc_imm, cpx_abs, sbc_abs, inc_abs, isb_abs,         // 0xEF
  beq, sbc_ind_y, nan, isb_ind_y, nop, sbc_zp_x, inc_zp_x, isb_zp_x, 
  sed, sbc_abs_y, nop, isb_abs_y, nop, sbc_abs_x, inc_abs_x, isb_abs_x        // 0xFF
};


/**
 * Reads the next instruction from the PRG-ROM
 * and executes it. Increments the stack pointer to
 * the next opcode instruction.
 */
void step(void) {
  uint8_t opcode = readByte(regs.pc);
  uint8_t time = cycles[opcode];
  uint8_t len = opcodes[opcode].operands;
  unsigned char * opName = opcodes[opcode].code;
  uint8_t arg1, arg2;
  arg1 = readByte(regs.pc + 1);
  arg2 = readByte(regs.pc + 2);
  if (logger) {
    fprintf(logFile, "%x, %x %x %x %s  A:%x X:%x Y:%x P:%x SP:%x\n",
      regs.pc, opcode, arg1, arg2, opName, regs.a, regs.x, regs.y, regs.p, regs.sp); 
  }
  functions[opcode](arg1, arg2);
  regs.pc += (strcmp(opName, "JSR") != 0 &&
              strcmp(opName, "JMP") != 0 &&
              strcmp(opName, "RTI") != 0) ? len : 0;
}

