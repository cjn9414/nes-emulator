#include <stdio.h>
#include <stdlib.h>
#include "SDL2/SDL.h"
#include "ppu.h"
#include "cpu.h"
#define SCREEN_WIDTH  256
#define SCREEN_HEIGHT 240

typedef struct {
  SDL_Renderer *renderer;
  SDL_Window *window;
  SDL_Texture *texture[32*30];
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
extern struct Header head;

Uint32 ** pixels;
SDL_Surface *tiles [32*30];

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
  pixels = malloc(sizeof(Uint32)*SCREEN_WIDTH*SCREEN_HEIGHT);
  memset(pixels, 0xFFFFFFFF, sizeof(Uint32)*SCREEN_WIDTH*SCREEN_HEIGHT);
  if (pixels == NULL) {
    printf("Couldn't allocate memory.\n");
    exit(1);
  }
  if (!display.renderer) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
    exit(1);
  }
  loadTiles();
}

Uint32 color2int(struct color c) {
  Uint32 val = 0x11000000 | c.rgb[0] << 16 | c.rgb[1] << 8 | c.rgb[2];
  return val;
}

void renderTiles(void) {
  for (int i = 0; i < 32*30; i++) {
    SDL_Rect loc = { 8*(i%32), 8*(i/32), 8, 8 };
    SDL_RenderCopy(display.renderer, display.texture[i], NULL, &loc);
  }
}

void loadTiles(void) {
  for (int i = 0; i < 32*30; i++) {
    SDL_Surface * tile = tiles[i];
    tile = SDL_CreateRGBSurfaceWithFormat(
      0, 8, 8, 32, SDL_PIXELFORMAT_ARGB8888);
    if (tile == NULL) {
      SDL_Log("SDL_CreateRGBSurfaceWithFormat() failed: %s", SDL_GetError());
      exit(1);
    }
    getTileData(tile, i);
    display.texture[i] = SDL_CreateTextureFromSurface(display.renderer, tile);
    SDL_FreeSurface(tile);
  }
}

void getTileData(SDL_Surface * tile, int offset) {
  unsigned char idx;
  Uint32 * tilePixels = (Uint32*) tile->pixels;
  // iterate through first half of bytes of the tile
  for (int row = 0; row < 0x8; row++) {
    unsigned char b1 = pTable0[offset+row];
    unsigned char b2 = pTable0[offset+row+8];
    // iterate through each bit of the tile
    for (int col = 7; col >= 0; col--) {
      idx = 0b00000001 & getBit(b1, col);
      idx = idx | (getBit(b2, col) << 1);
      tilePixels[ (row * tile->w) + col ] = 0xFF000000 | color2int(palette[idx]);
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
  renderTiles();
}

void presentScene(void)
{
  SDL_RenderPresent(display.renderer);
}

void cleanup(void) {
  SDL_DestroyRenderer(display.renderer); 
  SDL_DestroyWindow(display.window); 
  for (int i = 0; i < 32*30; i++) {
    SDL_DestroyTexture(display.texture[i]);
  }
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
