#include "graphics.h"

GameRenderer::GameRenderer(){
    try
	{
		if(!SDL_Init(SDL_INIT_VIDEO)) throw 2;

		window = SDL_CreateWindow( "hi", SCREEN_W, SCREEN_H, 0);
		if( window == NULL ) throw 1;

	}
	catch(int e)
	{
		//todo make the codes actual enums for readability
		SDL_Log("Error occured in GameRenderer::init(), code %d; %s", e, SDL_GetError());
	}

}

void GameRenderer::render(){
		this->screenSurface = SDL_GetWindowSurface(this->window);

		SDL_FillSurfaceRect( screenSurface, NULL, SDL_MapSurfaceRGB(this->screenSurface, 0xFF, 0x8F, 0xFF ) );

		SDL_UpdateWindowSurface( window );
}

GameRenderer::~GameRenderer(){
	SDL_DestroyWindow(this->window);
    //SDL_DestroyGPUDevice(this->gpu);
}