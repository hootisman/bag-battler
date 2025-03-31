#ifndef UTIL_H
#define UTIL_H
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_filesystem.h>
#include <string>

namespace GameUtil{
    const int SCREEN_W = 640;
    const int SCREEN_H = 480;
    const std::string SHADER_RESOURCE_PATH = "resources/shaders/";
    const std::string MODEL_RESOURCE_PATH = "resources/models/";

    std::string buildFilePath(const std::string, std::string);
}

#endif