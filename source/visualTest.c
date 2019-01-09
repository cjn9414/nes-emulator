#include <stdio.h>
#include <stdlib.h>
#include "SDL2/SDL.h"
#include "ppu.h"
#include "cpu.h"
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240
#define TILE_ROW 32
#define TILE_COL 30
typedef struct {
  SDL_Renderer *renderer;
  SDL_Window *window;
  SDL_Texture *texture[TILE_ROW*TILE_COL];
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

SDL_Surface *tiles [TILE_ROW*TILE_COL];

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
  if (!display.renderer) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
    exit(1);
  }
  loadTiles();
}

Uint32 color2int(struct color c) {
  Uint32 val = 0xFF000000 | c.rgb[0] << 16 | c.rgb[1] << 8 | c.rgb[2];
  return val;
}



void renderTiles(void) {
  for (int i = 0; i < TILE_ROW*TILE_COL; i += 2) {
    SDL_Rect loc;
    loc = (SDL_Rect) { 8*(i%TILE_ROW), 8*(i/TILE_ROW), 8, 8 };
    SDL_Texture * textureToRender = display.texture[nTable0.tbl[i]];
    //SDL_Texture * textureToRender = display.texture[i];
    SDL_RenderCopy(display.renderer, textureToRender, NULL, &loc);
    
    loc = (SDL_Rect) { 8 + 8*(i%TILE_ROW), 8*(i/TILE_ROW), 8, 8 };
    textureToRender = display.texture[nTable0.tbl[i+1]];
    //textureToRender = display.texture[i+1];
    SDL_RenderCopy(display.renderer, textureToRender, NULL, &loc);
    
    loc = (SDL_Rect) { 8*(i%TILE_ROW), 8 + 8*(i/TILE_ROW), 8, 8 };
    textureToRender = display.texture[nTable0.tbl[TILE_ROW+i]];
    //textureToRender = display.texture[i+2];
    SDL_RenderCopy(display.renderer, textureToRender, NULL, &loc);
    
    loc = (SDL_Rect) { 8 + 8*(i%TILE_ROW), 8 + 8*(i/TILE_ROW), 8, 8 };
    textureToRender = display.texture[nTable0.tbl[TILE_ROW+i+1]];
    //textureToRender = display.texture[i+3];
    SDL_RenderCopy(display.renderer, textureToRender, NULL, &loc);
  }
}

void loadTiles(void) {
  for (int i = 0; i < 64; i++) {  // size of attribute table
    unsigned char idx = 0;
    int offset;
    for (int j = 0; j < 16; j++) {  // number of tiles per chunk of attribute table
      SDL_Surface * tile = tiles[j + (16*i)];
      tile = SDL_CreateRGBSurfaceWithFormat(
        0, 8, 8, 32, SDL_PIXELFORMAT_ARGB8888);
      if (tile == NULL) {
        SDL_Log("SDL_CreateRGBSurfaceWithFormat() failed: %s", SDL_GetError());
        exit(1);
      }
      offset = 4*(i%8);
      if (j < 4) {
        idx = (0x3 & nTable0.attr[i]) << 2; // get bits 1/0
      } else if (j < 8) {
        offset += 2;
        idx = 0xC & nTable0.attr[i]; // get bits 3/2
      } else if (j < 12) {
        offset += 2*TILE_ROW;
        idx = (0x30 & nTable0.attr[i]) >> 2; // get bits 5/4
      } else {
        offset += 2*TILE_ROW + 2;
        idx = (0xC0 & nTable0.attr[i]) >> 4; // get bits 7/6
      }
      if (j % 4 > 1) offset += TILE_ROW;
      if (j % 2 == 1) offset += 1;
      getPatternData(tile, offset, idx);
      display.texture[j + (16*i)] = SDL_CreateTextureFromSurface(display.renderer, tile);
      SDL_FreeSurface(tile);
    }
  }
}

void getPatternData(SDL_Surface * tile, int offset, unsigned char idx) {
  Uint32 * tilePixels = (Uint32*) tile->pixels;
  // iterate through first half of bytes of the tile
  unsigned char upperBits = idx; 
  for (int row = 0; row < 0x8; row++) {
    unsigned char b1 = pTable0[offset+row];
    unsigned char b2 = pTable0[offset+row+8];
    // iterate through each bit of the tile
    for (int col = 7; col >= 0; col--) {
      idx = upperBits | getBit(b1, col);
      idx = upperBits | (getBit(b2, col) << 1);
      tilePixels[ (row * tile->w) + col ] = color2int(palette[idx]);
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
  for (int i = 0; i < TILE_ROW*TILE_COL; i++) {
    SDL_DestroyTexture(display.texture[i]);
  }
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
