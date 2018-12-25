#include <stdio.h>
#include <stdlib.h>
#include "SDL2/SDL.h"
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

typedef struct {
  SDL_Renderer *renderer;
  SDL_Window *window;
} App;

App app;

void init() {
  int rendererFlags, windowFlags;
  rendererFlags = SDL_RENDERER_ACCELERATED;
  windowFlags = 0;
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Error: %s\n", SDL_GetError());
    exit(1);
  }

  app.window = SDL_CreateWindow("Window ayyy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);
  if (!app.window) {
    printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
    exit(1);
  }
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  app.renderer = SDL_CreateRenderer(app.window, -1, rendererFlags);
  if (!app.renderer) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
    exit(1);
  }
}

void handleEvent() {
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
	SDL_SetRenderDrawColor(app.renderer, 96, 128, 255, 255);
	SDL_RenderClear(app.renderer);
}

void presentScene(void)
{
	SDL_RenderPresent(app.renderer);
}

void cleanup(void) {
  SDL_DestroyRenderer(app.renderer); 
  SDL_DestroyWindow(app.window); 
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


int main()
{
	memset(&app, 0, sizeof(App));
	init();
	atexit(cleanup);
	while (1)
	{
		prepareScene();
		doInput();
		presentScene();	
		SDL_Delay(16);
	}
	return 0;
}
