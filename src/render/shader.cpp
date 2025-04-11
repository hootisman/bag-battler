#include "render/graphics.h"
#include "render/shader.h"


void TestQuadShader::initPipelines(){
	SDL_GPUShader* vertShader = GameRenderer::initShader("transtest.vert.spv", GameRenderer::gpu, SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 0, 0);
	SDL_GPUShader* fragShader = GameRenderer::initShader("SolidColor.frag.spv", GameRenderer::gpu, SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 0, 0, 0);

	SDL_GPUVertexAttribute vertexAttr[] = {{0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0}, {1, 0, SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM, sizeof(float) * 3}};
	SDL_GPUVertexBufferDescription vertexDesc[] = {0, sizeof(PosColorVertex), SDL_GPU_VERTEXINPUTRATE_VERTEX, 0};
	SDL_GPUColorTargetDescription colorTarget[] = {SDL_GetGPUSwapchainTextureFormat(GameRenderer::gpu, GameRenderer::window)};
 
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

    GameRenderer::buildGraphicsPipeline(TestQuadShader::pipelineKey, this->config);
	if (GameRenderer::getGraphicsPipeline(TestQuadShader::pipelineKey) == NULL) throw RendererException("Failed to create Fill Pipeline");

    GameRenderer::buildGraphicsPipeline(TestQuadShader::wireframeKey, this->wireframeConfig);
	if (GameRenderer::getGraphicsPipeline(TestQuadShader::wireframeKey) == NULL) throw RendererException("Failed to create Wireframe Pipeline");

	SDL_ReleaseGPUShader(GameRenderer::gpu, vertShader);
	SDL_ReleaseGPUShader(GameRenderer::gpu, fragShader);
}

TestQuadShader::TestQuadShader(){
    this->initPipelines();
}

TestQuadShader::~TestQuadShader(){

}