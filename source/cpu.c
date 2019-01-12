#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "memory.h"
#include "registers.h"

#define KB 1024
#define ZERO_PAGE 0
#define ZERO_PAGE_X 1
#define ZERO_PAGE_Y 2
#define ABSOLUTE 3
#define ABSOLUTE_X 4
#define ABSOLUTE_Y 5
#define INDIRECT 6
#define INDIRECT_X 7
#define INDIRECT_Y 8
#define IMPLIED 9
#define ACCUMULATOR 10
#define IMMEDIATE 11
#define RELATIVE 12
#define INVALID -1

extern struct registers regs;
extern unsigned char prg_rom_lower[0x4000];
extern unsigned char prg_rom_upper[0x4000];

typedef void (*FunctionCallBack)(unsigned char);

//FunctionCallBack opFunctions[] = { &foo, &bar };

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
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"ORA", ZERO_PAGE, 2},
  {"ASL", ZERO_PAGE, 2},
  {"NAN", INVALID, 0},
  {"PHP", IMPLIED, 1},
  {"ORA", IMMEDIATE, 2},
  {"ASL", ACCUMULATOR, 1},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"ORA", ABSOLUTE, 3},
  {"ASL", ABSOLUTE, 3},
  {"NAN", INVALID, 0},    // 0x0F
  {"BPL", RELATIVE, 2},
  {"ORA", INDIRECT_Y, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"ORA", ZERO_PAGE_X, 1},
  {"ASL", ZERO_PAGE_X, 2},
  {"NAN", INVALID, 0},
  {"CLC", IMPLIED, 1},
  {"ORA", ABSOLUTE_Y, 3},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"ORA", ABSOLUTE_X, 3},
  {"ASL", ABSOLUTE_X, 3},
  {"NAN", INVALID, 0},    // 0x1F
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
  {"NAN", INVALID, 0},
  {"AND", ZERO_PAGE_X, 2},
  {"ROL", ZERO_PAGE_X, 2},
  {"NAN", INVALID, 0},
  {"SEC", IMPLIED, 1},
  {"AND", ABSOLUTE_Y, 3},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"AND", ABSOLUTE_X, 3},
  {"ROL", ABSOLUTE_X, 3},
  {"NAN", INVALID, 0},  // 0x3F
  {"RTI", IMPLIED, 1},
  {"EOR", INDIRECT_X, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
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
  {"NAN", INVALID, 0},
  {"EOR", ZERO_PAGE_X, 2},
  {"LSR", ZERO_PAGE_X, 2},
  {"NAN", INVALID, 0},
  {"CLI", IMPLIED, 1},
  {"EOR", ABSOLUTE_Y, 3},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"EOR", ABSOLUTE_X, 3},
  {"LSR", ABSOLUTE_X, 3},
  {"NAN", INVALID, 0},    // 0x5F
  {"RTS", IMPLIED, 1},
  {"ADC", INDIRECT_X, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
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
  {"NAN", INVALID, 0},
  {"ADC", ZERO_PAGE_X, 2},
  {"ROR", ZERO_PAGE_X, 2},
  {"NAN", INVALID, 0},
  {"SEI", IMPLIED, 1},
  {"ADC", ABSOLUTE_Y, 3},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"ADC", ABSOLUTE_X, 3},
  {"ROR", ABSOLUTE_X, 3},
  {"NAN", INVALID, 0},    // 0x7F
  {"NAN", INVALID, 0},
  {"STA", INDIRECT_X, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"STY", ZERO_PAGE, 2},
  {"STA", ZERO_PAGE, 2},
  {"STX", ZERO_PAGE, 2},
  {"NAN", INVALID, 0},
  {"DEY", IMPLIED, 1},
  {"NAN", INVALID, 0},
  {"TXA", IMPLIED, 1},
  {"NAN", INVALID, 0},
  {"STY", ABSOLUTE, 3},
  {"STA", ABSOLUTE, 3},
  {"STX", ABSOLUTE, 3},
  {"NAN", INVALID, 0},    // 0x8F
  {"BCC", RELATIVE, 2},
  {"STA", INDIRECT_Y, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"STY", ZERO_PAGE_X, 2},
  {"STA", ZERO_PAGE_X, 2},
  {"STX", ZERO_PAGE_X, 2},
  {"NAN", INVALID, 0},
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
  {"NAN", INVALID, 0},
  {"LDY", ZERO_PAGE, 2},
  {"LDA", ZERO_PAGE, 2},
  {"LDX", ZERO_PAGE, 2},
  {"NAN", INVALID, 0},
  {"TAY", IMPLIED, 1},
  {"LDA", IMMEDIATE, 2},
  {"TAX", IMPLIED, 1},
  {"NAN", INVALID, 0},
  {"LDY", ABSOLUTE, 3},
  {"LDA", ABSOLUTE, 3},
  {"LDX", ABSOLUTE, 3},
  {"NAN", INVALID, 0},    // 0xAF
  {"BCS", RELATIVE, 2}, 
  {"LDA", INDIRECT_Y, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"LDY", ZERO_PAGE_X, 2},
  {"LDA", ZERO_PAGE_X, 2},
  {"LDX", ZERO_PAGE_Y, 2},
  {"NAN", INVALID, 0},
  {"CLV", IMPLIED, 1},
  {"LDA", ABSOLUTE_Y, 3},
  {"TSX", IMPLIED, 1},
  {"NAN", INVALID, 0},
  {"LDY", ABSOLUTE_X, 3},
  {"LDA", ABSOLUTE_X, 3},
  {"LDX", ABSOLUTE_Y, 3},
  {"NAN", INVALID, 0},    // 0xBF
  {"CPY", IMMEDIATE, 2},
  {"CMP", INDIRECT_X, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"CPY", ZERO_PAGE, 2},
  {"CMP", ZERO_PAGE, 2},
  {"DEC", ZERO_PAGE, 2},
  {"NAN", INVALID, 0},
  {"INY", IMPLIED, 1},
  {"CMP", IMMEDIATE, 2},
  {"DEX", IMPLIED, 1},
  {"NAN", INVALID, 0},
  {"CPY", ABSOLUTE, 3},
  {"CMP", ABSOLUTE, 3},
  {"DEC", ABSOLUTE, 3},
  {"NAN", INVALID, 0},    // 0xCF     
  {"BNE", RELATIVE, 2},
  {"CMP", INDIRECT_Y, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"CMP", ZERO_PAGE_X, 2},
  {"DEC", ZERO_PAGE_X, 2},
  {"NAN", INVALID, 0},
  {"CLD", IMPLIED, 1},
  {"CMP", ABSOLUTE_Y, 3},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"CMP", ABSOLUTE_X, 3},
  {"DEC", ABSOLUTE_X, 3},
  {"NAN", INVALID, 0},      // 0xDF
  {"CPX", IMMEDIATE, 2},
  {"SBC", INDIRECT_X, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"CPX", ZERO_PAGE, 2},
  {"SBC", ZERO_PAGE, 2},
  {"INC", ZERO_PAGE, 2},
  {"NAN", INVALID, 0},
  {"INX", IMPLIED, 1},
  {"SBC", IMMEDIATE, 2},
  {"NOP", IMPLIED, 1},
  {"NAN", INVALID, 0},
  {"CPX", ABSOLUTE, 3},
  {"SBC", ABSOLUTE, 3},
  {"INC", ABSOLUTE, 3},
  {"NAN", INVALID, 0},  // 0xEF
  {"BEQ", RELATIVE, 2},
  {"SBC", INDIRECT_Y, 2},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"NAN", INVALID, 0},
  {"SBC", ZERO_PAGE_X, 2},
  {"INC", ZERO_PAGE_X, 2},
  {"NAN", INVALID, 0},
  {"SED", IMPLIED, 1},
  {"SBC", ABSOLUTE_Y, 3},
  {"NAN", INVALID, 0},      
  {"NAN", INVALID, 0},      
  {"NAN", INVALID, 0},      
  {"SBC", ABSOLUTE_X, 3},  
  {"INC", ABSOLUTE_X, 3},  
  {"NAN", INVALID, 0}      // 0xFF
};


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

