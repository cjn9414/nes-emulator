#include <stdio.h>
#include <stdlib.h>

#include "mappers.h"
#include "main.h"
#include "registers.h"
#include "visualTest.h"

#define KB 1024

// Declare structs which can be found in the header files
struct registers regs;
struct Header head;
struct MMC1 mmc1;

// Declare pointers to cartridge data.
unsigned char * programData;
unsigned char * graphicData;


/**
 * Called once from the main function to
 * load data from the .nes file into the header struct.
 * The header of the .nes file is critical as it
 * dictates how exactly the emulator will operate, 
 * especially with respect to the graphics.
 */
signed char loadHeader(FILE *file, struct Header* head) {
  // Declaring variables that will contain data from
  // the .nes file, which can then be fed into the 
  // attributes of the struct Header instance.
  unsigned char inspectByte;
  unsigned char format[3];
  
  // Copies data from file into previously declared variable.
  fread(format, sizeof(unsigned char), 3, file);
  
  // All .nes formats must begin with "NES".
  if (strcmp(format, "NES") != 0) return -1;
  
  // Loads byte from file into inspection byte, checking for file legitimacy.
  fread(&inspectByte, sizeof(unsigned char), 1, file);
  if (inspectByte != 0x1A) return -1;

  // Loads the number of 16 KB PRG banks and 8 KB CHR banks
  // from the header into the respective variables.
  fread(&(head->n_prg_banks), sizeof(unsigned char), 1, file);
  fread(&(head->n_chr_banks), sizeof(unsigned char), 1, file);
  
  // Loads byte from file into inspection byte.
  // Bits of inspection byte are then looked at
  fread(&inspectByte, sizeof(unsigned char), 1, file);
  head->mirror = (inspectByte & 1);                 // first bit
  head->batteryRamBit = (inspectByte & (1 << 1) );  // second bit
  head->trainerBit = (inspectByte & (1 << 2) );     // third bit
  head->fourScreenBit = (inspectByte & (1 << 3) );  // fourth bit
  head->mapperNumber = inspectByte >> 4;            // four most significant bits
  
  fread(&inspectByte, sizeof(unsigned char), 1, file);
  head->mapperNumber += (inspectByte & 0xF0); // four most significant bits

  // Load the number of 8 KB PRG RAM  (0 indicates 8 KB).
  fread(&(head->n_ram_banks), sizeof(unsigned char), 1, file);
  
  // Move file pointer to the end of the header.
  // The remainder of the header contains no information.
  fseek(file, 7, SEEK_CUR);
  return 0;
}


/**
 * Selects the memory mapper that the .nes file is using.
 * Will call the proper function to setup the memory mapper.
 */
void mapperSetup(void) {
  unsigned char success;
  switch(head.mapperNumber) {
    case 0:
      success = NROMSetup();
      break;
    case 1:
      success = MMC1Setup();
      break;
    case 2:
      success = MMC2Setup();
      break;
    case 3:
      success = MMC3Setup();
      break;
    default:
      printf("Error: Mapper type not supported!.\n");
      exit(1);
  }
  if (!success) {
    printf("Error: Couldn't setup mapper number %u.\n", head.mapperNumber);
  }
}


/**
 * This is the function that will be called when the
 * emulator program is run. This function is responsible for  
 * setting up all important functions of the emulator, and 
 * calling functions that will operate the processors of the
 * NES as well as the graphics using the SDL2 library.
 */
int main(int argc, char **argv) {
 
  // Declaring the string that represents the name of the .nes file.
  // Declaring the pointer to read from the .nes file.
  char *fileName;
  FILE *file;

  // This program is expected to be ran with the .nes filename as an argument.
  // Running this program with no arguments, or more than one will result in an error.
  if (argc != 2) {
    printf("Error: Expected 2 arguments; %d were given.\n", argc);
    exit(1);
  }
  
  // Initializing file pointer based on program argument.
  fileName = argv[1]; 
  file = fopen(fileName, "rb");
  
  // If the pointer to the file is null, the program will end.
  // This is likely because the given filename is not in the working directory.
  if (!file) {
    printf("Error: File unable to be opened. Is it in the working directory?\n");
    exit(1);
  }
  
  // A return of -1 from loadHeader means that the file contents 
  // could not be understood or clearly recognized as a .nes file.
  if (loadHeader(file, &head) < 0) {
    printf("Wrong file type or corrupted file. Please use a .nes file.\n");
    exit(1);
  }
  // Declare the ROM data that will be copied from the .nes file.
  programData = malloc(16*KB*head.n_prg_banks);
  graphicData = malloc(8*KB*head.n_chr_banks);
  unsigned char * trainer;
  
  // If the trainer exists in the file, load it into the array.
  if (head.trainerBit) {
    trainer = malloc(512);
    fread(trainer, sizeof(unsigned char), 512, file); 
  }
  
  // Load the program and graphic data into the respective arrays.
  fread(programData, sizeof(unsigned char), 16*KB*head.n_prg_banks, file);
  fread(graphicData, sizeof(unsigned char), 8*KB*head.n_chr_banks, file);
  
  // Terminate the file pointer.
  fclose(file);
  
  // Load the on-power status of the memory mapper and the cpu registers.
  
  mapperSetup();
  registerPowerup(&regs);
  // Initialize the picture display.
  displayInit();
  
  // Run the emulator display and perform CPU step.
  while (1) {
    step();
    unsigned char displayClosed = runDisplay();
    if (displayClosed) break;

  }
  // Free dynamically allocated memory.
  free(programData);
  free(graphicData);
  if (head.trainerBit) free(trainer);

  return 0;
}

