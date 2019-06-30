#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ppu.h"//delete this
#include "cpu.h"
#include "memory.h"
#include "memoryMappedIO.h"
#include "registers.h"
#include "main.h"

#define KB 1024

extern NameTable nTable0;

extern struct registers regs;
extern uint8_t prg_rom_lower[0x4000];
extern uint8_t prg_rom_upper[0x4000];

uint32_t cycle = 7;

void nan(void);

uint8_t interrupted = 0;

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

void NMInterruptHandler() {
  pushStack(regs.pc >> 8);
  pushStack(regs.pc);
  pushStack(regs.p);
  regs.pc = (readByte(0xFFFB) << 8) + readByte(0xFFFA);
}

void IRQHandler() {
  pushStack(regs.pc >> 8);
  pushStack(regs.pc);
  pushStack(regs.p);
  regs.pc = (readByte(0xFFFF) << 8) + readByte(0xFFFE);
  interrupted = 0;
  setFlagBreak(0);  // may not be kosher
}

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
  cycle += 1;
  if (!getBit(val, 7)) {
    regs.pc += val;
  } else {
    val = ~val + 1;
    regs.pc -= val;
  }
}

uint8_t fetchArgument(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val;
  switch (mode) {
    case IMMEDIATE:
      val = arg1;
      break;
    case ZERO_PAGE:
      val = readZeroPage(arg1);
      break;
    case ZERO_PAGE_X:
      val = readZeroPage(arg1 + regs.x);
      break;
    case ZERO_PAGE_Y:
      val = readZeroPage(arg1 + regs.y);
      break;
    case INDIRECT_X:
      {
      uint16_t addr = readZeroPage(arg1 + regs.x) + (readZeroPage(arg1 + regs.x + 1) << 8);
      val = readByte(addr);
      break;
      }
    case INDIRECT_Y:
      {
      uint16_t addr = readZeroPage(arg1) + (readZeroPage(arg1 + 1) << 8);
      addr += regs.y;
      val = readByte(addr);
      break;
      }
    case ABSOLUTE:
      {
      uint16_t addr = (arg2 << 8) + arg1;
      val = readByte(addr);
      break;
      }
    case ABSOLUTE_X:
      {
      uint16_t addr = (arg2 << 8) + arg1 + regs.x;
      val = readByte(addr);
      break;
      }
    case ABSOLUTE_Y:
      {
      uint16_t addr = (arg2 << 8) + arg1 + regs.y;
      val = readByte(addr);
      break;
      }
    default:
      printf("Invalid addressing mode: %X Terminating.\n", mode);
      exit(1);
      break;
  }
  return val;
}

void dataWriteBack(uint8_t val, uint8_t mode, uint8_t arg1, uint8_t arg2) {
  switch (mode) {
    case ZERO_PAGE:
      writeZeroPage(arg1, val);
      break;
    case ZERO_PAGE_X:
      writeZeroPage(arg1 + regs.x, val);
      break;
    case ZERO_PAGE_Y:
      writeZeroPage(arg1 + regs.y, val);
      break;
    case INDIRECT_X:
      {
      uint16_t addr = readZeroPage(arg1 + regs.x) + (readZeroPage(arg1 + regs.x + 1) << 8);
      writeByte(addr, val);
      break;
      }
    case INDIRECT_Y:
      {
      uint16_t addr = readZeroPage(arg1) + (readZeroPage(arg1 + 1) << 8);
      addr += regs.y;
      writeByte(addr, val);
      break;
      }
    case ABSOLUTE:
      {
      uint16_t addr = arg1 + (arg2 << 8);
      writeByte(addr, val);
      break;
      }
    case ABSOLUTE_X:
      {
      uint16_t addr = arg1 + (arg2 << 8) + regs.x;
      writeByte(addr, val);
      break;
      }
    case ABSOLUTE_Y:
      {
      uint16_t addr = arg1 + (arg2 << 8) + regs.y;
      writeByte(addr, val);
      break;
      }
  }
}

/*************************************/
/* START OF OFFICIAL OPCODE FUNCTIONS*/
/*************************************/

void nan(void) {
  printf("Error: Invalid opcode!.\n");
  exit(1);
}

void brk(void) { setFlagBreak(1); }

