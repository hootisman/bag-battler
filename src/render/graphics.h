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


#define SCREEN_W 640
#define SCREEN_H 480
#define SHADER_RESOURCE_PATH "resources/shaders/"

typedef struct PosColorVertex{
    float x,y,z;
    Uint8 r,g,b,a;
} PosColorVertex;


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

    static SDL_GPUShader* initShader(const char*, SDL_GPUDevice*, SDL_GPUShaderStage, Uint32, Uint32, Uint32, Uint32);
    static void configurePipelineInfo(
        SDL_GPUGraphicsPipelineCreateInfo*, std::span<SDL_GPUVertexBufferDescription>, std::span<SDL_GPUVertexAttribute>,
        std::span<SDL_GPUColorTargetDescription>, SDL_GPUShader*, SDL_GPUShader*
    );

    GameRenderer();
    void buildGraphicsPipeline(std::string, SDL_GPUGraphicsPipelineCreateInfo&);
    SDL_GPUGraphicsPipeline* getGraphicsPipeline(std::string);
    void render();
    ~GameRenderer();
private:
    std::unordered_map<std::string, SDL_GPUGraphicsPipeline*> graphicPipelines;
};

#endif