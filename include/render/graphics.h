#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "debug.h"
#include "util.h"
#include "render/camera.h"
#include <SDL3/SDL.h>
#include <vector>
#include <span>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class TestQuadShader;
class GameModel;

typedef struct PosColorVertex{
    float x,y,z;
    Uint8 r,g,b,a;
} PosColorVertex;

typedef struct GameBuffer {
    SDL_GPUBuffer* buffer;
    SDL_GPUDevice* gpu;
    SDL_GPUBufferCreateInfo info;
    SDL_GPUBufferRegion defaultRegion;

    GameBuffer() {}
    GameBuffer(SDL_GPUDevice*, SDL_GPUBufferUsageFlags, Uint32);
    Uint32 getSize() const;
    ~GameBuffer();

} GameBuffer;

typedef struct GameTransferBuffer {
    SDL_GPUTransferBuffer* buffer;
    SDL_GPUDevice* gpu;
    SDL_GPUTransferBufferCreateInfo info;

    GameTransferBuffer() {}
    GameTransferBuffer(SDL_GPUDevice*, SDL_GPUTransferBufferUsage, Uint32);
    Uint32 getSize() const;
    SDL_GPUTransferBufferLocation getLocation(Uint32);
    ~GameTransferBuffer();

    void loadTransferBuffer(void*, Uint32);

} GameTransferBuffer;

namespace GameRenderer{
    inline SDL_Window* window;
    inline SDL_Surface* screenSurface;
    inline SDL_GPUDevice* gpu;
    inline GameCamera* camera;
    inline GameBuffer* vertexBuffer;
    inline GameBuffer* indexBuffer;
    inline bool isWireframe;
    inline TestQuadShader* shapeRenderer;
    inline std::unordered_map<std::string, SDL_GPUGraphicsPipeline*> graphicPipelines;

    //todo: remove, daring today are we?
    inline GameModel* model;

    SDL_GPUShader* initShader(std::string, SDL_GPUDevice*, SDL_GPUShaderStage, Uint32, Uint32, Uint32, Uint32);
    SDL_GPUGraphicsPipelineCreateInfo createPipelineConfig(
        SDL_GPUPrimitiveType, std::span<SDL_GPUVertexBufferDescription>, std::span<SDL_GPUVertexAttribute>,
        std::span<SDL_GPUColorTargetDescription>, SDL_GPUShader*, SDL_GPUShader*
    );
    void buildGraphicsPipeline(std::string, SDL_GPUGraphicsPipelineCreateInfo&);
    SDL_GPUGraphicsPipeline* getGraphicsPipeline(std::string);
    void initRenderer();
    void uploadVIBuffers(void*, const GameBuffer*, const GameBuffer*);
    void render();
    void closeRenderer();
    void releasePipelines();

    
};
#endif