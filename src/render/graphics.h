#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL3/SDL.h>

#define SCREEN_W 640
#define SCREEN_H 480


const float triangle_v[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f
};

class GameShader{
public:
    // GameShader();
    static SDL_GPUShader* initShader(SDL_GPUDevice*, SDL_GPUShaderStage, const char*, Uint32, Uint32, Uint32, Uint32);
    // ~GameShader();
};


class GameRenderer{
public:
    SDL_Window* window;
    SDL_Surface* screenSurface;
    SDL_GPUGraphicsPipeline* fillPipeline;
    SDL_GPUGraphicsPipeline* linePipeline;
    SDL_GPUDevice* gpu;

    GameRenderer();
    void render();
    ~GameRenderer();
};


#endif