const unsigned char cycles[256] = {
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
 * Reads the next instruction from the PRG-ROM
 * and executes it. Increments the stack pointer to
 * the next opcode instruction.
 */
void step(void) {
  unsigned char opcode = regs.pc > 0x3FFF ? prg_rom_lower[regs.pc] : prg_rom_upper[regs.pc- 0x4000];
  unsigned char time = cycles[opcode];
  regs.pc += opcodes[opcode].operands;

}

/**
 * The next set of functions will either set or clear a flag
 * in the status register. For each function:
 *
 * @param bit: 0 to clear a flag, anything else to set a flag.
 *
 * @returns: Nothing.
 */

void setFlagCarry(unsigned char bit)  {
  regs.p = bit ? regs.p | 0b00000001 : regs.p & 0b11111110;
}
void setFlagZero(unsigned char bit) { 
  regs.p = bit ? regs.p | 0b00000010 : regs.p & 0b11111101; 
}

void setFlagInterrupt(unsigned char bit) {
  regs.p = bit ? regs.p | 0b00000100 : regs.p & 0b11111011; 
}

void setFlagBreak(unsigned char bit) { 
  regs.p = bit ? regs.p | 0b00010000 : regs.p & 0b11101111;
}

void setFlagOverflow(unsigned char bit) { 
  regs.p = bit ? regs.p | 0b01000000 : regs.p & 0b10111111;
}

void setFlagNegative(unsigned char bit) {
  regs.p = bit ? regs.p | 0b10000000 : regs.p & 0b01111111;
}


/**
 * The next set of functions will retrieve a flag
 * from the status register. For each function:
 *
 * @returns: Flag value. Returns 1 for set or 0 for clear.
 */

unsigned char getFlagCarry(void) { return (regs.p << 7) >> 7; }

unsigned char getFlagZero(void) { return (regs.p << 6) >> 7; }

unsigned char getFlagInterrupt(void) { return (regs.p << 5) >> 7; }

unsigned char getFlagBreak(void) { return (regs.p << 3) >> 7; }

unsigned char getFlagOverflow(void) { return (regs.p << 2) >> 7; }

unsigned char getFlagNegative(void) { return (regs.p << 1) >> 7; }


/**
 * Retrieves a selected bit from a given byte.
 *
 * @param byte: Byte in which a bit is retrieved from.
 * @param bit: Bit (7...0) that is retrieved from the byte.
 *
 * @returns: 1 or 0, if the selected bit is set or not.
 */
unsigned char getBit(unsigned char byte, unsigned char bit) {
  return (byte & (1 << bit)) >> bit;
}

/**
 * Determines if the overflag should be set after an instruction.
 *
 * @param a: First argument to the instruction.
 * @param b: Second argument to the instruction. 
 * @param c: Resolution to the instruction.
 */
void VFlag(unsigned char a, unsigned char b, unsigned char c) {
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
void SZFlags(unsigned char val) {
  if (val == 0) setFlagZero(1);
  else if (val >> 7) setFlagNegative(1);
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
void branchJump(unsigned char val) {
  if (getBit(7, val) == 0) {
    regs.pc += val;
  } else {
    val = ~val + 1;
    regs.pc -= val;
  }
}


/****************************/
/* START OF OPCODE FUNCTIONS*/
/****************************/

void brk(unsigned char garb0, unsigned char garb1) { setFlagBreak(1); }  //0x00

void adc_imm(unsigned char val, unsigned char res) {
  val += getFlagCarry();
  res = val + regs.a;
  VFlag(val, regs.a, res);
  res > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a);
}

void adc_zp(unsigned char val, unsigned char res) {
  val = readZeroPage(val) + getFlagCarry();
  res = val + regs.a;
  VFlag(val, regs.a, res);
  res > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a);
}

void adc_zp_x(unsigned char val, unsigned char res) {
  val = readZeroPage(val + regs.x) + getFlagCarry();
  res = val + regs.a;
  VFlag(val, regs.a, res);
  res > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a);
}

