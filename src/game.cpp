#include "game.h"

Game::Game(){
    this->isGameRunning = true;
    this->renderer = new GameRenderer();

	this->deltaTime = 0.0f;
	this->lastFrame = 0.0f;
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
		// this->renderer->camera->moveCamera(SDLK_W, this->deltaTime);
		this->renderer->camera->movementFlags.set(3);
		break;
	case SDLK_A:
		// this->renderer->camera->moveCamera(SDLK_A, this->deltaTime);
		this->renderer->camera->movementFlags.set(2);
		break;
	case SDLK_S:
		// this->renderer->camera->moveCamera(SDLK_S, this->deltaTime);
		this->renderer->camera->movementFlags.set(1);
		break;
	case SDLK_D:
		// this->renderer->camera->moveCamera(SDLK_D, this->deltaTime);
		this->renderer->camera->movementFlags.set(0);
		break;
	
	default:
		return;
	}
	this->heldKeys.insert(e.key);
	this->printHeldKeys();
}

void Game::keyUpHandler(SDL_KeyboardEvent& e){
	switch (e.key){
	case SDLK_W:
		this->renderer->camera->movementFlags.reset(3);
		break;
	case SDLK_A:
		this->renderer->camera->movementFlags.reset(2);
		break;
	case SDLK_S:
		this->renderer->camera->movementFlags.reset(1);
		break;
	case SDLK_D:
		this->renderer->camera->movementFlags.reset(0);
		break;
	
	default:
		return;
	}
	this->heldKeys.erase(e.key);
	this->printHeldKeys();

}

void Game::printHeldKeys(){
	SDL_Log("%d", this->heldKeys.size());

}

void Game::updateCamera(){
	this->renderer->camera->dynamicMove(this->deltaTime);
}

void Game::updateDeltaTime(){
	float currentTime = SDL_GetTicks();
	this->deltaTime = currentTime - this->lastFrame;
	this->lastFrame = currentTime;
}

void Game::gameLoop(){
    while (this->isGameRunning)
	{
		this->updateDeltaTime();
		this->updateCamera();
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
		case SDL_EVENT_KEY_UP:
			this->keyUpHandler(event.key);
			break;
		
		default:
			break;
		}
	}
}

Game::~Game(){
    delete this->renderer;
}