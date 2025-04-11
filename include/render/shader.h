#ifndef SHAPE_H
#define SHAPE_H
#include <SDL3/SDL.h>
#include <string>



class TestQuadShader{
public:
    inline static const std::string pipelineKey = "PIPE";
    inline static const std::string wireframeKey = "WIREFRAME";

    TestQuadShader();
    // void render();
    // void renderCamera();
    ~TestQuadShader();

private:
    SDL_GPUGraphicsPipelineCreateInfo config, wireframeConfig;

    void initPipelines();
};

#endif