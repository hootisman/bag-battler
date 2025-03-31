#include "util.h"

std::string GameUtil::buildFilePath(const std::string resourcePath, std::string fileName){
    const char* basePath = SDL_GetBasePath();
    char fullPath[256];

    SDL_snprintf(fullPath, sizeof(fullPath), "%s%s%s", basePath, resourcePath.c_str(), fileName.c_str());
    return fullPath;
}