void adc_ind_x(unsigned char val, unsigned char res) {
  unsigned short addr = readZeroPage(val + regs.x) + (readZeroPage(val + regs.x + 1) << 8);
  val = readByte(addr) + getFlagCarry();
  res = val + regs.a;
  VFlag(val, regs.a, res);
  res > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a);
}

void adc_ind_y(unsigned char val, unsigned char res) {
  unsigned short addr = readZeroPage(val) + (readZeroPage(val + 1) << 8);
  val = readByte(addr + regs.y) + getFlagCarry();
  res = val + regs.a;
  VFlag(val, regs.a, res);
  res > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a);
}

void adc_abs(unsigned char lower, unsigned char upper) {
  unsigned char res;
  unsigned short addr = (upper << 8) + lower;
  lower = readByte(addr) + getFlagCarry();
  res = lower + regs.a;
  VFlag(lower, regs.a, res);
  res > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a); 
}

void adc_abs_x(unsigned char lower, unsigned char upper) {
  unsigned char res;
  unsigned short addr = (upper << 8) + lower + regs.x;
  lower = readByte(addr) + getFlagCarry();
  res = lower + regs.a;
  VFlag(lower, regs.a, res);
  res > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a); 
}

void adc_abs_y(unsigned char lower, unsigned char upper) {
  unsigned char res;
  unsigned short addr = (upper << 8) + lower + regs.y;
  lower = readByte(addr) + getFlagCarry();
  res = lower + regs.a;
  VFlag(lower, regs.a, res);
  res > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a); 
}

void and_ind_x(unsigned char val, unsigned char garb) {
  unsigned short addr = (readZeroPage(regs.x + val + 1) << 8) + readZeroPage(regs.x + val); 
  val = regs.a & readByte(addr);
  SZFlags(val);
  regs.a = val;
};

void and_ind_y(unsigned char val, unsigned char garb) {
  unsigned short addr = (readZeroPage(val + 1) << 8) + readZeroPage(val); 
  val = regs.a & readByte(addr + regs.y);
  SZFlags(val);
  regs.a = val;
}

void and_imm(unsigned char val, unsigned char garb) {   //0x09
  val = regs.a & val;
  SZFlags(val);
  regs.a = val;
}

void and_zp(unsigned char val, unsigned char garb) {  //0x05
  val = readZeroPage(val) & regs.a;
  SZFlags(val);
  regs.a = val;
}

void and_zp_x(unsigned char val, unsigned char garb) {  // 0x15
  val = readZeroPage(val+regs.x) & regs.a;
  SZFlags(val);
  regs.a = val;
}

