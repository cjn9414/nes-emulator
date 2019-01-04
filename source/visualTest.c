#include <stdio.h>
#include <stdlib.h>
#include "SDL2/SDL.h"
#include "ppu.h"
#include "cpu.h"
#define SCREEN_WIDTH  256
#define SCREEN_HEIGHT 240

typedef struct {
  Uint32 argb[0x8][0x8];
} Tile;

typedef struct {
  SDL_Renderer *renderer;
  SDL_Window *window;
  SDL_Texture *texture;
} EmuDisplay;

EmuDisplay display;

extern unsigned char pTable0[0x1000];
extern unsigned char pTable1[0x1000];
extern NameTable nTable0;
extern NameTable nTable1;
extern NameTable nTable2;
extern NameTable nTable3;
extern unsigned char imagePalette[0x10];
extern unsigned char spritePalette[0x10];
extern const struct color palette[48];

Tile tiles[0x100];
Uint32 * pixels;

void init(void) {
  int rendererFlags, windowFlags;
  rendererFlags = SDL_RENDERER_ACCELERATED;
  windowFlags = 0;
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Error: %s\n", SDL_GetError());
    exit(1);
  }

  display.window = SDL_CreateWindow("Window whats good", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);
  if (!display.window) {
    printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
    exit(1);
  }
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  display.renderer = SDL_CreateRenderer(display.window, -1, rendererFlags);
  display.texture = SDL_CreateTexture(display.renderer, 
    SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);
 
  //unsigned long * pixels = new unsigned long[SCREEN_WIDTH*SCREEN_HEIGHT]; 
  pixels = malloc(sizeof(Uint32)*SCREEN_WIDTH*SCREEN_HEIGHT);
  if (pixels == NULL) {
    printf("Couldn't allocate memory.\n");
    exit(1);
  }
  if (!display.renderer) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
    exit(1);
  }
  loadTiles();
  loadDisplay(pixels);
}

Uint32 color2int(struct color c) {
  Uint32 val = 0x11000000 | c.rgb[0] << 16 | c.rgb[1] << 8 | c.rgb[2];
  return val;
}

void loadTiles(void) {
  // iterate through the pattern table, 16 bytes at a time
  for (int i = 0; i < 0x1000; i += 0x10) {
    // tile to be added to list of tiles
    Tile tile;
    unsigned char idx;
    // iterate through first half of bytes of the tile
    for (int j = 0; j < 0x8; j++) {
      unsigned char b1 = pTable0[i+j];
      unsigned char b2 = pTable0[i+j+8];
      // iterate through each bit of the tile
      for (int k = 7; k >= 0; k--) {
        idx = 0b00000001 & getBit(b1, k);
        idx = idx | (getBit(b2, k) << 1);
        tile.argb[j][7-k] = color2int(palette[idx]);
      }
    }
    tiles[i/0x10] = tile;
  }
}

void loadDisplay(Uint32 * pixels) {
  //iterate through first 64/256 tiles
  for (int i = 0; i < 0x40; i++) {
    for (int j = 0; j < 0x40; j++) {
      *(pixels + ((i*0x40) + j)*sizeof(Uint32)) = tiles[i].argb[j/8][j%8];
    }
  }
}

void handleEvent(void) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        exit(0);
        break;

      default:
        break;
    }
  }
}

void prepareScene(void)
{
	SDL_RenderClear(display.renderer);
  SDL_RenderCopy(display.renderer, display.texture, NULL, NULL);
}

void presentScene(void)
{
	SDL_RenderPresent(display.renderer);
}

void cleanup(void) {
  SDL_DestroyRenderer(display.renderer); 
  SDL_DestroyWindow(display.window); 
  SDL_DestroyTexture(display.texture);
  free(pixels);
  SDL_Quit();
}

unsigned char doInput(void) {
  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_QUIT:
        return 0;

      default:
        return 1;
    }
  }
  return 1;
}


void runDisplay(void)
{
	//memset(&display, 0, SCREEN_WIDTH*SCREEN_HEIGHT*(sizeof(unsigned long)));
  init();
	atexit(cleanup);
  while (1)
	{
		prepareScene();
		if (!doInput()) break;
		presentScene();	
		SDL_Delay(16);
	} 
}
