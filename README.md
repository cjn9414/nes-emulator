# nes-emulator
My attempt at making an emulator for the Nintendo Entertainment System.

## Motives
I really want to learn much more about low-level programming, specifically programming in C. Upon searching for project ideas, I came across a [gameboy emulator](https://cturt.github.io/cinoop.html) that really caught my eye. As a huge fan of the Nintendo Entertainment System, I thought I would try my hand at making an emulator for it.

#NES Overview
## Hardware
* 8-bit 2A03 processor for the CPU, developed by NMOS technology
  * Very similar to 8-bit 6502, but is equipped to handle sound, but lacking BCD ability.
  * Little endian processor - addresses are stored in memory least significant byte first
  * Supports 64 KB of memory ($0000-$FFFF)
    * Zero Page - ($0000-$00FF) - used by certain addressing modes to allow for quicker execution
    * $0000-$07FF mirrored at $0800-$0FFF, $1000-$17FF, and $1800-$1FFF
    * Memory mapped I/O registers located at $2000-$401F. Locations $2000-$2007 are mirrored every eight bytes, from $2008-$3FFF, with the remaining registers following this mirroring.
    * Additional I/O registers located at $4000-$401F
    * Expansion ROM from  $4020-$5FFF
    * SRAM from $6000-$7FFF
    * $8000-$FFFF are the addresses allocated to cartridge PRG-ROM.
      * Games with one 16 KB bank - Load bank into both $8000 and $C000
      * Games with two 16 KB banks - Load one into $8000 and the other into $C000
      * Games with more than two 16 KB banks - Memory mappers used to determine banks that are loaded
  * Three special purpose registers
    * The program counter
      * 16-bit register to hold address of the next instruction to be executed
    * The stack pointer
      * The stack is located at $0100-$01FF
      * Stack pointer is an 8-bit register at an offset from $0100
      * Stack pointer decremented when byte pushed onto stack, incremented when popped.
      * No detection of Stack Overflow (wraps from $00-$FF)
    * The status register
      * Carry Flag (C) - Set if last instruction caused overflow or underflow. Can be set using SEC (Set Carry Flag) instruction or cleared using CLC (Clear Carry Flag) instruction
      * Zero Flag (Z) - Set if the result of the last instruction resolved to 0
      * Interrupt Disable (I) - When set, prevents response to interrupt requests (IRQs). Set by SEI (Set Interrupt Disable) and cleared by CLI (Clear Interrupt Disable)
      * Decimal Mode (D) - Switches into BCD, but ignored in the 2A03 because it is not supported
      * Break Command (B) - Indicates execution of a BRK (break) instruction, which causes an IRQ
      * Overflow Flag (V) - Set if an invalid two's compliment result was obtained
      * Negative Flag (N) - Set if the sign bit of a byte is set to 1
      * From bit 7 to bit 0, flags are arranged like NV_BDIZC (bit 5 is unused)

  * Three general purpose registers
  * The accumulator
    * 8-bit register to store result of arithmetic operations, can also be set to a value from memory
  * Index registers (X and Y) to store data or temporarily control information
    * X register is an 8-bit register usually used as a counter or an offset. Can be set to a value from memory, or used to get/set the value of the stack pointer
    * Y register is identical in size and usage as the X register, but cannot affect the stack pointer

* 8-bit 6502 processor for the PPU
* NES used memory mapped I/O for the CPU to communicate to other components


