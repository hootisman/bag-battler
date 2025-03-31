#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "debug.h"
#include "camera.h"
#include <SDL3/SDL.h>
#include <vector>
#include <span>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
class ShapeRenderer;

#define SCREEN_W 640
#define SCREEN_H 480
#define SHADER_RESOURCE_PATH "resources/shaders/"

typedef struct PosColorVertex{
    float x,y,z;
    Uint8 r,g,b,a;
} PosColorVertex;

typedef struct GameBuffer {
    SDL_GPUBuffer* buffer;
    SDL_GPUDevice* gpu;
    SDL_GPUBufferCreateInfo info;

    GameBuffer() {}
    GameBuffer(SDL_GPUDevice*, SDL_GPUBufferUsageFlags, Uint32);
    int getSize();
    ~GameBuffer();

} GameBuffer;

typedef struct GameTransferBuffer {
    SDL_GPUTransferBuffer* buffer;
    SDL_GPUDevice* gpu;
    SDL_GPUTransferBufferCreateInfo info;

    GameTransferBuffer() {}
    GameTransferBuffer(SDL_GPUDevice*, SDL_GPUTransferBufferUsage, Uint32);
    int getSize();
    ~GameTransferBuffer();

    void loadTransferBuffer(void*, Uint32);

} GameTransferBuffer;

class GameRenderer{
public:
    SDL_Window* window;
    SDL_Surface* screenSurface;
    SDL_GPUDevice* gpu;
    GameBuffer* vertexBuffer;
    GameBuffer* indexBuffer;
    GameCamera* camera;
    bool isWireframe;
    ShapeRenderer* shapeRenderer;

    static SDL_GPUShader* initShader(const char*, SDL_GPUDevice*, SDL_GPUShaderStage, Uint32, Uint32, Uint32, Uint32);
    static SDL_GPUGraphicsPipelineCreateInfo createPipelineConfig(
        SDL_GPUPrimitiveType, std::span<SDL_GPUVertexBufferDescription>, std::span<SDL_GPUVertexAttribute>,
        std::span<SDL_GPUColorTargetDescription>, SDL_GPUShader*, SDL_GPUShader*
    );
    void buildGraphicsPipeline(std::string, SDL_GPUGraphicsPipelineCreateInfo&);
    SDL_GPUGraphicsPipeline* getGraphicsPipeline(std::string);

    GameRenderer();
    void render();
    ~GameRenderer();
private:
    std::unordered_map<std::string, SDL_GPUGraphicsPipeline*> graphicPipelines;

    void releasePipelines();
};
#endif