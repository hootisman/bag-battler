#ifndef PIPELINE_H
#define PIPELINE_H

#include <SDL3/SDL.h>
#include <map>

class GamePipelines{
public:
    // static void buildGraphicsPipeline(SDL_GPUShader*, SDL_GPUShader*);
private:
    std::map<const char*, SDL_GPUGraphicsPipeline*> pipelines;
};

#endif