void and_abs(unsigned char lower, unsigned char upper) {  
  unsigned char addr = (upper << 8) + lower;
  lower = readByte(addr) & regs.a;
  SZFlags(lower);
  regs.a = lower;
}

void and_abs_x(unsigned char lower, unsigned char upper) {
  unsigned char addr = (upper << 8) + lower + regs.x;
  lower = readByte(addr) & regs.a;
  SZFlags(lower);
  regs.a = lower;
}

void and_abs_y(unsigned char lower, unsigned char upper) {
  unsigned char addr = (upper << 8) + lower + regs.y;
  lower = readByte(addr) & regs.a;
  SZFlags(lower);
  regs.a = lower;
}

void asl_acc(unsigned char garb0, unsigned char garb1) { 
  setFlagCarry(getBit(regs.a, 7));
  regs.a = regs.a << 1;
  SZFlags(regs.a);
}

void asl_zp(unsigned char addr, unsigned char val) {
  val = readZeroPage(addr);
  setFlagCarry(getBit(val, 7));
  val = val << 1;
  writeZeroPage(addr, val); 
  SZFlags(val);
}

void asl_zp_x(unsigned char addr, unsigned char val) {
  addr += regs.x;
  val = readZeroPage(addr);
  setFlagCarry(getBit(val, 7));
  val = val << 1;
  writeZeroPage(addr, val);
  SZFlags(val);
}

void asl_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (lower << 8) + upper;
  lower = readByte(addr);
  setFlagCarry(getBit(lower, 7));
  lower = lower << 1;
  writeByte(addr, lower);
}

void asl_abs_x(unsigned char lower, unsigned char upper) {
  unsigned short addr = (lower << 8) + upper + regs.x;
  lower = readByte(addr);
  setFlagCarry(getBit(lower, 7));
  lower = lower << 1;
  writeByte(addr, lower);
}

void bit_zp(unsigned char val, unsigned char garb) {
  val = readZeroPage(val);
  setFlagNegative(getBit(val, 7));
  setFlagOverflow(getBit(val, 6));
  setFlagZero(val & regs.a);
}

void bit_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower;
  lower = readByte(addr);
  setFlagNegative(getBit(lower, 7));
  setFlagOverflow(getBit(lower, 6));
  setFlagZero(lower & regs.a);
}

void bpl(unsigned char val, unsigned char garb) {  
  if (!getFlagNegative()) {
    branchJump(val);
  }
}

void bmi(unsigned char val, unsigned char garb) {  
  if (getFlagNegative()) {
    branchJump(val);
  }
}

void bvc(unsigned char val, unsigned char garb) {  
  if (!getFlagOverflow()) {
    branchJump(val);
  }
}

void bvs(unsigned char val, unsigned char garb) {  
  if (getFlagOverflow()) {
    branchJump(val);
  }
}

void bcc(unsigned char val, unsigned char garb) {  
  if (!getFlagCarry()) {
    branchJump(val);
  }
}

void bcs(unsigned char val, unsigned char garb) {  
  if (getFlagCarry()) {
    branchJump(val);
  }
}

void bne(unsigned char val, unsigned char garb) {  
  if (!getFlagZero()) {
    branchJump(val);
  }
}

void beq(unsigned char val, unsigned char garb) {  
  if (getFlagZero()) {
    branchJump(val);
  }
}

void cmp_imm(unsigned char val, unsigned char garb) {
  if (regs.a > val) {
    setFlagCarry(1);
  } else if (regs.a == val) {
    setFlagZero(1);
  } else setFlagNegative(1);
}

void cmp_zp(unsigned char val, unsigned char garb) {
  val = readZeroPage(val);
  if (regs.a > val) {
    setFlagCarry(1);
  } else if (regs.a == val) {
    setFlagZero(1);
  } else setFlagNegative(1);
}

void cmp_zp_x(unsigned char val, unsigned char garb) {
  val = readZeroPage(val + regs.x);
  if (regs.a > val) {
    setFlagCarry(1);
  } else if (regs.a == val) {
    setFlagZero(1);
  } else setFlagNegative(1);
}

void cmp_ind_x(unsigned char val, unsigned char garb) {
  unsigned short addr;
  addr = (readZeroPage(val + regs.x + 1) << 8) + readZeroPage(val + regs.x);
  val = readByte(addr);
  if (regs.a > val) {
    setFlagCarry(1);
  } else if (regs.a == val) {
    setFlagZero(1);
  } else setFlagNegative(1);
}

void cmp_ind_y(unsigned char val, unsigned char garb) {
  unsigned short addr;
  addr = (readZeroPage(val + 1) << 8) + readZeroPage(val);
  val = readByte(addr + regs.y);
  if (regs.a > val) {
    setFlagCarry(1);
  } else if (regs.a == val) {
    setFlagZero(1);
  } else setFlagNegative(1);
}

