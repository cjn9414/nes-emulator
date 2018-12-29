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

// instruction name, address mode, address, length in bytes
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
  {"NAN", INVALID, 0x02, 0},  // 0x3F
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

unsigned char getFlagCarry(void) { return (regs.p << 7) >> 7; }

unsigned char getFlagZero(void) { return (regs.p << 6) >> 7; }

unsigned char getFlagInterrupt(void) { return (regs.p << 5) >> 7; }

unsigned char getFlagBreak(void) { return (regs.p << 3) >> 7; }

unsigned char getFlagOverflow(void) { return (regs.p << 2) >> 7; }

unsigned char getFlagNegative(void) { return (regs.p << 1) >> 7; }

void SZFlags(unsigned char val) {
  if (val == 0) setFlagZero(1);
  else if (val >> 7) setFlagNegative(1);
  else {
    setFlagZero(0);
    setFlagNegative(0);
  }
}

void brk(void) { setFlagBreak(1); }  //0x00

unsigned char ora_ind_x(unsigned char val) {     //0x01
  unsigned char res = regs.a | (regs.x + val)%0b100000000;
  val = readByte((readByte(res+1) << 8) + readByte(res)); // um
  SZFlags(val);
  return val;
};

unsigned char ora_ind_y(unsigned char val) {     //0x11
  unsigned short res = (readByte(val+1) << 8) + readByte(val);
  val = readByte(res + regs.y) | regs.a;
  SZFlags(val);
  return val;
}

unsigned char ora_imm(unsigned char val) {   //0x09
  val = regs.a | val;
  SZFlags(val);
  return val;
}

unsigned char ora_zp(unsigned char val) {  //0x05
  val = readByte(val) | regs.a;
  SZFlags(val);
  return val;
}

unsigned char ora_zp_x(unsigned char val) {  // 0x15
  val = readByte(val+regs.x) | regs.a;
  SZFlags(val);
  return val;
}

unsigned char ora_abs(unsigned char lower, unsigned char upper) {  //0x0D
  unsigned char val = readByte((readByte(upper) << 8) + readByte(lower)) | regs.a;
  SZFlags(val);
  return val;
}

unsigned char ora_abs_x(unsigned char lower, unsigned char upper) {  //0x1D
  unsigned char val = (regs.x + readByte(readByte(upper) << 8) + readByte(lower)) | regs.a;
  SZFlags(val);
  return val;
}

unsigned char ora_abs_y(unsigned char lower, unsigned char upper) {  //0x19
  unsigned char val = (regs.y + readByte(readByte(upper) << 8) + readByte(lower)) | regs.a;
  SZFlags(val);
  return val;
}

void tax(void) {
  regs.x = regs.a;
  SZFlags(regs.x);  
}

void txa(void) {
  regs.a = regs.x;
  SZFlags(regs.a);
}

void dex(void) {
  regs.x--;
  !~regs.x ? setFlagCarry(1) : setFlagCarry(0);
  SZFlags(regs.x);
}

void inx(void) {
  regs.x++;
  !regs.x ? setFlagCarry(1) : setFlagCarry(0);
  SZFlags(regs.x);
}

void tay(void) {
  regs.y = regs.a;
  SZFlags(regs.y);
}

void tya(void) { 
  regs.a = regs.y;
  SZFlags(regs.a);
}

void dey(void) { 
  regs.y--;
  !~regs.y ? setFlagCarry(1) : setFlagCarry(0);
  SZFlags(regs.y);
}

void iny(void) { 
  regs.y++;
  !regs.y ? setFlagCarry(1) : setFlagCarry(0);
  SZFlags(regs.y);
}

void rol_acc(void) { 
  unsigned char msb = getFlagCarry();
  setFlagCarry(regs.a >> 7);
  regs.a = msb | (regs.a << 1);
  SZFlags(regs.a);
}

void rol_zp(unsigned char addr) {
  unsigned char msb = getFlagCarry();
  unsigned char val = readByte(addr);
  setFlagCarry(val >> 7);
  val = msb | (val << 1);
  writeByte(addr, val);
  SZFlags(val);
}

void rol_zp_x(unsigned char addr) {
  unsigned char msb = getFlagCarry();
  addr = ((regs.x + addr) % 0b100000000);
  unsigned char val = readByte(addr);
  setFlagCarry(val >> 7);
  val = msb | (val << 1);
  writeByte(addr, val);
  SZFlags(val);
}

void rol_abs(unsigned char val) {
  unsigned short addr = (readByte(val+1) << 8) + readByte(val);
  unsigned char msb = getFlagCarry();
  val = readByte(addr);
  setFlagCarry(val >> 7);
  val = msb | (val << 1);
  writeByte(addr,  val);
  SZFlags(val);
}

void rol_abs_x(unsigned char val) {
  unsigned short addr = regs.x + (readByte(val+1) << 8) + readByte(val);
  unsigned char msb = getFlagCarry();
  val = readByte(addr);
  setFlagCarry(val >> 7);
  val = msb | (val << 1);
  writeByte(addr, val);
  SZFlags(val);
}

void ror_acc(void) { 
  unsigned char msb = getFlagCarry() << 7;
  setFlagCarry(regs.a << 7);
  regs.a = msb | (regs.a >> 1);
  SZFlags(regs.a);
}

void ror_zp(unsigned char addr) {
  unsigned char msb = getFlagCarry() << 7;
  unsigned char val = readByte(addr);
  setFlagCarry(val << 7);
  val = msb | (val >> 1);
  writeByte(addr, val);
  SZFlags(val);
}

void ror_zp_x(unsigned char addr) {
  unsigned char msb = getFlagCarry() << 7;
  addr = ((regs.x + addr) % 0b100000000);
  unsigned char val = readByte(addr);
  setFlagCarry(val << 7);
  val = msb | (val >> 1);
  writeByte(addr, val);
  SZFlags(val);
}

void ror_abs(unsigned char val) {
  unsigned short addr = (readByte(val+1) << 8) + readByte(val);
  unsigned char msb = getFlagCarry() << 7;
  val = readByte(addr);
  setFlagCarry(val << 7);
  val = msb | (val >> 1);
  writeByte(addr,  val);
  SZFlags(val);
}

void ror_abs_x(unsigned char val) {
  unsigned short addr = regs.x + (readByte(val+1) << 8) + readByte(val);
  unsigned char msb = getFlagCarry() << 7;
  val = readByte(addr);
  setFlagCarry(val << 7);
  val = msb | (val >> 1);
  writeByte(addr, val);
  SZFlags(val);
}