/**
 * @brief performs add with carry operation on accumulator register.
 * @note carry bit comes from carry flag of status register.
 * @modes: IMM, ZP, ZP_X, IND_X, IND_Y, ABS, ABS_X, ABS_Y
 * @param arg1, arg2: first and second instruction operands.
 *        If an ADC instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void adc(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val, res;
  val = fetchArgument(mode, arg1, arg2);
  res = val + regs.a + getFlagCarry();
  VFlag(val, regs.a, res);
  res < regs.a ? setFlagCarry(1) : setFlagCarry(0);
  regs.a = res;
  SZFlags(regs.a);
}


/**
 * @brief performs and operation with accumulator register.
 * @modes: IMM, ZP, ZP_X, IND_X, IND_Y, ABS, ABS_X, ABS_Y
 * @param arg1, arg2: first and second instruction operands.
 *        If an AND instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void and(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val, res;
  val = fetchArgument(mode, arg1, arg2);
  val &= regs.a;
  SZFlags(val);
  regs.a = val;
}



/**
 * @brief performs arithmetic shift left operation in memory.
 * @modes: ACC, ZP, ZP_X, ABS, ABS_X
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no first or second arguments,
 *        arg1 or arg2 is disregarded, respectively.
 * @param mode: addressing mode of instruction
 */
void asl(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val, res;
  if (mode == ACCUMULATOR) {
    setFlagCarry(getBit(regs.a, 7));
    regs.a <<= 1;
    val = regs.a;
  } else {
    val = fetchArgument(mode, arg1, arg2);
    setFlagCarry(getBit(val, 7));
    val <<= 1;
    dataWriteBack(val, mode, arg1, arg2);
  }
  SZFlags(val);
}


