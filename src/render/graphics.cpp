#include "render/graphics.h"
#include "render/shader.h"
#include "model/model.h"

/* GameBuffer */
GameBuffer::GameBuffer(SDL_GPUDevice* gpu, SDL_GPUBufferUsageFlags usage, Uint32 size) {
	this->info = { .usage = usage, .size = size};
	this->gpu = gpu;
	this->buffer = SDL_CreateGPUBuffer(this->gpu, &this->info);
	this->defaultRegion = {
		.buffer = this->buffer, 
		.offset = 0, 
		.size = this->getSize()
	};
	if(this->buffer == NULL) throw RendererException("Failed to create game buffer");
}

Uint32 GameBuffer::getSize() const{
	return this->info.size;
}


GameBuffer::~GameBuffer(){
	SDL_Log("Releasing regular buffer, %u", this->info.usage);
	SDL_ReleaseGPUBuffer(this->gpu, this->buffer);
}


/* GameTransferBuffer */
GameTransferBuffer::GameTransferBuffer(SDL_GPUDevice* gpu, SDL_GPUTransferBufferUsage usage, Uint32 size) {
	this->info = {.usage = usage, .size = size};
	this->gpu = gpu;
	this->buffer = SDL_CreateGPUTransferBuffer(this->gpu, &this->info);
	if(this->buffer == NULL) throw RendererException("Failed to create transfer buffer");
}

Uint32 GameTransferBuffer::getSize() const{
	return this->info.size;
}

SDL_GPUTransferBufferLocation GameTransferBuffer::getLocation(Uint32 offset){
	return {
		.transfer_buffer = this->buffer, 
		.offset = offset
	};
}

void GameTransferBuffer::loadTransferBuffer(void* data, Uint32 size){
	void* transferData = SDL_MapGPUTransferBuffer(this->gpu, this->buffer, false);
	if(transferData == NULL) throw RendererException("Failed to map transfer buffer to gpu");

	SDL_memmove(transferData, data, size);
	

	SDL_UnmapGPUTransferBuffer(this->gpu, this->buffer);
}

GameTransferBuffer::~GameTransferBuffer(){
	SDL_Log("Releasing transfer buffer");
	SDL_ReleaseGPUTransferBuffer(this->gpu, this->buffer);
}

/* GameRenderer */

/**
 * Loads shader file and returns pointer to the SDL shader
 * 
 * 
 * \param fileName name of shader file to load
 * \param gpu gpu device being used
 * \param stage shader stage being used e.g. vertex, fragment
 * \param sampler num of samplers defined in shader
 * \param uniformBuffer num of uniform buffers defined in shader
 * \param storageBuffer num of storage buffers defined in shader
 * \param storageTexture num of storage textures defined in shader
 */
SDL_GPUShader* GameRenderer::initShader(
	std::string fileName,
	SDL_GPUDevice* gpu, SDL_GPUShaderStage stage, 
	Uint32 sampler, Uint32 uniformBuffer, 
	Uint32 storageBuffer, Uint32 storageTexture
){
	const char* entryPoint = "main";	//todo entrypoint and format different per system
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_SPIRV;
	std::string fullPath = GameUtil::buildFilePath(GameUtil::SHADER_RESOURCE_PATH, fileName);

	size_t fileSize;
	void* file;

	file = SDL_LoadFile(fullPath.c_str(), &fileSize);
	if(file == NULL) throw RendererException("Error loading file");

	SDL_GPUShaderCreateInfo info = {
		.code_size = fileSize,
		.code = (const Uint8*)file,
		.entrypoint = entryPoint,
		.format = format,
		.stage = stage,
		.num_samplers = sampler,
		.num_storage_textures = storageTexture,
		.num_storage_buffers = storageBuffer,
		.num_uniform_buffers = uniformBuffer,
		.props = 0
	};
	SDL_GPUShader* shader = SDL_CreateGPUShader(gpu, &info);
	if (shader == NULL) {SDL_free(file); throw RendererException("Error creating shader from file");}
	SDL_free(file);

	return shader;
}


