#include "model/model.h"
#include "render/shader.h"
#include "render/graphics.h"
#include "util.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

/* Mesh */
void GameModel::Mesh::draw(){

}

/* GameModel */
void GameModel::importModel(std::string path){
    Assimp::Importer importer;
    std::string fullPath = GameUtil::buildFilePath(GameUtil::MODEL_RESOURCE_PATH, path);
    const aiScene* scene = importer.ReadFile(fullPath, aiProcess_Triangulate);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
        throw RendererException("Assimp importModel error");
    }

    this->indexBSize = 0;
    this->vertexBSize= 0;

    //load meshes
    this->loadNode(scene->mRootNode, scene);

    //todo: remove immediately
    this->initPipelines();

    //init buffers
    //todo: remove probably
    this->vertexBuffer = new GameBuffer(GameRenderer::gpu, SDL_GPU_BUFFERUSAGE_VERTEX, vertexBSize);
    this->indexBuffer = new GameBuffer(GameRenderer::gpu, SDL_GPU_BUFFERUSAGE_INDEX, indexBSize);


    //copy to gpu
    //todo: remove
    this->uploadToBuffers();
}

void GameModel::uploadToBuffers(){

	Uint32 transferSize = this->vertexBSize + this->indexBSize;

    Vertex* vertexData = (Vertex*)SDL_malloc(transferSize);
    Uint32* indexData = (Uint32*) &(vertexData[vertexBSize / sizeof(Vertex)]);

    for(int i = 0; i < this->meshes.size(); i++){
        const Mesh& mesh = this->meshes[i];
        const std::vector<Vertex>& meshVs = mesh.vertices;
        const std::vector<Uint32>& meshIs = mesh.indices;

        SDL_memcpy(&vertexData[i], meshVs.data(), meshVs.size() * sizeof(Vertex));
        SDL_memcpy(&indexData[i], meshIs.data(), meshIs.size() * sizeof(Uint32));
    }

    GameRenderer::uploadVIBuffers(vertexData, this->vertexBuffer, this->indexBuffer);
    SDL_free(vertexData);
}

void GameModel::loadNode(aiNode* node, const aiScene* scene){
    for(int i = 0; i < node->mNumMeshes; i++){
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        this->meshes.push_back(this->loadMesh(mesh, scene));
    }
    for(int i = 0; i < node->mNumChildren; i++){
        this->loadNode(node->mChildren[i], scene);
    }
}

GameModel::Mesh GameModel::loadMesh(aiMesh* mesh, const aiScene* scene){
    GameModel::Mesh output;

    for(int i = 0; i < mesh->mNumVertices; i++){
        GameModel::Vertex vertex;
        vertex.vertices.x = mesh->mVertices[i].x;
        vertex.vertices.y = mesh->mVertices[i].y;
        vertex.vertices.z = mesh->mVertices[i].z;

        vertex.normals.x = mesh->mNormals[i].x;
        vertex.normals.y = mesh->mNormals[i].y;
        vertex.normals.z = mesh->mNormals[i].z;

        //todo: redo this
        vertex.texCoords = mesh->mTextureCoords[0] ? 
        glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) :
        glm::vec2(0.0f, 0.0f);

        output.vertices.push_back(vertex);
    }

    for(int i = 0; i < mesh->mNumFaces; i++){
        aiFace face = mesh->mFaces[i];

        for(int j = 0; j < mesh->mNumFaces; j++){
            output.indices.push_back(face.mIndices[j]);
        }
    }
    // add bytes of mesh vertices to total vertex buffer size; for initing vertex buffer
    //todo: delete whenever I change model vertex buffers
    this->vertexBSize += sizeof(Vertex) * output.vertices.size();

    // add bytes of mesh indices to total index buffer size; for initing index buffer
    this->indexBSize += sizeof(Uint32) * output.indices.size();

    return output;
}

void GameModel::initPipelines(){
	SDL_GPUShader* vertShader = GameRenderer::initShader("ModelTest.vert.spv", GameRenderer::gpu, SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 0, 0);
	SDL_GPUShader* fragShader = GameRenderer::initShader("SolidColor.frag.spv", GameRenderer::gpu, SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 0, 0, 0);
	SDL_GPUVertexAttribute vertexAttr[] = {
        {
            0, 0, 
            SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 
            0
        }, {
            1, 0, 
            SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 
            offsetof(Vertex, normals)
        }, {
            2, 0,
            SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, 
            offsetof(Vertex, texCoords)
        }
    };
	SDL_GPUVertexBufferDescription vertexDesc[] = {0, sizeof(Vertex), SDL_GPU_VERTEXINPUTRATE_VERTEX, 0};
	SDL_GPUColorTargetDescription colorTarget[] = {SDL_GetGPUSwapchainTextureFormat(GameRenderer::gpu, GameRenderer::window)};

    //HACK: usually is a member function, trying to use without doing that
    /**
     * \see TestQuadShader
     */
    auto config = GameRenderer::createPipelineConfig(
        SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, 
        vertexDesc, 
        vertexAttr, 
        colorTarget, 
        vertShader, 
        fragShader
    );

    config.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;

    GameRenderer::buildGraphicsPipeline(GameModel::pipelineKey, config);
	if (GameRenderer::getGraphicsPipeline(GameModel::pipelineKey) == NULL) throw RendererException("Failed to create Fill Pipeline");

	SDL_ReleaseGPUShader(GameRenderer::gpu, vertShader);
	SDL_ReleaseGPUShader(GameRenderer::gpu, fragShader);
}

void GameModel::draw(SDL_GPURenderPass* pass){
	// SDL_BindGPUGraphicsPipeline(pass, GameRenderer::getGraphicsPipeline(GameModel::pipelineKey)); 

    SDL_GPUBufferBinding buffBinding{.buffer = this->vertexBuffer->buffer, .offset = 0};
	SDL_GPUBufferBinding buffBinding_index{.buffer = this->indexBuffer->buffer, .offset = 0};

	SDL_BindGPUVertexBuffers(pass, 0, &buffBinding, 1);
	SDL_BindGPUIndexBuffer(pass, &buffBinding_index, SDL_GPU_INDEXELEMENTSIZE_16BIT);


    Sint32 vertexOffset = 0;
    Uint32 firstIndex = 0;
    for(int i = 0; i < this->meshes.size(); i++){
        const Mesh& mesh = this->meshes[i];
        const std::vector<Vertex>& meshVs = mesh.vertices;
        const std::vector<Uint32>& meshIs = mesh.indices;

	    SDL_DrawGPUIndexedPrimitives(
            pass, 
            meshIs.size(), 
            1, 
            firstIndex ,
            vertexOffset, 
            0
        );

        firstIndex += meshIs.size();
        vertexOffset += meshVs.size() * sizeof(Vertex);
    }


}
GameModel::~GameModel(){
    delete this->vertexBuffer;
    delete this->indexBuffer;
}