void cmp_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower;
  lower = readByte(addr);
  if (regs.a > lower) {
    setFlagCarry(1);
  } else if (regs.a == lower) {
    setFlagZero(1);
  } else setFlagNegative(1);
}

void cmp_abs_x(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower + regs.x;
  lower = readByte(addr);
  if (regs.a > lower) {
    setFlagCarry(1);
  } else if (regs.a == lower) {
    setFlagZero(1);
  } else setFlagNegative(1);
}

void cmp_abs_y(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower + regs.y;
  lower = readByte(addr);
  if (regs.a > lower) {
    setFlagCarry(1);
  } else if (regs.a == lower) {
    setFlagZero(1);
  } else setFlagNegative(1);
}

void cpx_imm(unsigned char val, unsigned char garb) {
  if (regs.x > val) {
    setFlagCarry(1);
  } else if (regs.x == val) {
    setFlagZero(1);
  } else setFlagNegative(1);
}

void cpx_zp(unsigned char val, unsigned char garb) {
  val = readZeroPage(val);
  if (regs.x > val) {
    setFlagCarry(1);
  } else if (regs.x == val) {
    setFlagZero(1);
  } else setFlagNegative(1);
}

void cpx_abs(unsigned char lower, unsigned char upper) {
  unsigned char addr = (upper << 8) + lower;
  lower = readByte(addr);
  if (regs.x > lower) {
    setFlagCarry(1);
  } else if (regs.x == lower) {
    setFlagZero(1);
  } else setFlagNegative(1);
}

void cpy_imm(unsigned char val, unsigned char garb) {
  if (regs.y > val) {
    setFlagCarry(1);
  } else if (regs.y == val) {
    setFlagZero(1);
  } else setFlagNegative(1);
}

void cpy_zp(unsigned char val, unsigned char garb) {
  val = readZeroPage(val);
  if (regs.y > val) {
    setFlagCarry(1);
  } else if (regs.y == val) {
    setFlagZero(1);
  } else setFlagNegative(1);
}

void cpy_abs(unsigned char lower, unsigned char upper) {
  unsigned char addr = (upper << 8) + lower;
  lower = readByte(addr);
  if (regs.y > lower) {
    setFlagCarry(1);
  } else if (regs.y == lower) {
    setFlagZero(1);
  } else setFlagNegative(1);
}



void dec_zp(unsigned char addr, unsigned char garb) {
  regs.pc -= readZeroPage(addr);
  SZFlags(regs.pc);
}

void dec_zp_x(unsigned char addr, unsigned char garb) {
  regs.pc -= readZeroPage(regs.x + addr);
  SZFlags(regs.pc);
}

void dec_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower;
  regs.pc -= readByte(addr);
  SZFlags(regs.pc);
}

void dec_abs_x(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower + regs.x;
  regs.pc -= readByte(addr);
  SZFlags(regs.pc);
}

void eor_imm(unsigned char val, unsigned char garb) {
  regs.a = regs.a ^ val;
  SZFlags(regs.a);
}

void eor_zp(unsigned char val, unsigned char garb) {
  regs.a = regs.a ^ readZeroPage(val);
  SZFlags(regs.a);
}

void eor_zp_x(unsigned char val, unsigned char garb) {
  regs.a = regs.a ^ readZeroPage(val + regs.x);
  SZFlags(regs.a);
}

void eor_ind_x(unsigned char val, unsigned char garb) {
  unsigned short addr;
  addr = (readZeroPage(val + regs.x + 1) << 8) + readZeroPage(val + regs.x);
  regs.a = regs.a ^ readByte(addr);
  SZFlags(regs.a);
}

void eor_ind_y(unsigned char val, unsigned char garb) {
  unsigned short addr = (readZeroPage(val + 1) << 8) + readZeroPage(val);
  regs.a = regs.a ^ readByte(addr + regs.y);
  SZFlags(regs.a);
}

void eor_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower;
  regs.a = regs.a ^ readByte(addr);
  SZFlags(regs.a);
}

void eor_abs_x(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower + regs.x;
  regs.a = regs.a ^ readByte(addr);
  SZFlags(regs.a);
}

void eor_abs_y(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower + regs.y;
  regs.a = regs.a ^ readByte(addr);
  SZFlags(regs.a);
}

void clc(unsigned char garb0, unsigned char garb1) { setFlagCarry(0); }

void sec(unsigned char garb0, unsigned char garb1) { setFlagCarry(1); }

void cli(unsigned char garb0, unsigned char garb1) { setFlagInterrupt(0); }

void sei(unsigned char garb0, unsigned char garb1) { setFlagInterrupt(1); }

void clv(unsigned char garb0, unsigned char garb1) { setFlagOverflow(0); }

void cld(unsigned char garb0, unsigned char garb1) { return; }

void sed(unsigned char garb0, unsigned char garb1) { return; }

