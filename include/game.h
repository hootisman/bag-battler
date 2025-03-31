#ifndef GAME_H
#define GAME_H
#include "render/graphics.h"
#include <unordered_set>
#include <string>

class Game{
public:
    std::unordered_set<Uint32> heldKeys;    //todo: remove
    
    bool isGameRunning, isMouseHeld;
    float deltaTime, lastFrame;

    Game();
    void mouseButtonHandler(SDL_MouseButtonEvent&);
    void mouseMotionHandler(SDL_MouseMotionEvent&);
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