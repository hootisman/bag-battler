#include "game.h"

Game::Game(){
    this->isGameRunning = true;
	this->isMouseHeld = false;

	GameRenderer::initRenderer();

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
		GameRenderer::isWireframe = !GameRenderer::isWireframe;
		break;
	case SDLK_W:
		// GameRenderer::camera->moveCamera(SDLK_W, this->deltaTime);
		GameRenderer::camera->movementFlags.set(3);
		break;
	case SDLK_A:
		// GameRenderer::camera->moveCamera(SDLK_A, this->deltaTime);
		GameRenderer::camera->movementFlags.set(2);
		break;
	case SDLK_S:
		// GameRenderer::camera->moveCamera(SDLK_S, this->deltaTime);
		GameRenderer::camera->movementFlags.set(1);
		break;
	case SDLK_D:
		// GameRenderer::camera->moveCamera(SDLK_D, this->deltaTime);
		GameRenderer::camera->movementFlags.set(0);
		break;
	
	default:
		return;
	}
	this->heldKeys.insert(e.key);
	// this->printHeldKeys();
}

void Game::keyUpHandler(SDL_KeyboardEvent& e){
	switch (e.key){
	case SDLK_W:
		GameRenderer::camera->movementFlags.reset(3);
		break;
	case SDLK_A:
		GameRenderer::camera->movementFlags.reset(2);
		break;
	case SDLK_S:
		GameRenderer::camera->movementFlags.reset(1);
		break;
	case SDLK_D:
		GameRenderer::camera->movementFlags.reset(0);
		break;
	
	default:
		return;
	}
	this->heldKeys.erase(e.key);
	// this->printHeldKeys();

}

void Game::mouseButtonHandler(SDL_MouseButtonEvent& e){
	this->isMouseHeld = e.down;
	SDL_SetWindowRelativeMouseMode(GameRenderer::window, e.down);
}

void Game::mouseMotionHandler(SDL_MouseMotionEvent& e){
	if (this->isMouseHeld) GameRenderer::camera->updateDirection(e.xrel, e.yrel);
}

void Game::printHeldKeys(){
	SDL_Log("%d", this->heldKeys.size());

}

void Game::updateCamera(){
	GameRenderer::camera->dynamicMove(this->deltaTime);
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
		GameRenderer::render();
		
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
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
			this->mouseButtonHandler(event.button);
			break;
		case SDL_EVENT_MOUSE_MOTION:
			this->mouseMotionHandler(event.motion);
			break;
		
		default:
			break;
		}
	}
}

Game::~Game(){
	GameRenderer::closeRenderer();
}