void inc_zp(unsigned char addr, unsigned char garb) {
  regs.pc += readZeroPage(addr);
  SZFlags(regs.pc);
}

void inc_zp_x(unsigned char addr, unsigned char garb) {
  regs.pc += readZeroPage(regs.x + addr);
  SZFlags(regs.pc);
}

void inc_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower;
  regs.pc += readByte(addr);
  SZFlags(regs.pc);
}

void inc_abs_x(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower + regs.x;
  regs.pc += readByte(addr);
  SZFlags(regs.pc);
}

void jmp_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower;
  regs.pc = readByte(addr);
}

void jmp_ind(unsigned char val, unsigned char garb) {
  unsigned short addr = (readZeroPage(val+1) << 8) + readZeroPage(val);
  regs.pc = readByte(addr);
}

void jsr(unsigned char lower, unsigned char upper) {
  unsigned short toStack = (upper << 8) + lower - 1;
  pushStack(toStack);
}

void ldx_imm(unsigned char addr, unsigned char garb) {
  regs.x = addr;
  SZFlags(regs.x);
}

void ldx_zp(unsigned char addr, unsigned char garb) {
  regs.x = readZeroPage(addr);
  SZFlags(regs.x);
}

void ldx_zp_y(unsigned char addr, unsigned char garb) {
  regs.x = readZeroPage(addr + regs.y);
  SZFlags(regs.x);
}

void ldx_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower;
  regs.x = readByte(addr);
  SZFlags(regs.x);
}

void ldx_abs_y(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower + regs.y;
  regs.x = readByte(addr);
  SZFlags(regs.x);
}



void ldy_imm(unsigned char addr, unsigned char garb) {
  regs.y = addr;
  SZFlags(regs.y);
}

void ldy_zp(unsigned char addr, unsigned char garb) {
  regs.y = readZeroPage(addr);
  SZFlags(regs.y);
}

void ldy_zp_x(unsigned char addr, unsigned char garb) {
  regs.y = readZeroPage(addr + regs.x);
  SZFlags(regs.y);
}

void ldy_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower;
  regs.y = readByte(addr);
  SZFlags(regs.y);
}

void ldy_abs_x(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower + regs.x;
  regs.y = readByte(addr);
  SZFlags(regs.y);
}

void lsr_acc(unsigned char garb0, unsigned char garb1) { 
  setFlagCarry(getBit(regs.a, 0));
  regs.a = regs.a >> 1;
  SZFlags(regs.a);
}

void lsr_zp(unsigned char addr, unsigned char garb) {
  unsigned char val = readZeroPage(addr);
  setFlagCarry(getBit(val, 0));
  val = val >> 1;
  writeZeroPage(addr, val); 
  SZFlags(val);
}

void lsr_zp_x(unsigned char addr, unsigned char garb) {
  unsigned char val = readZeroPage(addr+regs.x);
  setFlagCarry(getBit(val, 0));
  val = val >> 1;
  writeZeroPage(addr, val);
  SZFlags(val);
}

void lsr_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower;
  lower = readByte(addr);
  setFlagCarry(getBit(lower, 0));
  lower = lower >> 1;
  writeByte(addr, lower);
}

void lsr_abs_x(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower + regs.x;
  lower = readByte(addr);
  setFlagCarry(getBit(lower, 0));
  lower = lower >> 1;
  writeByte(addr, lower);
}

void nop(unsigned char garb0, unsigned char garb1) { return; }

void ora_ind_x(unsigned char val, unsigned char garb) {
  unsigned short addr = (readZeroPage(regs.x + val + 1) << 8) + readZeroPage(regs.x + val); 
  val = regs.a | readByte(addr);
  SZFlags(val);
  regs.a = val;
};

void ora_ind_y(unsigned char val, unsigned char garb) {
  unsigned short addr = (readZeroPage(val + 1) << 8) + readZeroPage(val); 
  val = regs.a | readByte(addr + regs.y);
  SZFlags(val);
  regs.a = val;
}

void ora_imm(unsigned char val, unsigned char garb) {   //0x09
  val = regs.a | val;
  SZFlags(val);
  regs.a = val;
}

void ora_zp(unsigned char val, unsigned char garb) {  //0x05
  val = readZeroPage(val) | regs.a;
  SZFlags(val);
  regs.a = val;
}

void ora_zp_x(unsigned char val, unsigned char garb) {  // 0x15
  val = readZeroPage(val+regs.x) | regs.a;
  SZFlags(val);
  regs.a = val;
}

void ora_abs(unsigned char lower, unsigned char upper) {  
  unsigned char addr = (upper << 8) + lower;
  lower = readByte(addr) | regs.a;
  SZFlags(lower);
  regs.a = lower;
}

void ora_abs_x(unsigned char lower, unsigned char upper) {
  unsigned char addr = (upper << 8) + lower + regs.x;
  lower = readByte(addr) | regs.a;
  SZFlags(lower);
  regs.a = lower;
}

