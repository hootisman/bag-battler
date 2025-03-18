#include "graphics.h"


SDL_GPUShader* GameShader::initShader(
	SDL_GPUDevice* gpu, SDL_GPUShaderStage stage, 
	const char* filePath, Uint32 sampler, 
	Uint32 uniformBuffer, Uint32 storageBuffer, 
	Uint32 storageTexture
){

	const char* entryPoint = "main";
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_SPIRV;

	size_t fileSize;
	void* file = SDL_LoadFile(filePath, &fileSize);
	if(file == NULL) throw RendererException("Error loading file");

	SDL_GPUShaderCreateInfo info = {};

	info.code_size = fileSize,
	info.code = (const Uint8*)file,
	info.entrypoint = entryPoint,
	info.format = format,
	info.stage = stage,
	info.num_samplers = sampler,
	info.num_storage_textures = storageTexture,
	info.num_storage_buffers = storageBuffer,
	info.num_uniform_buffers = uniformBuffer,
	info.props = 0;
	SDL_GPUShader* shader = SDL_CreateGPUShader(gpu, &info);
	if (shader == NULL) {SDL_free(file); throw RendererException("Error creating shader from file");}
	
	SDL_free(file);
	return shader;

}


    void GameShader::configurePipelineInfo(
		SDL_GPUGraphicsPipelineCreateInfo* pipelineInfo,
        std::span<SDL_GPUVertexBufferDescription> vertexDesc,
        std::span<SDL_GPUVertexAttribute> vertexAttr,
        std::span<SDL_GPUColorTargetDescription> colorTarget,
        SDL_GPUShader* vertShader,
        SDL_GPUShader* fragShader
	){
		// SDL_GPUVertexInputState vertexInput = {

		(*pipelineInfo) = {
			.vertex_shader = vertShader,
			.fragment_shader = fragShader,
			// .vertex_input_state = vertexInput,
			.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			.target_info = {.color_target_descriptions = colorTarget.data(), .num_color_targets = (Uint32)colorTarget.size()}
		};

		pipelineInfo->vertex_input_state = {
			.vertex_buffer_descriptions = vertexDesc.data(),
			.num_vertex_buffers = (Uint32)vertexDesc.size(),
			.vertex_attributes = vertexAttr.data(),
			.num_vertex_attributes = (Uint32)vertexAttr.size()
		};
	}

