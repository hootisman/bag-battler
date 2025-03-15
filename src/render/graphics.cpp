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
	if (shader == NULL)
	{
		SDL_Log("Error! %s", SDL_GetError());
		SDL_free(file);
		return NULL;
	}
	
	SDL_free(file);
	return shader;

}


GameRenderer::GameRenderer(){
    try
	{
		if(!SDL_Init(SDL_INIT_VIDEO)) throw 2;

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

		SDL_snprintf(fullPath, sizeof(fullPath), "%sresources/shaders/%s", basePath, "RawTriangle.vert.spv");
		SDL_Log("%s\n", fullPath);
		SDL_GPUShader* vertShader = GameShader::initShader(this->gpu, SDL_GPU_SHADERSTAGE_VERTEX, fullPath, 0, 0, 0, 0);
		
		SDL_snprintf(fullPath, sizeof(fullPath), "%sresources/shaders/%s", basePath, "SolidColor.frag.spv");
		SDL_Log("%s\n", fullPath);
		SDL_GPUShader* fragShader = GameShader::initShader(this->gpu, SDL_GPU_SHADERSTAGE_FRAGMENT, fullPath, 0, 0, 0, 0);


		SDL_GPUColorTargetDescription colorTargetDesc[] = {{
			.format = SDL_GetGPUSwapchainTextureFormat(this->gpu, this->window)
		}};

		SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {
			.vertex_shader = vertShader,
			.fragment_shader = fragShader,
			.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			.target_info = {
				// .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
				// 	.format = SDL_GetGPUSwapchainTextureFormat(this->gpu, this->window)
				// }},
				.color_target_descriptions = colorTargetDesc,
				.num_color_targets = 1,
			},
		};

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
	}
	catch(int e)	//todo convert to exceptions
	{
		//todo make the codes actual enums for readability
		SDL_Log("Error occured in GameRenderer::init(), code %d; %s", e, SDL_GetError());
	}

}

void GameRenderer::render(){
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
	CTI.clear_color = (SDL_FColor){1.0f, 0.0f, 0.0f, 1.0f};
	CTI.load_op = SDL_GPU_LOADOP_CLEAR;
	CTI.store_op = SDL_GPU_STOREOP_STORE;

	
	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(buff, &CTI, 1, NULL);	//like photoshop layers
	// SDL_BindGPUGraphicsPipeline(render_pass, this->fillPipeline);
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
	SDL_DestroyWindow(this->window);
    SDL_DestroyGPUDevice(this->gpu);
}