/**
 * @brief performs bit test on byte in memory.
 * @modes: ZP, ABS
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void bit(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val;
  val = fetchArgument(mode, arg1, arg2);
  setFlagNegative(getBit(val, 7));
  setFlagOverflow(getBit(val, 6));
  setFlagZero((val & regs.a) != 0 ? 0 : 1);
}

void bpl(AddressMode unused, uint8_t val) {  
  if (!getFlagNegative()) {
    branchJump(val);
  }
}

void bmi(AddressMode unused, uint8_t val) {  
  if (getFlagNegative()) {
    branchJump(val);
  }
}

void bvc(AddressMode unused, uint8_t val) {  
  if (!getFlagOverflow()) {
    branchJump(val);
  }
}

void bvs(AddressMode unused, uint8_t val) {  
  if (getFlagOverflow()) {
    branchJump(val);
  }
}

void bcc(AddressMode unused, uint8_t val) {  
  if (!getFlagCarry()) {
    branchJump(val);
  }
}

void bcs(AddressMode unused, uint8_t val) {  
  if (getFlagCarry()) {
    branchJump(val);
  }
}

void bne(AddressMode unused, uint8_t val) {  
  if (!getFlagZero()) {
    branchJump(val);
  }
}

void beq(AddressMode unused, uint8_t val) {  
  if (getFlagZero()) {
    branchJump(val);
  }
}


/**
 * @brief performs a compare of accumulator with an addressable argument
 * @modes IMM, ZP, ZP_X, IND_X, IND_Y, ABS, ABS_X, ABS_Y
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void cmp(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val, res;
  val = fetchArgument(mode, arg1, arg2);
  flagCompare(regs.a, val);
}


/**
 * @brief performs a compare of X register with an addressable argument
 * @modes IMM, ZP, ABS
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void cpx(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val, res;
  val = fetchArgument(mode, arg1, arg2);
  flagCompare(regs.x, val);
}


/**
 * @brief performs a compare of Y register with an addressable argument
 * @modes IMM, ZP, ABS
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void cpy(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val, res;
  val = fetchArgument(mode, arg1, arg2);
  flagCompare(regs.y, val);
}


/**
 * @brief performs a decrement of byte in memory addressed by argument
 * @modes ZP, ZP_X, ABS, ABS_X
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void dec(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val, res;
  val = fetchArgument(mode, arg1, arg2) - 1;
  dataWriteBack(val, mode, arg1, arg2);
  SZFlags(val);
}

/**
 * @brief performs an exclusive or into accumulator by an addressable argument
 * @modes IMM, ZP, ZP_X, IND_X, IND_Y, ABS, ABS_X, ABS_Y
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void eor(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val, res;
  val = fetchArgument(mode, arg1, arg2);
  regs.a = regs.a ^ val;
  SZFlags(regs.a);
}

/**
 * @brief performs an increment of byte in memory addressed by argument
 * @modes ZP, ZP_X, ABS, ABS_X
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void inc(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val;
  val = fetchArgument(mode, arg1, arg2) + 1;
  dataWriteBack(val, mode, arg1, arg2);
  SZFlags(val);
}

/**
 * @brief performs a jump to an addressable argument address
 * @modes ABS, IND
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void jmp(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint16_t addr = (arg2 << 8) + arg1;
  if (mode == INDIRECT) { 
    addr = readByte(addr) + (readByte(addr + 
      (addr % 0x100 == 0xFF ? -0xFF : 1)) << 8);
  }
  regs.pc = addr;
}


/**
 * @brief performs load into X register by an addressable argument
 * @modes IMM, ZP, ZP_X, ABS, ABS_X
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void ldx(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  regs.x = fetchArgument(mode, arg1, arg2);
  SZFlags(regs.x);
}

/**
 * @brief performs load into Y register by an addressable argument
 * @modes IMM, ZP, ABS
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void ldy(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  regs.y = fetchArgument(mode, arg1, arg2);
  SZFlags(regs.y);
}

/**
 * @brief performs load into accumulator by an addressable argument
 * @modes IMM, ZP, ZP_X, IND_X, IND_Y, ABS, ABS_X, ABS_Y
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void lda(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  regs.a = fetchArgument(mode, arg1, arg2);
  SZFlags(regs.a);

}

/**
 * @brief performs a logical shift right of an addressable argument in memory
 * @modes ACC, ZP, ZP_X, ABS, ABS_X
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void lsr(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val;
  if (mode == ACCUMULATOR) {
    setFlagCarry(getBit(regs.a, 0));
    regs.a >>= 1;
    val = regs.a;
  } else {
    val = fetchArgument(mode, arg1, arg2);
    setFlagCarry(getBit(val, 0));
    val >>= 1;
    dataWriteBack(val, mode, arg1, arg2); 
  }
  SZFlags(val);
}

void nop(void) { return; }

/**
 * @brief performs an or operation into accumulator by an addressable argument
 * @modes IMM, ZP, ZP_X, IND_X, IND_Y, ABS, ABS_X, ABS_Y
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void ora(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val;
  val = fetchArgument(mode, arg1, arg2);
  val |= regs.a;
  SZFlags(val);
  regs.a = val;
}

/**
 * @brief performs a rotate left with an addressable argument
 * @modes ACC, ZP, ZP_X, ABS, ABS_X
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void rol(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val, res;
  uint8_t lsb = getFlagCarry();
  if (mode == ACCUMULATOR) {
    setFlagCarry(regs.a >> 7);
    res = (regs.a << 1) | lsb;
    regs.a = res;
  } else {
    val = fetchArgument(mode, arg1, arg2);
    setFlagCarry(val >> 7);
    res = (val << 1) | lsb;
    dataWriteBack(res, mode, arg1, arg2);
  }
  SZFlags(res);
}

/**
 * @brief performs a rotate right with an addressable argument
 * @modes ACC, ZP, ZP_X, ABS, ABS_X
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void ror(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val, res;
  uint8_t msb = getFlagCarry() << 7;
  if (mode == ACCUMULATOR) {
    setFlagCarry(regs.a << 7);
    res = msb | (regs.a >> 1);
    regs.a = res;
  } else  {
    val = fetchArgument(mode, arg1, arg2);
    setFlagCarry(val << 7);
    res = msb | (val >> 1);
    dataWriteBack(res, mode, arg1, arg2);
  }
  SZFlags(res);
}

/**
 * @brief performs a subtract with carry of accumulator with argument(s)
 * @modes IMM, ZP, ZP_X, IND_X, IND_Y, ABS, ABS_X, ABS_Y
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void sbc(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t val, res;
  val = fetchArgument(mode, arg1, arg2);
  val = ~val + 1 - (getFlagCarry() ? 0 : 1);
  res = val + regs.a;
  VFlag(val, regs.a, res);
  regs.a = res;
  SZFlags(regs.a);
  (res >= 0 && res < 0x80) ? setFlagCarry(1) : setFlagCarry(0);
}


/**
 * @brief performs a store of accumulator into addressable argument
 * @modes ZP, ZP_X, IND_X, IND_Y, ABS, ABS_X, ABS_Y
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void sta(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  dataWriteBack(regs.a, mode, arg1, arg2);
}


/**
 * @brief performs a store of X register into addressable argument
 * @modes ZP, ZP_Y, ABS
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void stx(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  dataWriteBack(regs.x, mode, arg1, arg2);
}


/**
 * @brief performs a store of Y register into addressable argument
 * @modes ZP, ZP_X, ABS
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void sty(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  dataWriteBack(regs.y, mode, arg1, arg2);
}


void clc(void) { setFlagCarry(0); }

void sec(void) { setFlagCarry(1); }

void cli(void) { setFlagInterrupt(0); }

void sei(void) { setFlagInterrupt(1); }

void clv(void) { setFlagOverflow(0); }

void cld(void) { setFlagDecimal(0); }

void sed(void) { setFlagDecimal(1); }

void jsr(AddressMode unused, uint8_t lower, uint8_t upper) {
  uint16_t val = regs.pc + 3 - 1;
  pushStack(val >> 8);
  pushStack(val & 0x00FF);
  val = (upper << 8) + lower;
  regs.pc = val;
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
  SZFlags(regs.x);
}

void inx(void) {
  regs.x++;
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
  SZFlags(regs.y);
}

void iny(void) { 
  regs.y++;
  SZFlags(regs.y);
}

void rti(void) {
  regs.p = (popStack() & 0xEF) | 0x20;
  regs.pc = popStack();
  regs.pc |= (popStack() << 8);
  interrupted = 0;
}

void rts(void) {
  uint16_t addr = popStack();
  addr += (uint16_t) (popStack() << 8);
  regs.pc = addr;
}

void txs(void) { 
  regs.sp = regs.x;
}

void tsx(void) {
  regs.x = regs.sp;
  SZFlags(regs.x);
}

void pha(void) { pushStack(regs.a); }

void pla(void) { 
  regs.a = popStack();
  SZFlags(regs.a);
}

void php(void) { pushStack(regs.p | 0x10); }

void plp(void) { 
  regs.p = (popStack() & 0xEF) | 0x20;
}

/*************************************/
/* START OF UNOFFICIAL OPCODE FUNCTIONS*/
/*************************************/


