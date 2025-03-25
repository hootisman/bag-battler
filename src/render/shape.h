#ifndef SHAPE_H
#define SHAPE_H
#include <SDL3/SDL.h>
#include <string>
#include "graphics.h"

class ShapeRenderer{
public:
    const std::string graphicsPipelineKey = "PIPE";
    const std::string wireframeKey = "WIREFRAME";




private:
    SDL_GPUGraphicsPipelineCreateInfo config, wireframeConfig;
    GameRenderer* gameRenderer;


    ShapeRenderer();
    ~ShapeRenderer();
};


#endif