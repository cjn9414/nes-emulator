#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "SDL2/SDL.h"
#include "ppu.h"
#include "cpu.h"
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240
#define TILE_ROW 32
#define TILE_COL 30
#define TILE_LEN 8

/*
 * Holds all important information for
 * the emulator display.
 */
typedef struct {
  SDL_Renderer *renderer;
  SDL_Window *window;
  SDL_Texture *scanlineTexture;
} EmuDisplay; 

// Define an instance of the EmuDisplay
// struct to hold display data.
EmuDisplay display;

extern uint8_t pTable0[0x1000];
extern uint8_t pTable1[0x1000];
extern NameTable nTable0;
extern NameTable nTable1;
extern NameTable nTable2;
extern NameTable nTable3;
extern uint8_t imagePalette[0x10];
extern uint8_t spritePalette[0x10];
extern const struct color palette[48];
extern struct Header head;

// pixels from end of each scanline to be
// placed at the beginning of the next scanline
uint32_t preRenderPixels[0x10];

/**
 * Performs SDL and memory management
 * related cleanup operations before the
 * program terminates.
 */
void cleanup(void) {
  SDL_DestroyRenderer(display.renderer); 
  SDL_DestroyWindow(display.window); 
  SDL_DestroyTexture(display.scanlineTexture);
  SDL_Quit();
}


/**
 * Checks and handles any SDL event that occurs
 * during runtime of the program.
 */
uint8_t handleEvent(void) {
  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_QUIT:
	//devPrintNameTable0();
	return 0;

      default:
        return 1;
    }
  }
  return 1;
}

/**
 * Calls functions to initialize, render, and
 * display the window for the emulator.
 * Also checks for user events that occur
 * such as closing the display window.
 *
 * @return status
 */
uint8_t presentScene(void)
{
  SDL_RenderPresent(display.renderer);
  if (!handleEvent()) {
    return -1;
  }
  //SDL_Delay(1);
  return 0;
}


void getDisplayStatus() {
  return handleEvent();
}


/**
 * Called once upon the display startup to properly initialize
 * the display and set up key components of the NES graphics.
 */
void displayInit(void) {
  
  // Define flags for SDL_Window and SDL_Renderer.
  int rendererFlags, windowFlags;
  rendererFlags = SDL_RENDERER_ACCELERATED;
  windowFlags = 0;

  // SDL initialization fails.
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Error: %s\n", SDL_GetError());
    exit(1);
  }
  // Instantiate the display window.
  display.window = SDL_CreateWindow("NES Emulator Window", SDL_WINDOWPOS_UNDEFINED, 
    SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);

  // Error creating the window.
  if (!display.window) {
    printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
    exit(1);
  }

  // Sets linear scaling quality.
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  
  // Define the display renderer.
  display.renderer = SDL_CreateRenderer(display.window, -1, rendererFlags);
  
  // Error creating the renderer.
  if (!display.renderer) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
    exit(1);
  }
  atexit(cleanup);
}


/**
 * Converts color struct to a Uint32 value.
 *
 * @param c: struct color instance that contains an RGB value.
 *
 * @returns: Uint32 value that is a convert struct color instance.
 */
Uint32 color2int(struct color c) {
  Uint32 val = 0xFF000000 | (c.rgb[0] << 16) | (c.rgb[1] << 8) | (c.rgb[2]);
  return val;
}


/**
 * Renders a scanline of pixel data onto the display.
 *
 * @param buffer: pointer to the scanline of pixel data.
 * @param scanline: current scanline (row) that is being displayed.
 */
uint8_t renderScanline(uint8_t *buffer, uint8_t scanline) {
  uint8_t tileIdx = 0, fullPaletteIdx = 0, upperPaletteIdx = 0;
  uint8_t fetchCycle = 0;
  uint32_t * pixels;
  int pitch;
  SDL_Texture * text = SDL_CreateTexture(display.renderer,
    SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 
    256, 1);
  if (text == NULL) {
    SDL_Log("SDL_CreateTexture() failed: %s", SDL_GetError());
    exit(1);
  }
  SDL_LockTexture(text, NULL, (void **)&pixels, &pitch); 
  Uint32 * scanlinePixels = (Uint32*) pixels;
  memcpy(scanlinePixels, preRenderPixels, (size_t) 16*sizeof(Uint32));
  for (int tile = 0; tile < 32; tile++) {
    tileIdx = *(buffer + tile); // gets the AT byte for each tile
    uint8_t lowerByte = *(buffer + tile + 32);
    uint8_t upperByte = *(buffer + tile + 64);
    if (scanline % 32 < 16) {
      if (tile % 4 < 2) {
	upperPaletteIdx = (tileIdx & 0b00000011) << 2;
      } else {
        upperPaletteIdx = tileIdx & 0b00001100;
      }
    } else if (tile % 4 < 2) {
      upperPaletteIdx = (tileIdx & 0b00110000) >> 2;
    } else {
      upperPaletteIdx = (tileIdx & 0b11000000) >> 4;
    }
    for (int cycleCount = 0; cycleCount < 8; cycleCount++) {
	fullPaletteIdx = upperPaletteIdx | 
		( ( ( upperByte & ( (1 << (7-cycleCount) ) ) ) >> (7-cycleCount) ) << 1 ) | 
		    ( lowerByte & ( (1 << (7-cycleCount) ) ) ) >> (7-cycleCount);
	//Uint32 colorValue = color2int(palette[imagePalette[(8*tile + cycleCount)%64]]);
	Uint32 colorValue = color2int(palette[imagePalette[fullPaletteIdx]]);
	if (tile < 30) {
	  scanlinePixels[8*tile + cycleCount + 16] = colorValue;
	} else {
	  preRenderPixels[8*(tile-30) + cycleCount] = colorValue;
	}
    }
  }
  SDL_UnlockTexture(text);
  display.scanlineTexture = text;
  SDL_Rect line = (SDL_Rect) { 0, scanline, 256, 1};
  SDL_RenderCopy(display.renderer, display.scanlineTexture, NULL, &line);
  SDL_DestroyTexture(display.scanlineTexture);
  if (presentScene() == -1) exit(1);
}


