CC := gcc

IDIR := ../include

ODIR := obj

LIBS := -lSDL2

CFLAGS := -I$(IDIR) -w -std=c99

_DEPS := display.h main.h cpu.h registers.h memory.h
DEPS := $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ := display.o main.o cpu.o registers.o memory.o
OBJ := $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

display: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o display *.gch
