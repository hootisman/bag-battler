#ifndef GAME_H
#define GAME_H
#include "render/graphics.h"


class Game{
public:
    GameRenderer* renderer;
    bool isGameRunning;

    Game();
    void keyDownHandler(SDL_KeyboardEvent&);
    void gameLoop();
    void eventLoop();
    ~Game();
};

#endif