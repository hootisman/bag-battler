#ifndef GAMEDEBUG_H
#define GAMEDEBUG_H

#include <stdexcept>
#include <SDL3/SDL_error.h>

class RendererException : public std::runtime_error {
public:
    RendererException(const std::string& message) : std::runtime_error(message + "\nSDL Error: " + SDL_GetError()){}
};


#endif