SDL_GPUGraphicsPipelineCreateInfo GameRenderer::createPipelineConfig(
	SDL_GPUPrimitiveType primType,
	std::span<SDL_GPUVertexBufferDescription> vertexDesc,
	std::span<SDL_GPUVertexAttribute> vertexAttr,
	std::span<SDL_GPUColorTargetDescription> colorTarget,
	SDL_GPUShader* vertShader,
	SDL_GPUShader* fragShader
){

	SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {
		.vertex_shader = vertShader,
		.fragment_shader = fragShader,
		.vertex_input_state = {
			.vertex_buffer_descriptions = vertexDesc.data(),
			.num_vertex_buffers = (Uint32)vertexDesc.size(),
			.vertex_attributes = vertexAttr.data(),
			.num_vertex_attributes = (Uint32)vertexAttr.size()
		},
		.primitive_type = primType,
		.target_info = {.color_target_descriptions = colorTarget.data(), .num_color_targets = (Uint32)colorTarget.size()}
	};

	return pipelineInfo;
}
void GameRenderer::uploadVIBuffers(void* data, const GameBuffer* vertexBuffer, const GameBuffer* indexBuffer){
	Uint32 transferSize = vertexBuffer->getSize() + indexBuffer->getSize();
	GameTransferBuffer transferBuffer = GameTransferBuffer(GameRenderer::gpu, SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, transferSize);

	transferBuffer.loadTransferBuffer(data, transferSize);

	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(GameRenderer::gpu);
	if(uploadCmdBuf == NULL) throw RendererException("Failed to acquire command buffer");

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

	auto transferVertexLoc = transferBuffer.getLocation(0);
	auto transferIndexLoc = transferBuffer.getLocation(vertexBuffer->getSize());
	
	SDL_UploadToGPUBuffer(copyPass, &transferVertexLoc, &vertexBuffer->defaultRegion, false);
	SDL_UploadToGPUBuffer(copyPass, &transferIndexLoc, &indexBuffer->defaultRegion, false);

	SDL_EndGPUCopyPass(copyPass);
	if(!SDL_SubmitGPUCommandBuffer(uploadCmdBuf)) throw RendererException("Failed to submit command buffer");
}

void GameRenderer::initRenderer(){

	/* Create SDL */
	if(!SDL_Init(SDL_INIT_VIDEO)) throw RendererException("Failed to init SDL");

	/* Create Window*/
	GameRenderer::window = SDL_CreateWindow( "hi", GameUtil::SCREEN_W, GameUtil::SCREEN_H, 0);
	if(!GameRenderer::window) throw RendererException("Error creating window");

	/* Create GPU Device*/
	GameRenderer::gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
	if(!GameRenderer::gpu) throw RendererException("Error creating gpu device");

	//bind gpu to window
	if(!SDL_ClaimWindowForGPUDevice(GameRenderer::gpu, GameRenderer::window)) throw RendererException("Error binding gpu to window");

	GameRenderer::shapeRenderer = new TestQuadShader();

	/* Buffers */

	GameRenderer::vertexBuffer = new GameBuffer(GameRenderer::gpu, SDL_GPU_BUFFERUSAGE_VERTEX, sizeof(PosColorVertex) * 4);
	GameRenderer::indexBuffer = new GameBuffer(GameRenderer::gpu, SDL_GPU_BUFFERUSAGE_INDEX, sizeof(Uint16) * 6);
	
	/*
	Uint32 transferSize = GameRenderer::vertexBuffer->getSize() + GameRenderer::indexBuffer->getSize();
	
	PosColorVertex* transferData = (PosColorVertex*) SDL_malloc(transferSize);

	transferData[0] = {-1.0,-1.0,0,255,0,0,255};	//bot left
	transferData[1] = {1.0,-1.0,0,0,255,0,255};		//bot right
	transferData[2] = {-1.0,1.0,0,0,0,255,255};		//top left
	transferData[3] = {1.0,1.0,0,0,0,255,255};		//top right

	Uint16* indexData = (Uint16*) &(transferData[4]);
	indexData[0] = 1;
	indexData[1] = 0;
	indexData[2] = 2;
	indexData[3] = 2;
	indexData[4] = 3;
	indexData[5] = 1;

	GameRenderer::uploadVIBuffers(transferData, GameRenderer::vertexBuffer, GameRenderer::indexBuffer);

	SDL_free(transferData);
	*/

	SDL_Log("%s\n", SDL_GetGPUDeviceDriver(GameRenderer::gpu));

	GameRenderer::camera = new GameCamera();
	GameRenderer::isWireframe = false;

	GameRenderer::model = new GameModel("teapot.stl");
}

