#include "game.h"

Game::Game(){
    this->isGameRunning = true;
    this->renderer = new GameRenderer();
}

void Game::keyDownHandler(SDL_KeyboardEvent& e){
	//printf("%d\n", static_cast<int>(e.key));
	switch (e.key){
	case SDLK_Q:
        this->isGameRunning = false;
		break;
	case SDLK_P:
		this->renderer->isWireframe = !this->renderer->isWireframe;
		break;
	case SDLK_W:
		this->renderer->camera->moveCamera(0.0f, 0.0f, 0.1f);
		break;
	case SDLK_A:
		this->renderer->camera->moveCamera(0.1f, 0.0f, 0.0f);
		break;
	case SDLK_S:
		this->renderer->camera->moveCamera(0.0f, 0.0f, -0.1f);
		break;
	case SDLK_D:
		this->renderer->camera->moveCamera(-0.1f, 0.0f, 0.0f);
		break;
	
	default:
		break;
	}
}

void Game::gameLoop(){
    while (this->isGameRunning)
	{
		this->eventLoop();
		this->renderer->render();
		
	}
}

void Game::eventLoop(){
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		switch (event.type){
		case SDL_EVENT_QUIT:
            this->isGameRunning = false;
			break;
		case SDL_EVENT_KEY_DOWN:
			this->keyDownHandler(event.key);
			break;
		
		default:
			break;
		}
	}
}

Game::~Game(){
    delete this->renderer;
}