void ora_abs_y(unsigned char lower, unsigned char upper) {
  unsigned char addr = (upper << 8) + lower + regs.y;
  lower = readByte(addr) | regs.a;
  SZFlags(lower);
  regs.a = lower;
}

void tax(unsigned char garb0, unsigned char garb1) {
  regs.x = regs.a;
  SZFlags(regs.x);  
}

void txa(unsigned char garb0, unsigned char garb1) {
  regs.a = regs.x;
  SZFlags(regs.a);
}

void dex(unsigned char garb0, unsigned char garb1) {
  regs.x--;
  !~regs.x ? setFlagCarry(1) : setFlagCarry(0);
  SZFlags(regs.x);
}

void inx(unsigned char garb0, unsigned char garb1) {
  regs.x++;
  !regs.x ? setFlagCarry(1) : setFlagCarry(0);
  SZFlags(regs.x);
}

void tay(unsigned char garb0, unsigned char garb1) {
  regs.y = regs.a;
  SZFlags(regs.y);
}

void tya(unsigned char garb0, unsigned char garb1) { 
  regs.a = regs.y;
  SZFlags(regs.a);
}

void dey(unsigned char garb0, unsigned char garb1) { 
  regs.y--;
  !~regs.y ? setFlagCarry(1) : setFlagCarry(0);
  SZFlags(regs.y);
}

void iny(unsigned char garb0, unsigned char garb1) { 
  regs.y++;
  !regs.y ? setFlagCarry(1) : setFlagCarry(0);
  SZFlags(regs.y);
}

void rol_acc(unsigned char garb0, unsigned char garb1) { 
  unsigned char msb = getFlagCarry();
  setFlagCarry(regs.a >> 7);
  regs.a = msb | (regs.a << 1);
  SZFlags(regs.a);
}

void rol_zp(unsigned char addr, unsigned char garb) {
  unsigned char msb = getFlagCarry();
  unsigned char val = readZeroPage(addr);
  setFlagCarry(val >> 7);
  val = msb | (val << 1);
  writeByte(addr, val);
  SZFlags(val);
}

void rol_zp_x(unsigned char addr, unsigned char garb) {
  unsigned char msb = getFlagCarry();
  addr = regs.x + addr;
  unsigned char val = readZeroPage(addr);
  setFlagCarry(val >> 7);
  val = msb | (val << 1);
  writeByte(addr, val);
  SZFlags(val);
}

void rol_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower;
  unsigned char msb = getFlagCarry();
  lower = readByte(addr);
  setFlagCarry(lower >> 7);
  lower = msb | (lower << 1);
  writeByte(addr, lower);
  SZFlags(lower);
}

void rol_abs_x(unsigned char lower, unsigned char upper) {
  unsigned short addr = regs.x + (upper << 8) + lower;
  unsigned char msb = getFlagCarry();
  lower = readByte(addr);
  setFlagCarry(lower >> 7);
  lower = msb | (lower << 1);
  writeByte(addr, lower);
  SZFlags(lower);
}

void ror_acc(unsigned char garb0, unsigned char garb1) { 
  unsigned char msb = getFlagCarry() << 7;
  setFlagCarry(regs.a << 7);
  regs.a = msb | (regs.a >> 1);
  SZFlags(regs.a);
}

void ror_zp(unsigned char addr, unsigned char garb) {
  unsigned char msb = getFlagCarry() << 7;
  unsigned char val = readZeroPage(addr);
  setFlagCarry(val << 7);
  val = msb | (val >> 1);
  writeZeroPage(addr, val);
  SZFlags(val);
}

void ror_zp_x(unsigned char addr, unsigned char garb) {
  unsigned char msb = getFlagCarry() << 7;
  addr = regs.x + addr;
  unsigned char val = readZeroPage(addr);
  setFlagCarry(val << 7);
  val = msb | (val >> 1);
  writeZeroPage(addr, val);
  SZFlags(val);
}

void ror_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower;
  unsigned char msb = getFlagCarry() << 7;
  lower = readByte(addr);
  setFlagCarry(lower << 7);
  lower = msb | (lower >> 1);
  writeByte(addr, lower);
  SZFlags(lower);
}

void ror_abs_x(unsigned char lower, unsigned char upper) {
  unsigned short addr = regs.x + (upper << 8) + lower;
  unsigned char msb = getFlagCarry() << 7;
  lower = readByte(addr);
  setFlagCarry(lower << 7);
  lower = msb | (lower >> 1);
  writeByte(addr, lower);
  SZFlags(lower);
}

void rti(unsigned char garb0, unsigned char garb1) {
  regs.p = popStack();
  regs.pc = (unsigned short) popStack();
  regs.pc = (regs.pc << 8) + (unsigned short) popStack();
}

void rts(unsigned char garb0, unsigned char garb1) {
  unsigned short addr = (unsigned short) popStack();
  addr += ((unsigned short) popStack()) << 8;
  regs.pc = addr + 1;
}

