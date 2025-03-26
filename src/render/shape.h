#ifndef SHAPE_H
#define SHAPE_H
#include <SDL3/SDL.h>
#include <string>


class GameRenderer;


class ShapeRenderer{
public:

    inline static const std::string pipelineKey = "PIPE";
    inline static const std::string wireframeKey = "WIREFRAME";

    GameRenderer* gameRenderer;


    ShapeRenderer(GameRenderer*);
    void render();
    void renderCamera();
    ~ShapeRenderer();

private:
    SDL_GPUGraphicsPipelineCreateInfo config, wireframeConfig;

    void initPipelines();
};

#endif