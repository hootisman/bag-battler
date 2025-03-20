#include "camera.h"

GameCamera::GameCamera(){
    this->model = glm::mat4(1.0f);
    this->view = glm::mat4(1.0f);
    this->proj = glm::mat4(1.0f);

    this->model = glm::rotate(this->model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    this->view = glm::translate(this->view, glm::vec3(0.0f, 0.0f, -3.0f));
    this->proj = glm::perspective(glm::radians(45.0f), (float)640 / (float)480, 0.1f, 100.0f);

}

void GameCamera::rotateModel(float x, float y, float z, float angle){
    this->model = glm::rotate(this->model, glm::radians(angle), glm::vec3(x, y, z));
}

void GameCamera::moveCamera(float x, float y, float z){
    this->view = glm::translate(this->view, glm::vec3(x, y, z));
}


glm::mat4 GameCamera::getCameraMatrix(){
    return this->proj * this->view * this->model;
}

GameCamera::~GameCamera() {}

