#include <stdio.h>
#include <stdlib.h>
#include "SDL2/SDL.h"
#include "display.h"
#define SCREEN_WIDTH  256
#define SCREEN_HEIGHT 240

typedef struct {
  SDL_Renderer *renderer;
  SDL_Window *window;
} EmuDisplay;

EmuDisplay display;

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
	SDL_SetRenderDrawColor(display.renderer, 96, 128, 255, 255);
	SDL_RenderClear(display.renderer);
}

void presentScene(void)
{
	SDL_RenderPresent(display.renderer);
}

void cleanup(void) {
  SDL_DestroyRenderer(display.renderer); 
  SDL_DestroyWindow(display.window); 
  SDL_Quit();
}

void doInput(void) {
  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_QUIT:
        exit(0);
        break;

      default:
        break;
    }
  }
}


void runDisplay(void)
{
	memset(&display, 0, sizeof(EmuDisplay));
	init();
	atexit(cleanup);
	while (1)
	{
		prepareScene();
		doInput();
		presentScene();	
		SDL_Delay(16);
	}
}
