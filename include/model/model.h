#ifndef MODEL_H
#define MODEL_H

#include "debug.h"
#include <SDL3/SDL_stdinc.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <string>
#include <vector>

class GameShader;


class GameModel{
public:
    struct Vertex{
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };

    struct Mesh{
        std::vector<Vertex> vertices;
        std::vector<Uint32> indices;
        const GameShader& shader;

        Mesh(const GameShader& _shader) : shader(_shader) {}
        void draw();
    };

    std::vector<Mesh> meshes;

    GameModel(std::string path) { importModel(path); }
    void importModel(std::string);
    void draw();
private:
} GameModel;


#endif