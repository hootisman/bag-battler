#include "graphics.h"
#include "shaper.h"


void ShapeRenderer::initPipelines(){
	SDL_GPUShader* vertShader = GameRenderer::initShader("transtest.vert.spv", this->gameRenderer->gpu, SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 0, 0);
	SDL_GPUShader* fragShader = GameRenderer::initShader("SolidColor.frag.spv", this->gameRenderer->gpu, SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 0, 0, 0);

	SDL_GPUVertexAttribute vertexAttr[] = {{0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0}, {1, 0, SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM, sizeof(float) * 3}};
	SDL_GPUVertexBufferDescription vertexDesc[] = {0, sizeof(PosColorVertex), SDL_GPU_VERTEXINPUTRATE_VERTEX, 0};
	SDL_GPUColorTargetDescription colorTarget[] = {SDL_GetGPUSwapchainTextureFormat(this->gameRenderer->gpu, this->gameRenderer->window)};
 
    this->config = GameRenderer::createPipelineConfig(
        SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, 
        vertexDesc, 
        vertexAttr, 
        colorTarget, 
        vertShader, 
        fragShader
    );
    this->wireframeConfig = this->config;

    this->config.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    this->wireframeConfig.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;

    this->gameRenderer->buildGraphicsPipeline(ShapeRenderer::pipelineKey, this->config);
	if (this->gameRenderer->getGraphicsPipeline(ShapeRenderer::pipelineKey) == NULL) throw RendererException("Failed to create Fill Pipeline");

    this->gameRenderer->buildGraphicsPipeline(ShapeRenderer::wireframeKey, this->wireframeConfig);
	if (this->gameRenderer->getGraphicsPipeline(ShapeRenderer::wireframeKey) == NULL) throw RendererException("Failed to create Wireframe Pipeline");

	SDL_ReleaseGPUShader(this->gameRenderer->gpu, vertShader);
	SDL_ReleaseGPUShader(this->gameRenderer->gpu, fragShader);
}

ShapeRenderer::ShapeRenderer(GameRenderer* gameRenderer){
    this->gameRenderer = gameRenderer;
    this->initPipelines();
}

ShapeRenderer::~ShapeRenderer(){

}