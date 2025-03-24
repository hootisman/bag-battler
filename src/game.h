#ifndef GAME_H
#define GAME_H
#include "render/graphics.h"


class Game{
public:
    GameRenderer* renderer;
    bool isGameRunning;
    float deltaTime, lastFrame;

    Game();
    void keyDownHandler(SDL_KeyboardEvent&);
    void keyUpHandler(SDL_KeyboardEvent&);
    void updateDeltaTime();
    void gameLoop();
    void eventLoop();
    ~Game();
};

#endif