#ifndef MEMORY_H
#define MEMORY_H

uint8_t readByte(unsigned short);
uint8_t readZeroPage(uint8_t);
void writeByte(unsigned short, uint8_t);
void writeZeroPage(uint8_t, uint8_t);
uint8_t popStack(void);
void pushStack(uint8_t);

#endif
