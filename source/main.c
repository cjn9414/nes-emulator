#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "display.h"
#include "registers.h"
#include "mmc1.h"
#define KB 1024
#define MAX_SIZE 64*KB

struct registers regs;
struct Header head;
struct MMC1 mmc1;

signed char loadHeader(FILE *file, struct Header* head) {
  unsigned char inspectByte;
  unsigned char format[3];
  fread(&format, sizeof(unsigned char), 3, file);
  if (strcmp(format, "NES") != 0) return -1;
  fseek(file, 1, SEEK_CUR);
  fread(&(head->n_prg_banks), sizeof(unsigned char), 1, file);
  fread(&(head->n_chr_banks), sizeof(unsigned char), 1, file);
  fread(&inspectByte, sizeof(unsigned char), 1, file);
  head->mirror = (inspectByte & 1);
  head->batteryRamBit = (inspectByte & 1 << 1);
  head->trainerBit = (inspectByte & 1 << 2);
  head->fourScreenBit = (inspectByte & 1 << 3);
  head->mapperNumber = inspectByte >> 4;
  fread(&inspectByte, sizeof(unsigned char), 1, file);
  head->mapperNumber += ((inspectByte >> 4) << 4);
  fread(&(head->n_ram_banks), sizeof(unsigned char), 1, file);
  fseek(file, 7, SEEK_CUR);
  return 0;
}

int main(int argc, char **argv) {
  
  if (argc != 2) {
    printf("Expected 2 arguments; %d were given.\n", argc);
    exit(1);
  }

  char *fileName = argv[1]; 
  FILE *file = fopen(fileName, "rb");
  if (!file) {
    printf("File unable to be opened. Is it in the working directory?\n");
    exit(1);
  }
  
  registerPowerup(&regs);
  if (loadHeader(file, &head) < 0) {
    printf("Wrong file type or corrupted file. Please use a .nes file.\n");
    exit(1);
  }
  unsigned char programData[16*KB*head.n_prg_banks];
  unsigned char graphicData[8*KB*head.n_chr_banks];
  unsigned char trainer[KB/2];
  if (head.trainerBit > 0) {
    fread(trainer, sizeof(unsigned char), KB/2, file); 
  }
  fread(programData, sizeof(unsigned char), 16*KB*head.n_prg_banks, file);
  fread(graphicData, sizeof(unsigned char), 8*KB*head.n_chr_banks, file);
  fclose(file);

  mmc1Powerup();
  loadMMC1Ptrs(programData, graphicData);
  //runDisplay();
  return 0;
}

