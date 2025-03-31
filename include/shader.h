#ifndef SHAPE_H
#define SHAPE_H
#include <SDL3/SDL.h>
#include <string>


// class GameRenderer;


class GameShader{
public:

    inline static const std::string pipelineKey = "PIPE";
    inline static const std::string wireframeKey = "WIREFRAME";

    // GameRenderer* gameRenderer;


    GameShader();
    void render();
    void renderCamera();
    ~GameShader();

private:
    SDL_GPUGraphicsPipelineCreateInfo config, wireframeConfig;

    void initPipelines();
};

#endif