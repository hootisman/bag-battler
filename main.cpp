#include <SDL3/SDL.h>
#include <stdio.h>


const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;
SDL_GLContext* context = NULL;
bool gameRunning;

int init();
int eventLoop();
int gameLoop();
int keyDownHandler(SDL_KeyboardEvent&);
void closeGame();
void freeGraphics();


int init(){
	try
	{
		if(!SDL_Init(SDL_INIT_VIDEO)) throw 2;
		window = SDL_CreateWindow( "SDL Tutorial", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
		if( window == NULL ) throw 1;
		gameRunning = true;

	}
	catch(int e)
	{
		//todo make the codes actual enums for readability
		printf("Exception occured in init(), code %d; %s", e, SDL_GetError());
	}
	return 0;
}


int eventLoop(){
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		switch (event.type){
		case SDL_EVENT_QUIT:
			closeGame();
			break;
		case SDL_EVENT_KEY_DOWN:
			keyDownHandler(event.key);
			break;
		default:
			break;
		}
	}
	return 0;
}

int gameLoop(){
	while (gameRunning)
	{
		eventLoop();
		
		screenSurface = SDL_GetWindowSurface( window );

		SDL_FillSurfaceRect( screenSurface, NULL, SDL_MapSurfaceRGB( screenSurface, 0xFF, 0x8F, 0xFF ) );

		SDL_UpdateWindowSurface( window );
	}
	
	return 0;
}

int keyDownHandler(SDL_KeyboardEvent& e){
	//printf("%d\n", static_cast<int>(e.key));
	switch (e.key){
	case SDLK_Q:
		closeGame();
		break;
	
	default:
		break;
	}
	return 0;
}

void closeGame(){
	gameRunning = false;
}

void freeGraphics(){
	SDL_DestroyWindow( window );
	SDL_Quit();
}

int main( int argc, char* args[] )
{
	init();

	gameLoop();

	freeGraphics();
	return 0;
}