/**
 * @brief performs a load into accumulator and X register
 * @modes ZP, ZP_Y, IND_X, IND_Y, ABS, ABS_Y
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void lax(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  lda(mode, arg1, arg2);
  ldx(mode, arg1, arg2);
}


void sax(AddressMode unused, uint8_t val) {
  val = (regs.a & regs.x) - val;
  setFlagCarry((val > (regs.a & regs.x)) ? 1 : 0);
  SZFlags(val);
}


/**
 * @brief performs a store into memory of accumulator AND X register
 * @modes ZP, ZP_Y, IND_X, ABS
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void axs(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  dataWriteBack(regs.a & regs.x, mode, arg1, arg2);
}



/**
 * @brief performs a decrement followed by a compare instruction
 * @modes ZP, ZP_X, IND_X, IND_Y, ABS, ABS_X, ABS_Y
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void dcm(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  dec(mode, arg1, arg2);
  cmp(mode, arg1, arg2);
}

/**
 * @brief performs an increment followed by a subtract with carry
 * @modes ZP, ZP_X, IND_X, IND_Y, ABS, ABS_X, ABS_Y
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void isb(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  uint8_t carry = getFlagCarry();
  inc(mode, arg1, arg2);
  sbc(mode, arg1, arg2);
  setFlagCarry(carry);
}


/**
 * @brief performs an arithmetic shift left followed by an or with accumulator
 * @modes ZP, ZP_X, IND_X, IND_Y, ABS, ABS_X, ABS_Y
 * @param arg1, arg2: first and second instruction operands.
 *        If instruction has no second argument,
 *        arg2 is disregarded.
 * @param mode: addressing mode of instruction
 */
