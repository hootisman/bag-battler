#include "graphics.h"
#include "shape.h"

/* GameBuffer */
GameBuffer::GameBuffer(SDL_GPUDevice* gpu, SDL_GPUBufferUsageFlags usage, Uint32 size) {
	this->info = { .usage = usage, .size = size};
	this->gpu = gpu;
	this->buffer = SDL_CreateGPUBuffer(this->gpu, &this->info);
	if(this->buffer == NULL) throw RendererException("Failed to create game buffer");
}

int GameBuffer::getSize(){
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

int GameTransferBuffer::getSize(){
	return this->info.size;
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
SDL_GPUShader* GameRenderer::initShader(
	const char* fileName,
	SDL_GPUDevice* gpu, SDL_GPUShaderStage stage, 
	Uint32 sampler, Uint32 uniformBuffer, 
	Uint32 storageBuffer, Uint32 storageTexture
){
	const char* entryPoint = "main";	//todo entrypoint and format different per system
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_SPIRV;
	const char* basePath = SDL_GetBasePath();
	char fullPath[256];
	size_t fileSize;
	void* file;

	/* io stuff */
	SDL_snprintf(fullPath, sizeof(fullPath), "%s%s%s", basePath, SHADER_RESOURCE_PATH, fileName);
	SDL_Log("%s\n", fullPath);

	file = SDL_LoadFile(fullPath, &fileSize);
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

GameRenderer::GameRenderer(){

	/* Create SDL */
	if(!SDL_Init(SDL_INIT_VIDEO)) throw RendererException("Failed to init SDL");

	/* Create Window*/
	this->window = SDL_CreateWindow( "hi", SCREEN_W, SCREEN_H, 0);
	if(!this->window) throw RendererException("Error creating window");

	/* Create GPU Device*/
	this->gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
	if(!this->gpu) throw RendererException("Error creating gpu device");

	//bind gpu to window
	if(!SDL_ClaimWindowForGPUDevice(this->gpu, this->window)) throw RendererException("Error binding gpu to window");

	this->shapeRenderer = new ShapeRenderer(this);

	/* Buffers */

	this->vertexBuffer = new GameBuffer(this->gpu, SDL_GPU_BUFFERUSAGE_VERTEX, sizeof(PosColorVertex) * 4);
	this->indexBuffer = new GameBuffer(this->gpu, SDL_GPU_BUFFERUSAGE_INDEX, sizeof(Uint16) * 6);
	

	Uint32 transferSize = this->vertexBuffer->getSize() + this->indexBuffer->getSize();
	GameTransferBuffer transferBuffer = GameTransferBuffer(this->gpu, SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, transferSize);
	
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

	transferBuffer.loadTransferBuffer(transferData, transferSize);
	SDL_free(transferData);

	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(this->gpu);
	if(uploadCmdBuf == NULL) throw RendererException("Failed to acquire command buffer");

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);


	SDL_GPUTransferBufferLocation transferLoc{.transfer_buffer = transferBuffer.buffer, .offset = 0};
	SDL_GPUBufferRegion vertexLoc{.buffer = this->vertexBuffer->buffer, .offset = 0, .size = this->vertexBuffer->getSize()};
	SDL_GPUTransferBufferLocation transferLoc_index{.transfer_buffer = transferBuffer.buffer, .offset = this->vertexBuffer->getSize()};
	SDL_GPUBufferRegion indexLoc{.buffer = this->indexBuffer->buffer, .offset = 0, .size = this->indexBuffer->getSize()};
	SDL_UploadToGPUBuffer(copyPass, &transferLoc, &vertexLoc, false);
	SDL_UploadToGPUBuffer(copyPass, &transferLoc_index, &indexLoc, false);
	
	SDL_EndGPUCopyPass(copyPass);
	if(!SDL_SubmitGPUCommandBuffer(uploadCmdBuf)) throw RendererException("Failed to submit command buffer");
	

	
	SDL_Log("%s\n", SDL_GetGPUDeviceDriver(this->gpu));

	this->camera = new GameCamera();
	this->isWireframe = false;
}

void GameRenderer::render(){

	SDL_GPUCommandBuffer* buff = SDL_AcquireGPUCommandBuffer(this->gpu);
	if(buff == NULL) throw RendererException("Failed to acquire command buffer");

	SDL_GPUTexture* swapchainTex;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(buff, this->window, &swapchainTex, NULL, NULL)) {
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

	
	//this->camera->rotateModel(1.0f, 0.8f, 0.0f, 0.6f);
	glm::mat4 cameraMatrix = this->camera->getCameraMatrix();
	

	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(buff, &CTI, 1, NULL);	//like photoshop layers

	std::string pipelineKey = this->isWireframe ? ShapeRenderer::wireframeKey : ShapeRenderer::pipelineKey;

	SDL_BindGPUGraphicsPipeline(render_pass, this->getGraphicsPipeline(pipelineKey)); 
	SDL_GPUBufferBinding buffBinding{.buffer = this->vertexBuffer->buffer, .offset = 0};
	SDL_BindGPUVertexBuffers(render_pass, 0, &buffBinding, 1);

	SDL_GPUBufferBinding buffBinding_index{.buffer = this->indexBuffer->buffer, .offset = 0};
	SDL_BindGPUIndexBuffer(render_pass, &buffBinding_index, SDL_GPU_INDEXELEMENTSIZE_16BIT);
	SDL_PushGPUVertexUniformData(buff, 0, &cameraMatrix, sizeof(cameraMatrix));

	SDL_DrawGPUIndexedPrimitives(render_pass, 6, 1, 0, 0, 0);
	// SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);
	
	SDL_EndGPURenderPass(render_pass);
	
	if(!SDL_SubmitGPUCommandBuffer(buff)) throw RendererException("Failed to submit command buffer");
	
}

void GameRenderer::buildGraphicsPipeline(std::string key, SDL_GPUGraphicsPipelineCreateInfo& config){

	if (!this->graphicPipelines.contains(key)){
		this->graphicPipelines[key] = SDL_CreateGPUGraphicsPipeline(this->gpu, &config);
	}else{
		SDL_Log("error: graphics pipeline with key %s exists!", key);
	}

}

SDL_GPUGraphicsPipeline* GameRenderer::getGraphicsPipeline(std::string key){
	SDL_GPUGraphicsPipeline* pipeline = nullptr;
	try{
		pipeline = this->graphicPipelines.at(key);
	}catch(std::out_of_range e){
		SDL_Log("%s\nCould not find pipeline with key \'%s\'", e.what(), key);
	}
	return pipeline;
}

void GameRenderer::releasePipelines(){
	for(const auto& [key, pipeline] : this->graphicPipelines){
		SDL_Log("Releasing %s" , key.c_str());
		SDL_ReleaseGPUGraphicsPipeline(this->gpu, pipeline);
	}
}

GameRenderer::~GameRenderer(){
	this->releasePipelines();
	delete this->vertexBuffer;
	delete this->indexBuffer;
	delete this->camera;
	delete this->shapeRenderer;

	SDL_ReleaseWindowFromGPUDevice(this->gpu, this->window);
	SDL_DestroyWindow(this->window);
    SDL_DestroyGPUDevice(this->gpu);

}