void GameRenderer::render(){

	SDL_GPUCommandBuffer* buff = SDL_AcquireGPUCommandBuffer(GameRenderer::gpu);
	if(buff == NULL) throw RendererException("Failed to acquire command buffer");

	SDL_GPUTexture* swapchainTex;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(buff, GameRenderer::window, &swapchainTex, NULL, NULL)) {
		SDL_SubmitGPUCommandBuffer(buff);
		throw RendererException("Failed to acquire swapchain texture");
	}	

	if (swapchainTex == NULL)
	{
		SDL_SubmitGPUCommandBuffer(buff);
		throw RendererException("Failed to acquire swapchain texture");
	}

	SDL_GPUColorTargetInfo CTI = { 0 };
	CTI.texture = swapchainTex;
	CTI.clear_color = (SDL_FColor){0.0f, 0.5f, 0.8f, 1.0f};
	CTI.load_op = SDL_GPU_LOADOP_CLEAR;
	CTI.store_op = SDL_GPU_STOREOP_STORE;

	
	//GameRenderer::camera->rotateModel(1.0f, 0.8f, 0.0f, 0.6f);
	glm::mat4 cameraMatrix = GameRenderer::camera->getCameraMatrix();
	

	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(buff, &CTI, 1, NULL);	//like photoshop layers

	SDL_BindGPUGraphicsPipeline(render_pass, GameRenderer::getGraphicsPipeline(GameModel::pipelineKey)); 
	SDL_PushGPUVertexUniformData(buff, 0, &cameraMatrix, sizeof(cameraMatrix));
	GameRenderer::model->draw(render_pass);
	// std::string pipelineKey = GameRenderer::isWireframe ? TestQuadShader::wireframeKey : TestQuadShader::pipelineKey;
	// SDL_BindGPUGraphicsPipeline(render_pass, GameRenderer::getGraphicsPipeline(pipelineKey)); 
	// SDL_GPUBufferBinding buffBinding{.buffer = GameRenderer::vertexBuffer->buffer, .offset = 0};
	// SDL_BindGPUVertexBuffers(render_pass, 0, &buffBinding, 1);
	// SDL_GPUBufferBinding buffBinding_index{.buffer = GameRenderer::indexBuffer->buffer, .offset = 0};
	// SDL_BindGPUIndexBuffer(render_pass, &buffBinding_index, SDL_GPU_INDEXELEMENTSIZE_16BIT);
	// SDL_PushGPUVertexUniformData(buff, 0, &cameraMatrix, sizeof(cameraMatrix));
	// SDL_DrawGPUIndexedPrimitives(render_pass, 6, 1, 0, 0, 0);
	// SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);
	


	SDL_EndGPURenderPass(render_pass);
	
	if(!SDL_SubmitGPUCommandBuffer(buff)) throw RendererException("Failed to submit command buffer");
	
}

void GameRenderer::buildGraphicsPipeline(std::string key, SDL_GPUGraphicsPipelineCreateInfo& config){

	if (!GameRenderer::graphicPipelines.contains(key)){
		GameRenderer::graphicPipelines[key] = SDL_CreateGPUGraphicsPipeline(GameRenderer::gpu, &config);
	}else{
		SDL_Log("error: graphics pipeline with key %s exists!", key);
	}

}

SDL_GPUGraphicsPipeline* GameRenderer::getGraphicsPipeline(std::string key){
	SDL_GPUGraphicsPipeline* pipeline = nullptr;
	try{
		pipeline = GameRenderer::graphicPipelines.at(key);
	}catch(std::out_of_range e){
		SDL_Log("%s\nCould not find pipeline with key \'%s\'", e.what(), key);
	}
	return pipeline;
}

void GameRenderer::releasePipelines(){
	for(const auto& [key, pipeline] : GameRenderer::graphicPipelines){
		SDL_Log("Releasing %s" , key.c_str());
		SDL_ReleaseGPUGraphicsPipeline(GameRenderer::gpu, pipeline);
	}
}

void GameRenderer::closeRenderer(){
	GameRenderer::releasePipelines();
	delete GameRenderer::vertexBuffer;
	delete GameRenderer::indexBuffer;
	delete GameRenderer::camera;
	delete GameRenderer::shapeRenderer;
	delete GameRenderer::model;

	SDL_ReleaseWindowFromGPUDevice(GameRenderer::gpu, GameRenderer::window);
	SDL_DestroyWindow(GameRenderer::window);
    SDL_DestroyGPUDevice(GameRenderer::gpu);

}