GameRenderer::GameRenderer(){
	this->isWireframe = false;

	if(!SDL_Init(SDL_INIT_VIDEO)) throw RendererException("Failed to init SDL");

	this->lastTime = SDL_GetTicks();
	//window
	this->window = SDL_CreateWindow( "hi", SCREEN_W, SCREEN_H, 0);
	if(!this->window) throw RendererException("Error creating window");


	//gpu
	this->gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
	if(!this->gpu) throw RendererException("Error creating gpu device");

	//bind gpu to window
	if(!SDL_ClaimWindowForGPUDevice(this->gpu, this->window)) throw RendererException("Error binding gpu to window");

	const char* basePath = SDL_GetBasePath();
	char fullPath[256];

	SDL_snprintf(fullPath, sizeof(fullPath), "%sresources/shaders/%s", basePath, "TriangleBuffer.vert.spv");
	SDL_Log("%s\n", fullPath);
	SDL_GPUShader* vertShader = GameShader::initShader(this->gpu, SDL_GPU_SHADERSTAGE_VERTEX, fullPath, 0, 1, 0, 0);
	
	SDL_snprintf(fullPath, sizeof(fullPath), "%sresources/shaders/%s", basePath, "SolidColor.frag.spv");
	SDL_Log("%s\n", fullPath);
	SDL_GPUShader* fragShader = GameShader::initShader(this->gpu, SDL_GPU_SHADERSTAGE_FRAGMENT, fullPath, 0, 0, 0, 0);



	SDL_GPUVertexAttribute vertexAttr[] = {{0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0}, {1, 0, SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM, sizeof(float) * 3}};
	SDL_GPUVertexBufferDescription vertexDesc[] = {0, sizeof(PosColorVertex), SDL_GPU_VERTEXINPUTRATE_VERTEX, 0};
	SDL_GPUColorTargetDescription colorTargetDesc[] = {SDL_GetGPUSwapchainTextureFormat(this->gpu, this->window)};
	SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {};

	GameShader::configurePipelineInfo(
		&pipelineInfo,
		vertexDesc,
		vertexAttr,
		colorTargetDesc,
		vertShader,
		fragShader
	);

	pipelineInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	this->fillPipeline = SDL_CreateGPUGraphicsPipeline(this->gpu, &pipelineInfo);
	if (fillPipeline == NULL) throw RendererException("Failed to create Fill Pipeline");

	pipelineInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;
	this->linePipeline = SDL_CreateGPUGraphicsPipeline(this->gpu, &pipelineInfo);
	if (linePipeline == NULL) throw RendererException("Failed to create line pipeline");

	SDL_ReleaseGPUShader(this->gpu, vertShader);
	SDL_ReleaseGPUShader(this->gpu, fragShader);

	Uint32 vertexBSize = sizeof(PosColorVertex) * 4;
	Uint32 indexBSize = sizeof(Uint16) * 6;

	/* Vertex Buffer */
	SDL_GPUBufferCreateInfo vertexBufferInfo{.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = vertexBSize};
	this->vertexBuffer = SDL_CreateGPUBuffer(this->gpu, &vertexBufferInfo);
	if(this->vertexBuffer == NULL) throw RendererException("Failed to create vertex buffer");

	SDL_GPUBufferCreateInfo indexBufferInfo{.usage = SDL_GPU_BUFFERUSAGE_INDEX, .size = indexBSize};
	this->indexBuffer = SDL_CreateGPUBuffer(this->gpu, &indexBufferInfo);
	if(this->indexBuffer == NULL) throw RendererException("Failed to create index buffer");

	SDL_GPUTransferBufferCreateInfo transferBufferInfo{.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = vertexBSize + indexBSize};
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(this->gpu, &transferBufferInfo);
	if(transferBuffer == NULL) throw RendererException("Failed to create transfer buffer");

	PosColorVertex* transferData = (PosColorVertex*)SDL_MapGPUTransferBuffer(this->gpu, transferBuffer, false);
	if(transferData == NULL) throw RendererException("Failed to map transfer buffer to gpu");


	transferData[0] = {-0.8,-0.8,0,255,0,0,255};	//bot left
	transferData[1] = {0.8,-0.8,0,0,255,0,255};		//bot right
	transferData[2] = {-0.8,0.8,0,0,0,255,255};		//top left
	transferData[3] = {0.8,0.8,0,0,0,255,255};		//top right

	Uint16* indexData = (Uint16*) &(transferData[4]);
	indexData[0] = 1;
	indexData[1] = 0;
	indexData[2] = 2;
	indexData[3] = 2;
	indexData[4] = 3;
	indexData[5] = 1;

	SDL_UnmapGPUTransferBuffer(this->gpu, transferBuffer);

	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(this->gpu);
	if(uploadCmdBuf == NULL) throw RendererException("Failed to acquire command buffer");

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);


	SDL_GPUTransferBufferLocation transferLoc{.transfer_buffer = transferBuffer, .offset = 0};
	SDL_GPUBufferRegion vertexLoc{.buffer = this->vertexBuffer, .offset = 0, .size = vertexBSize};
	SDL_GPUTransferBufferLocation transferLoc_index{.transfer_buffer = transferBuffer, .offset = vertexBSize};
	SDL_GPUBufferRegion indexLoc{.buffer = this->indexBuffer, .offset = 0, .size = indexBSize};
	SDL_UploadToGPUBuffer(copyPass, &transferLoc, &vertexLoc, false);
	SDL_UploadToGPUBuffer(copyPass, &transferLoc_index, &indexLoc, false);
	
	SDL_EndGPUCopyPass(copyPass);
	if(!SDL_SubmitGPUCommandBuffer(uploadCmdBuf)) throw RendererException("Failed to submit command buffer");
	

	SDL_ReleaseGPUTransferBuffer(this->gpu, transferBuffer);
	
	SDL_Log("%s\n", SDL_GetGPUDeviceDriver(this->gpu));

}

void GameRenderer::render(){

	float currentTime = (float)(SDL_GetTicks() - this->lastTime);
	if (currentTime > 6000)
	{
		this->lastTime = SDL_GetTicks();
	}
	
	int calcTime = ((int)currentTime) / 100;
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
	CTI.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f};
	CTI.load_op = SDL_GPU_LOADOP_CLEAR;
	CTI.store_op = SDL_GPU_STOREOP_STORE;


	TempGarbo toSend = { (SDL_sin(calcTime) + 1.0f)/ 2.0f, 0, 0, 0};
	// SDL_Log("%d\n", calcTime);

	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(buff, &CTI, 1, NULL);	//like photoshop layers

	SDL_BindGPUGraphicsPipeline(render_pass, this->isWireframe ? this->linePipeline : this->fillPipeline); 
	SDL_GPUBufferBinding buffBinding{.buffer = this->vertexBuffer, .offset = 0};
	SDL_BindGPUVertexBuffers(render_pass, 0, &buffBinding, 1);

	SDL_GPUBufferBinding buffBinding_index{.buffer = this->indexBuffer, .offset = 0};
	SDL_BindGPUIndexBuffer(render_pass, &buffBinding_index, SDL_GPU_INDEXELEMENTSIZE_16BIT);
	SDL_PushGPUVertexUniformData(buff, 0, &toSend, sizeof(toSend));

	SDL_DrawGPUIndexedPrimitives(render_pass, 6, 1, 0, 0, 0);
	// SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);
	

	SDL_EndGPURenderPass(render_pass);

	
	
	
	if(!SDL_SubmitGPUCommandBuffer(buff)) throw RendererException("Failed to submit command buffer");
	
}

GameRenderer::~GameRenderer(){
	SDL_ReleaseGPUGraphicsPipeline(this->gpu, fillPipeline);
	SDL_ReleaseGPUGraphicsPipeline(this->gpu, linePipeline);
	SDL_ReleaseWindowFromGPUDevice(this->gpu, this->window);
	SDL_ReleaseGPUBuffer(this->gpu, this->vertexBuffer);
	SDL_ReleaseGPUBuffer(this->gpu, this->indexBuffer);
	SDL_DestroyWindow(this->window);
    SDL_DestroyGPUDevice(this->gpu);
}