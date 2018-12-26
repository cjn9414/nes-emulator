#include <stdio.h>
#include <stdlib.h>

struct Game {
  unsigned char header[16];
  union {
    unsigned char data[512];
    int existFlag;
  };
} extern Game;

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
  struct Game game;
  fread(game.header, sizeof(unsigned char), 16, file);
  fclose(file);
  for (int i=0; i < 16; i++) {
    printf("%d  ", i);
    for (int j = 0; j < 8; j++) { 
      printf("%d", !!((game.header[i] << j) & 0x80));
    }
    printf("\n");
  }
  return 0;
}
