#ifndef MODEL_H
#define MODEL_H

#include "debug.h"
// #include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct GameBuffer;
struct aiScene;
struct aiMesh;
struct aiNode;

class GameModel{
public:
    struct Vertex{
        glm::vec3 vertices;
        glm::vec3 normals;
        glm::vec2 texCoords;
    };

    struct Mesh{
        std::vector<Vertex> vertices;
        std::vector<Uint32> indices;
        // const TestQuadShader& shader;

        // Mesh(const TestQuadShader& _shader) : shader(_shader) {}
        Mesh() {}
        void draw();
    };
    inline static const std::string pipelineKey = "MODEL";
    std::vector<Mesh> meshes;

    //todo: use GameRenderer allocated buffers
    GameBuffer* vertexBuffer;
    GameBuffer* indexBuffer;
    Uint32 vertexBSize;
    Uint32 indexBSize;

    GameModel() {}
    GameModel(
        std::string path
    )
    { 
        importModel(path); 
    }
    void importModel(std::string);
    void draw(SDL_GPURenderPass*);
    ~GameModel();
private:
    //todo: init graphics pipelines a better way
    void initPipelines();
    void uploadToBuffers();
    void loadNode(aiNode*, const aiScene*);
    Mesh loadMesh(aiMesh*, const aiScene*);
};


#endif