#include "game.h"
#include <SDL3/SDL_main.h>


Game* theGame;

void freeGame();

void freeGame(){
	delete theGame;
	SDL_Quit();
}
int main( int argc, char* args[] )
{
	theGame = new Game();

	theGame->gameLoop();
	freeGame();
	return 0;
}
