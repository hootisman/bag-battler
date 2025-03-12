#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL3/SDL.h>

#define SCREEN_W 640
#define SCREEN_H 480


class GameRenderer{
public:
    SDL_Window* window;
    SDL_Surface* screenSurface;
    SDL_GPUDevice* gpu;

    GameRenderer();
    void render();
    ~GameRenderer();
};


#endif