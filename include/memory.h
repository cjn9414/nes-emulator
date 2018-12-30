#ifndef MEMORY_H
#define MEMORY_H

unsigned char readByte(unsigned short);
unsigned char readZeroPage(unsigned char);
void writeByte(unsigned short, unsigned char);
void writeZeroPage(unsigned char, unsigned char);
unsigned char popStack(void);
void pushStack(unsigned char);

#endif
