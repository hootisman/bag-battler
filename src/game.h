#ifndef GAME_H
#define GAME_H
#include "render/graphics.h"
#include <unordered_set>
#include <string>

class Game{
public:
    GameRenderer* renderer;
    std::unordered_set<Uint32> heldKeys;
    
    bool isGameRunning;
    float deltaTime, lastFrame;

    Game();
    void keyDownHandler(SDL_KeyboardEvent&);
    void keyUpHandler(SDL_KeyboardEvent&);
    void updateDeltaTime();
    void updateCamera();
    void printHeldKeys();
    void gameLoop();
    void eventLoop();
    ~Game();
};

#endif