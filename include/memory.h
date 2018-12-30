#ifndef MEMORY_H
#define MEMORY_H

unsigned char readByte(unsigned short);
void writeByte(unsigned short, unsigned char);
unsigned char popStack(void);
void pushStack(unsigned char);

#endif