void sbc_imm(unsigned char val, unsigned char garbage) {
  val = ~val + 1 + getFlagCarry();
  garbage = val + regs.a;
  VFlag(val, regs.a, garbage);
  garbage > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = garbage;
  SZFlags(regs.a);
}

void sbc_zp(unsigned char val, unsigned char garbage) {
  val = ~readZeroPage(val) + 1 + getFlagCarry();
  garbage = val + regs.a;
  VFlag(val, regs.a, garbage);
  garbage > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = garbage;
  SZFlags(regs.a);
}

void sbc_zp_x(unsigned char val, unsigned char garbage) {
  val = ~readZeroPage(regs.x + val) + 1 + getFlagCarry();  
  garbage = val + regs.a;
  VFlag(val, regs.a, garbage);
  garbage > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = garbage;
  SZFlags(regs.a);
}

void sbc_ind_x(unsigned char val, unsigned char garbage) {
  unsigned short addr;
  addr = readZeroPage(val + regs.x) + (readZeroPage(val + regs.x + 1) << 8);
  val = ~readByte(addr) + 1 + getFlagCarry();
  garbage = val + regs.a;
  VFlag(val, regs.a, garbage);
  garbage > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = garbage;
  SZFlags(regs.a);
}

void sbc_ind_y(unsigned char val, unsigned char garbage) {
  unsigned short addr;
  addr = readZeroPage(val) + (readZeroPage(val + 1) << 8);
  val = ~readByte(addr + regs.y) + 1 + getFlagCarry();
  garbage = val + regs.a;
  VFlag(val, regs.a, garbage);
  garbage > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = garbage;
  SZFlags(regs.a);
}

void sbc_abs(unsigned char lower, unsigned char upper) {
  unsigned char res;
  unsigned short addr;
  addr = (upper << 8) + lower;
  lower = ~readByte(addr) + 1 + getFlagCarry();
  res = lower + regs.a;
  VFlag(lower, regs.a, res);
  res > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a);
}

void sbc_abs_x(unsigned char lower, unsigned char upper) {
  unsigned char res;
  unsigned short addr;
  addr = (upper << 8) + lower + regs.x;
  lower = ~readByte(addr) + 1 + getFlagCarry();
  res = lower + regs.a;
  VFlag(lower, regs.a, res);
  res > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a);
}


void sbc_abs_y(unsigned char lower, unsigned char upper) {
  unsigned char res;
  unsigned short addr;
  addr = (upper << 8) + lower + regs.y;
  lower = ~readByte(addr) + 1 + getFlagCarry();
  res = lower + regs.a;
  VFlag(lower, regs.a, res);
  res > regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a);
}


void sta_zp(unsigned char val, unsigned char garbage) {
  regs.a = readZeroPage(val);
}

void sta_zp_x(unsigned char val, unsigned char garbage) {
  regs.a = readZeroPage(val + regs.x);
}

void sta_ind_x(unsigned char val, unsigned char garbage) {
  unsigned short addr = readZeroPage(val + regs.x) + (readZeroPage(val + regs.x + 1) << 8);
  regs.a = readByte(addr);
}

void sta_ind_y(unsigned char val, unsigned char garbage) {
  unsigned short addr = readZeroPage(val) + (readZeroPage(val + 1) << 8);
  regs.a = readByte(addr + regs.y);
}

void sta_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower;
  regs.a = readByte(addr);
}

void sta_abs_x(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower + regs.x;
  regs.a = readByte(addr);
}

void sta_abs_y(unsigned char lower, unsigned char upper) {
  unsigned short addr = (upper << 8) + lower + regs.y;
  regs.a = readByte(addr);
}

void txs(unsigned char garb0, unsigned char garb1) { regs.sp = regs.x; }

void tsx(unsigned char garb0, unsigned char garb1) { regs.x = regs.sp; }

void pha(unsigned char garb0, unsigned char garb1) { pushStack(regs.a); }

void pla(unsigned char garb0, unsigned char garb1) { regs.a = popStack(); }

void php(unsigned char garb0, unsigned char garb1) { pushStack(regs.p); }

void plp(unsigned char garb0, unsigned char garb1) { regs.p = popStack(); }

void stx_zp(unsigned char val, unsigned char garb) { regs.x = readZeroPage(val); }

void stx_zp_y(unsigned char val, unsigned char garb) { regs.x = readZeroPage(val+regs.y); }

void stx_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (readByte(upper)) << 8 + readByte(lower);
  regs.x = readByte(addr);
}

void sty_zp(unsigned char val, unsigned char garb) { regs.y = readZeroPage(val); }

void sty_zp_x(unsigned char val, unsigned char garb) { regs.y = readZeroPage(val+regs.x); }

void sty_abs(unsigned char lower, unsigned char upper) {
  unsigned short addr = (readByte(upper) << 8) + readByte(lower);
  regs.y = readByte(addr);
}


