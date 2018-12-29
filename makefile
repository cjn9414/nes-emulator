CC := gcc

CFLAGS := -I -Wall -lSDL2

DEPS := display.h main.h CPU.h registers.h memory.h

OBJ := display.o main.o CPU.o registers.o memory.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

display: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o display *.gch
