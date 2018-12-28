#ifndef DISPLAY_H
#define DISPLAY_H

#include "SDL2/SDL.h"

void init(void);
void handleEvent(void);
void prepareScene(void);
void presentScene(void);
void cleanup(void);
void doInput(void);
void runDisplay(void);

#endif
