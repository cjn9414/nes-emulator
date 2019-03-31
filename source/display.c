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
 */
void presentScene(void)
{
  SDL_RenderPresent(display.renderer);
  if (!handleEvent()) return -1;
  SDL_Delay(1);
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
  Uint32 val = 0xFF000000 | c.rgb[0] << 16 | c.rgb[1] << 8 | c.rgb[2];
  return val;
}


/**
 * Renders a scanline of pixel data onto the display.
 *
 * @param buffer: pointer to the scanline of pixel data.
 * @param scanline: current scanline (row) that is being displayed.
 */
uint8_t renderScanline(uint8_t *buffer, uint8_t scanline) {

  uint8_t paletteIdx = 0;
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
  for (int i = 0; i < 32; i++) {
    for (int cycleCount = 0; cycleCount < 8; cycleCount++) {
      if (scanline % 32 < 16) {
        if ((i*8 + cycleCount) % 32 < 16) {
          paletteIdx = (*(buffer+3*i) & 0b00000011) << 2;
        } else {
          paletteIdx = (*(buffer+3*i) & 0b00001100);
        }
      } else if ((i*8 + cycleCount) % 32 < 16) {
        paletteIdx = (*(buffer+3*i) & 0b00110000) >> 2;
      } else {
        paletteIdx = (*(buffer+3*i) & 0b11000000) >> 4;
      }

      paletteIdx |= ( *(buffer+3*i+1) & ( (1 << 7-cycleCount ) ) >> ( cycleCount-7 ) ) << 1;
      paletteIdx |=   *(buffer+3*i+2) & ( (1 << 7-cycleCount ) ) >> ( cycleCount-7 );
      Uint32 colorValue = color2int(palette[imagePalette[paletteIdx]]);
      //printf("color: %X index: %X palette index: %X\n", colorValue, paletteIdx, imagePalette[paletteIdx]);
      *(scanlinePixels + 8*i + cycleCount) = colorValue; 
    }
  }

  SDL_UnlockTexture(text);
  display.scanlineTexture = text;
  SDL_Rect line = (SDL_Rect) { 0, scanline, 256, 1};
  SDL_RenderCopy(display.renderer, display.scanlineTexture, NULL, &line);
  presentScene();
}


































