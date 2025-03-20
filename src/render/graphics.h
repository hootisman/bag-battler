#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "debug.h"
#include "camera.h"
#include <SDL3/SDL.h>
#include <vector>
#include <span>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define SCREEN_W 640
#define SCREEN_H 480

typedef struct PosColorVertex{
    float x,y,z;
    Uint8 r,g,b,a;
} PosColorVertex;

typedef struct TempGarbo{
    float x, y, z, a;
} TempGarbo;

class GameShader{
public:
    // GameShader();
    static SDL_GPUShader* initShader(SDL_GPUDevice*, SDL_GPUShaderStage, const char*, Uint32, Uint32, Uint32, Uint32);
    static void configurePipelineInfo(
        SDL_GPUGraphicsPipelineCreateInfo*,
        std::span<SDL_GPUVertexBufferDescription>,
        std::span<SDL_GPUVertexAttribute>,
        std::span<SDL_GPUColorTargetDescription>,
        SDL_GPUShader*,
        SDL_GPUShader*
    );
    // ~GameShader();
};


class GameRenderer{
public:
    SDL_Window* window;
    SDL_Surface* screenSurface;
    SDL_GPUGraphicsPipeline* fillPipeline;
    SDL_GPUGraphicsPipeline* linePipeline;
    SDL_GPUDevice* gpu;
    SDL_GPUBuffer* vertexBuffer;
    SDL_GPUBuffer* indexBuffer;
    GameCamera* camera;
    bool isWireframe;

    Uint64 lastTime;

    GameRenderer();
    void render();
    ~GameRenderer();
// private:
//     SDL_GPUBuffer* createBuffer();
};

#endif