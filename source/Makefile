CC = gcc
VG = valgrind

IDIR = ../include
LIBS = -lSDL2

CFLAGS = -I$(IDIR) -w --std=c99
VGFLAGS = --tool=memcheck --leak-check=full --track-origins=yes --show-reachable=yes

ROMDIR = ./ROMS
TESTROM = $(ROMDIR)/donkey kong.nes

VG_OUT = vg_out.txt
BIN = ./display
ODIR = obj

_DEPS = main.h cpu.h registers.h memory.h ppu.h MMC1.h MMC2.h MMC3.h NROM.h mappers.h display.h memoryMappedIO.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJS = main.o cpu.o registers.o memory.o ppu.o MMC1.o MMC2.o MMC3.o NROM.o display.o memoryMappedIO.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))


.PHONY: all
.SILENT: all
all: $(ODIR) $(BIN)
	@echo "Build complete!"

$(ODIR)/%.o: %.c $(DEPS)
	@echo -n "Compiling file: \"$<\".. "
	@$(CC) -c -o $@ $< $(CFLAGS)
	@echo "Done!"

$(BIN): $(OBJS)
	@echo -n "Making executable.. "
	@$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	@echo "Done!"

$(ODIR):
	mkdir -p $(ODIR)

.PHONY: clean
.SILENT: clean
clean:
	@echo -n "Cleaning directory.. "
	-rm -f $(ODIR)/*.o display *.gch cpu.log $(VG_OUT)
	@echo "Done!"

.PHONY: mem
.SILENT: mem
mem: all
	@echo -n "Running memory test.. "
	@$(VG) $(VGFLAGS) $(BIN) "$(TESTROM)" > $(VG_OUT) 2>&1
	@echo "Done!"


.PHONY: help
.SILENT: help
help:
	@echo "Make options: all, clean, help, mem"