void slo(AddressMode mode, uint8_t arg1, uint8_t arg2) {
  asl(mode, arg1, arg2);
  ora(mode, arg1, arg2);
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
  {"SAX", INDIRECT_X, 2},
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
  {"STX", ZERO_PAGE_Y, 2},
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
  brk, ora, nan, slo, nop, ora, asl, slo,
  php, ora, asl, nan, nop, ora, asl, slo,  // 0x0F
  bpl, ora, nan, slo, nop, ora, asl, slo,
  clc, ora, nop, slo, nop, ora, asl, slo,  // 0x1F
  jsr, and, nan, nan, bit, and, rol, nan,
  plp, and, rol, nan, bit, and, rol, nan,  // 0x2F
  bmi, and, nan, nan, nop, and, rol, nan,
  sec, and, nop, nan, nop, and, rol, nan,  // 0x3F
  rti, eor, nan, nan, nop, eor, lsr, nan,
  pha, eor, lsr, nan, jmp, eor, lsr, nan,  // 0x4F
  bvc, eor, nan, nan, nop, eor, lsr, nan,
  cli, eor, nop, nan, nop, eor, lsr, nan,  // 0x5F
  rts, adc, nan, nan, nop, adc, ror, nan,
  pla, adc, ror, nan, jmp, adc, ror, nan,  // 0x6F
  bvs, adc, nan, nan, nop, adc, ror, nan, 
  sei, adc, nop, nan, nop, adc, ror, nan,  // 0x7F
  nop, sta, nop, sax, sty, sta, stx, sax,
  dey, nan, txa, nan, sty, sta, stx, sax,  // 0x8F
  bcc, sta, nan, nan, sty, sta, stx, sax,
  tya, sta, txs, nan, nan, sta, nan, nan,  // 0x9F
  ldy, lda, ldx, lax, ldy, lda, ldx, lax,
  tay, lda, tax, nan, ldy, lda, ldx, lax,  // 0xAF
  bcs, lda, nan, lax, ldy, lda, ldx, lax,
  clv, lda, tsx, nan, ldy, lda, ldx, lax,  // 0xBF
  cpy, cmp, nop, dcm, cpy, cmp, dec, dcm,
  iny, cmp, dex, sax, cpy, cmp, dec, dcm,  // 0xCF
  bne, cmp, nan, dcm, nop, cmp, dec, dcm, 
  cld, cmp, nop, dcm, nop, cmp, dec, dcm,  // 0xDF
  cpx, sbc, nop, isb, cpx, sbc, inc, isb, 
  inx, sbc, nop, sbc, cpx, sbc, inc, isb,  // 0xEF
  beq, sbc, nan, isb, nop, sbc, inc, isb, 
  sed, sbc, nop, isb, nop, sbc, inc, isb   // 0xFF
};


/**
 * Updates the cycle counter of the CPU.
 *
 * @param len: length of the most recently executed cycle
 * @param opcode: byte instruction that was last executed.
 */
void updateCycle(uint16_t addr, uint8_t offset) {
  if (addr & 0x00FF < offset) {
    cycle++;
  }
    //switch (opcode) {
      //case 0x7D:  // ADC
      //case 0x79:
      //case 0x71:
      //case 0x3D:  // AND
      //case 0x39:
      //case 0x31:
      //case 0xDD:  // CMP
      //case 0xD9:
      //case 0xD1:
      //case 0x5D:  // EOR
      //case 0x59:
      //case 0x51:
      //case 0xBD:  // LDA
      //case 0xB9:
      //case 0xB1:
      //case 0xBE:  // LDX
      //case 0xBC:  // LDY
      //case 0x1D:  // ORA
      //case 0x19:
      //case 0x11:
      //case 0xFD:  // SBC
      //case 0xF9:
      //case 0xF1:
        //cycle++;
        //break;
  //}
}


/**
 * Reads the next instruction from the PRG-ROM
 * and executes it. Increments the stack pointer to
 * the next opcode instruction.
 */
void step(void) {
  uint8_t opcode = readByte(regs.pc);
  uint8_t time = cycles[opcode];
  uint8_t len = opcodes[opcode].operands;
  unsigned char * opname = opcodes[opcode].code;
  AddressMode mode = opcodes[opcode].addrMode;
  uint8_t arg1, arg2;
  arg1 = readByte(regs.pc + 1);
  arg2 = readByte(regs.pc + 2);
  if (logger) {
    fprintf(logFile, "%x, %x %x %x %s  A:%x X:%x Y:%x P:%x SP:%x CYCLE:%d\n",
      regs.pc, opcode, arg1, arg2, opname, regs.a, regs.x, regs.y, regs.p, regs.sp, cycle); 
  }
  if (len == 1) {
    functions[opcode].FunctionEx_1Arg(mode);
  } else if (len == 2) {
    functions[opcode].FunctionEx_2Arg(mode, arg1);
  } else if (len == 3) {
    functions[opcode].FunctionEx_3Arg(mode, arg1, arg2);
  } else { 
    functions[opcode].FunctionEx_0Arg();
  }
  cycle += time;
  regs.pc += (strcmp(opname, "JSR") != 0 &&
              strcmp(opname, "JMP") != 0 &&
              strcmp(opname, "RTI") != 0) ? len : 0;
  if (getVerticalBlankStart()) {
    if (getNMIGeneration() && !interrupted) {
      interrupted = 1;
      NMInterruptHandler();
    }
  }
  if (getFlagBreak()) {
    if (interrupted == 1) {
      IRQHandler();
    } else if (interrupted == 0) {
      interrupted = 7;
    } else interrupted--;
  }
}