///**
// * Maps the previously prepared SDL_Textures to an area on the display.
// */
//void renderTiles(void) {
//  // Iterate through the size of the display in number of tiles.
//  for (int i = 0; i < TILE_ROW*TILE_COL; i += 2) {
//    
//    // Declare an SDL_Rect variable to store location and size of
//    // a tile on the screen.
//    SDL_Rect loc;
//
//    // Update SDL_Rect instance to represent top-left tile of a 
//    // 2x2 tile area on the screen.
//    loc = (SDL_Rect) { TILE_LEN*(i%TILE_ROW), TILE_LEN*(i/TILE_ROW),
//      TILE_LEN, TILE_LEN };
//    
//    // Find the texture to render and copy it to the display
//    // at the correct location.
//    //SDL_Texture * textureToRender = display.texture[nTable0.tbl[i]];
//    SDL_Texture * textureToRender = display.texture[i];
//    SDL_RenderCopy(display.renderer, textureToRender, NULL, &loc);
//    
//    // Update SDL_Rect instance to represent top-right tile of a 
//    // 2x2 tile area on the screen.
//    loc = (SDL_Rect) { TILE_LEN + TILE_LEN*(i%TILE_ROW),
//      TILE_LEN*(i/TILE_ROW), TILE_LEN, TILE_LEN};
//    
//    //textureToRender = display.texture[nTable0.tbl[i+1]];
//    textureToRender = display.texture[i+1];
//    SDL_RenderCopy(display.renderer, textureToRender, NULL, &loc);
//    
//    // Update SDL_Rect instance to represent bottom-left 
//    // tile of a 2x2 tile area on the screen.
//    loc = (SDL_Rect) { TILE_LEN*(i%TILE_ROW),
//      TILE_LEN + TILE_LEN*(i/TILE_ROW), TILE_LEN, TILE_LEN };
//    
//    //textureToRender = display.texture[nTable0.tbl[i+2]];
//    textureToRender = display.texture[i+2];
//    SDL_RenderCopy(display.renderer, textureToRender, NULL, &loc);
//    
//    // Update SDL_Rect instance to represent bottom-right
//    // tile of a 2x2 tile area on the screen.
//    loc = (SDL_Rect) { TILE_LEN + TILE_LEN*(i%TILE_ROW),
//      TILE_LEN + TILE_LEN*(i/TILE_ROW), TILE_LEN, TILE_LEN };
//    
//    //textureToRender = display.texture[nTable0.tbl[i+3]];
//    textureToRender = display.texture[i+3];
//    SDL_RenderCopy(display.renderer, textureToRender, NULL, &loc);
//  }
//}
//
//
///**
// * Iterate through the attribute table to get the most
// * significant two bits of the index that addresses a color
// * from the color palette for each tile in the display.
// * This index is passed into a function that both retrieves
// * the lower two bits of the index, and loads the pixel data
// * into each tile's pixel data array.
// */
//void loadTiles(void) {
//  // Declare index used to address color in the color palette.
//  uint8_t idx;
//  int offset;
//  // Iterate through the size of the attribute table, in bytes.
//  for (uint8_t i = 0; i < 64; i++) {
//    // Iterate though the number of tiles per byte of the attribute table.
//    for (uint8_t j = 0; j < 16; j++) { 
//      // Define the pointer to the SDL_Texture representing the tile.
//      SDL_Texture * tile = SDL_CreateTexture(display.renderer,
//        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 
//        TILE_LEN, TILE_LEN);
//      // Error creating the SDL_Texture.
//      if (tile == NULL) {
//        SDL_Log("SDL_CreateTexture() failed: %s", SDL_GetError());
//        exit(1);
//      }
//      // Define the offset of the tile data in the pattern table.
//      // May be incremented before it is used next.
//      offset = 4*(i%TILE_LEN) + 128*(i/TILE_LEN);
//      // Upper-left quadrant.
//      if (j < 4) {
//        idx = (0x3 & nTable0.attr[i]) << 2;   // get bits 1/0
//      } 
//      // Upper-right quadrant.
//      else if (j < 8) {
//        offset += 2;
//        idx = 0xC & nTable0.attr[i];          // get bits 3/2
//      } 
//      // Lower-left quadrant.
//      else if (j < 12) {
//        offset += 2*TILE_ROW;
//        idx = (0x30 & nTable0.attr[i]) >> 2;  // get bits 5/4
//      } 
//      // Lower-right quadrant.
//      else {
//        offset += 2*TILE_ROW + 2;
//        idx = (0xC0 & nTable0.attr[i]) >> 4;  // get bits 7/6
//      }
//      // Lower two tiles of 2x2 tile area. Offset incremented by fill tile row.
//      if (j % 4 > 1) offset += TILE_ROW;
//
//      // Right two tiles of 2x2 tile area. Offset incremented by one.
//      if (j % 2 == 1) offset += 1;
//      
//      // Loads pixel data into SDL_Texture.
//      getPatternData(tile, offset, idx);
//      // Edge case, as the upper nybble of each byte in the attribute table
//      // in the last row is not used.
//      if (i >= 56) {
//        if (j < 8) {
//          display.texture[(16*56) + 8*(i-56) + j] = tile;
//        }
//      } else display.texture[j + (16*i)] = tile;
//    }
//  }
//}
//
//
///**
// * Loads pixel data from a pattern table into an SDL_Texture.
// *
// * @param tile: Pointer to an instance of an SDL_Texture.
// * @param offset: Offset of the tile data in the pattern
// *                table from the start of the table.
// * @param idx: Index of the color used for a pixel in the color palette.
// *             The palette is indexed by four bits, and this parameter
// *             is passed into the function with the most significant
// *             two bits evaluated. The lower two bits are added to this
// *             value in the function to complete the four bit index.
// */
//void getPatternData(SDL_Texture * tile, int offset, uint8_t idx) {
//  // Flattened array pointer to contain pixel data.
//  uint32_t * pixels;
//  int pitch;
//  
//  // Set the texture for write only access to pixel data.
//  SDL_LockTexture(tile, NULL, (void **)&pixels, &pitch); 
//  
//  // Define the pointer to the pixel data for a tile.
//  Uint32 * tilePixels = (Uint32*) pixels;
//  
//  // Define a variable that holds that two most significant bits
//  // of the four bit index.
//  uint8_t upperBits = idx;
//  // Iterate through each byte in a tile from the pattern table.
//  for (int row = 0; row < TILE_LEN; row++) {
//    // Get two dependent bytes from the tile.
//    uint8_t b1 = pTable0[offset+row];
//    uint8_t b2 = pTable0[offset+row+TILE_LEN];
//
//    // Iterate through each bit of the specified bytes.
//    for (int col = TILE_LEN-1; col >= 0; col--) {
//      // Append upper two bits and lower two bits to find the index.
//      idx = upperBits | getBit(b1, col);
//      idx = upperBits | (getBit(b2, col) << 1);
//      
//      // Find the color in the palette from the index, and 
//      // set the respective address in the pixel data array to the color.
//      
//      tilePixels[ (row * TILE_LEN) + (TILE_LEN-1-col) ] = imagePalette[idx];//color2int(palette[idx]);
//      printf("%X", idx);
//    }
//  }
//  // Set the texture for graphical update and display of pixel data.
//  SDL_UnlockTexture(tile);
//}


/**
 * Prepares the window display before
 * each frame during runtime.
 */
//void prepareScene(void)
//{
//  SDL_RenderClear(display.renderer);
//}

/**
 * Renders the window display with an
 * SDL_Renderer instance that contains
 * all the pixel data for each tile.
 */
