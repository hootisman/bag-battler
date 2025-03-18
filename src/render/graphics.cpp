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
	if(file == NULL){
		SDL_Log("Error! %s", SDL_GetError());
		return NULL;
	}


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
	if (shader == NULL)
	{
		SDL_Log("Error! %s", SDL_GetError());
		SDL_free(file);
		return NULL;
	}
	
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
		SDL_GPUVertexInputState vertexInput = {
			.vertex_buffer_descriptions = vertexDesc.data(),
			.num_vertex_buffers = vertexDesc.size(),
			.vertex_attributes = vertexAttr.data(),
			.num_vertex_attributes = vertexAttr.size()
		};

		(*pipelineInfo) = {
			.vertex_shader = vertShader,
			.fragment_shader = fragShader,
			.vertex_input_state = vertexInput,
			.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			.target_info = {.color_target_descriptions = colorTarget.data(), .num_color_targets = (Uint32)colorTarget.size()}
		};
	}

GameRenderer::GameRenderer(){
	this->isWireframe = false;
    try
	{
		if(!SDL_Init(SDL_INIT_VIDEO)) throw 2;

		this->lastTime = SDL_GetTicks();
		//window
		this->window = SDL_CreateWindow( "hi", SCREEN_W, SCREEN_H, 0);
		if(!this->window) throw 1;


		//gpu
		this->gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
		if(!this->gpu) throw 2;

		//bind gpu to window
		if(!SDL_ClaimWindowForGPUDevice(this->gpu, this->window)) throw 3;

		const char* basePath = SDL_GetBasePath();
		char fullPath[256];

		SDL_snprintf(fullPath, sizeof(fullPath), "%sresources/shaders/%s", basePath, "TriangleBuffer.vert.spv");
		SDL_Log("%s\n", fullPath);
		SDL_GPUShader* vertShader = GameShader::initShader(this->gpu, SDL_GPU_SHADERSTAGE_VERTEX, fullPath, 0, 1, 0, 0);
		
		SDL_snprintf(fullPath, sizeof(fullPath), "%sresources/shaders/%s", basePath, "SolidColor.frag.spv");
		SDL_Log("%s\n", fullPath);
		SDL_GPUShader* fragShader = GameShader::initShader(this->gpu, SDL_GPU_SHADERSTAGE_FRAGMENT, fullPath, 0, 0, 0, 0);



		/****** Pipeline Setup *******/
		/*** 	ALso vertex buffer setup */
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
		if (fillPipeline == NULL)
		{
			SDL_Log("Failed to create fill pipeline!");
			throw 4;
		}
		

		pipelineInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;
		this->linePipeline = SDL_CreateGPUGraphicsPipeline(this->gpu, &pipelineInfo);
		if (linePipeline == NULL)
		{
			SDL_Log("Failed to create line pipeline!");
			throw 4;
		}

		SDL_ReleaseGPUShader(this->gpu, vertShader);
		SDL_ReleaseGPUShader(this->gpu, fragShader);

		Uint32 vertexBSize = sizeof(PosColorVertex) * 4;
		Uint32 indexBSize = sizeof(Uint16) * 6;

		/* Vertex Buffer */
		SDL_GPUBufferCreateInfo vertexBufferInfo{.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = vertexBSize};
		this->vertexBuffer = SDL_CreateGPUBuffer(this->gpu, &vertexBufferInfo);

		SDL_GPUBufferCreateInfo indexBufferInfo{.usage = SDL_GPU_BUFFERUSAGE_INDEX, .size = indexBSize};
		this->indexBuffer = SDL_CreateGPUBuffer(this->gpu, &indexBufferInfo);

		SDL_GPUTransferBufferCreateInfo transferBufferInfo{.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = vertexBSize + indexBSize};
		SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(this->gpu, &transferBufferInfo);

		PosColorVertex* transferData = (PosColorVertex*)SDL_MapGPUTransferBuffer(this->gpu, transferBuffer, false);


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
		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);


		SDL_GPUTransferBufferLocation transferLoc{.transfer_buffer = transferBuffer, .offset = 0};
		SDL_GPUBufferRegion vertexLoc{.buffer = this->vertexBuffer, .offset = 0, .size = vertexBSize};
		SDL_GPUTransferBufferLocation transferLoc_index{.transfer_buffer = transferBuffer, .offset = vertexBSize};
		SDL_GPUBufferRegion indexLoc{.buffer = this->indexBuffer, .offset = 0, .size = indexBSize};
		SDL_UploadToGPUBuffer(copyPass, &transferLoc, &vertexLoc, false);
		SDL_UploadToGPUBuffer(copyPass, &transferLoc_index, &indexLoc, false);
		
		SDL_EndGPUCopyPass(copyPass);
		SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
		SDL_ReleaseGPUTransferBuffer(this->gpu, transferBuffer);
		
		SDL_Log("%s\n", SDL_GetGPUDeviceDriver(this->gpu));
	}
	catch(int e)	//todo convert to exceptions
	{
		//todo make the codes actual enums for readability
		SDL_Log("Error occured in GameRenderer::init(), code %d; %s", e, SDL_GetError());
	}

}

