CC := gcc

IDIR := ../include

ODIR := obj

LIBS := -lSDL2

CFLAGS := -I$(IDIR) -w --std=c99

_DEPS := main.h cpu.h registers.h memory.h ppu.h MMC1.h MMC2.h MMC3.h NROM.h mappers.h visualTest.h
DEPS := $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ := main.o cpu.o registers.o memory.o ppu.o MMC1.o MMC2.o MMC3.o NROM.o visualTest.o
OBJ := $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

display: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o display *.gch
