CC := gcc

IDIR := ../include

ODIR := obj

LIBS := -lSDL2

CFLAGS := -I$(IDIR) -w -std=c99

_DEPS := main.h cpu.h registers.h memory.h ppu.h mmc1.h visualTest.h
DEPS := $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ := main.o cpu.o registers.o memory.o ppu.o mmc1.o visualTest.o
OBJ := $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

display: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o display *.gch