void GameRenderer::render(){

	float currentTime = (float)(SDL_GetTicks() - this->lastTime);
	if (currentTime > 6000)
	{
		this->lastTime = SDL_GetTicks();
	}
	
	int calcTime = ((int)currentTime) / 100;
	SDL_GPUCommandBuffer* buff = SDL_AcquireGPUCommandBuffer(this->gpu);
	if(buff == NULL){
		SDL_Log("Failed to load command buffer %s\n", SDL_GetError());
		return;
	}

	SDL_GPUTexture* swapchainTex;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(buff, this->window, &swapchainTex, NULL, NULL))
	{
		SDL_Log("Failed to load swapchain texture %s\n", SDL_GetError());
		return;
	}

	if (swapchainTex == NULL)
	{
		SDL_Log("swap chain texture is null! %s\n", SDL_GetError());
		SDL_SubmitGPUCommandBuffer(buff);
		return;
	}

	SDL_GPUColorTargetInfo CTI = { 0 };
	CTI.texture = swapchainTex;
	CTI.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f};
	CTI.load_op = SDL_GPU_LOADOP_CLEAR;
	CTI.store_op = SDL_GPU_STOREOP_STORE;


	TempGarbo toSend = { (SDL_sin(calcTime) + 1.0f)/ 2.0f, 0, 0, 0};
	// SDL_Log("%d\n", calcTime);

	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(buff, &CTI, 1, NULL);	//like photoshop layers

	this->isWireframe ? SDL_BindGPUGraphicsPipeline(render_pass, this->linePipeline) : SDL_BindGPUGraphicsPipeline(render_pass, this->fillPipeline); 
	SDL_GPUBufferBinding buffBinding{.buffer = this->vertexBuffer, .offset = 0};
	SDL_BindGPUVertexBuffers(render_pass, 0, &buffBinding, 1);

	SDL_GPUBufferBinding buffBinding_index{.buffer = this->indexBuffer, .offset = 0};
	SDL_BindGPUIndexBuffer(render_pass, &buffBinding_index, SDL_GPU_INDEXELEMENTSIZE_16BIT);
	SDL_PushGPUVertexUniformData(buff, 0, &toSend, sizeof(toSend));

	SDL_DrawGPUIndexedPrimitives(render_pass, 6, 1, 0, 0, 0);
	// SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);
	

	SDL_EndGPURenderPass(render_pass);

	
	
	
	SDL_SubmitGPUCommandBuffer(buff);
		//this->screenSurface = SDL_GetWindowSurface(this->window);

		// SDL_FillSurfaceRect( screenSurface, NULL, SDL_MapSurfaceRGB(this->screenSurface, 0xFF, 0xFF, 0xFF ) );
		// SDL_UpdateWindowSurface( window );
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