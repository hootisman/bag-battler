#include "camera.h"



GameCamera::GameCamera(){
    /* Camera Speed*/
    this->speed = 0.05f;

    /* Camera position */
    this->pos = glm::vec3(0.0f, 0.0f, 3.0f);

    /* Camera right vector */
    this->dir = glm::normalize(this->pos - glm::vec3(0, 0, 0));
    this->right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), this->dir));

    /* Camera up vector*/
    this->up = glm::cross(this->dir, this->right);
    this->front = glm::vec3(0.0f, 0.0f, -1.0f);

    /* Init matrices */
    this->model = glm::mat4(1.0f);
    this->view = glm::mat4(1.0f);
    this->proj = glm::mat4(1.0f);

    this->model = glm::rotate(this->model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    this->view = glm::lookAt(this->pos, this->pos + this->front, this->up);

    this->proj = glm::perspective(glm::radians(45.0f), (float)640 / (float)480, 0.1f, 100.0f);
}

void GameCamera::rotateModel(float x, float y, float z, float angle){
    this->model = glm::rotate(this->model, glm::radians(angle), glm::vec3(x, y, z));
}

void GameCamera::updateSpeed(float deltaTime){
    this->speed = 0.02f * deltaTime;
}

void GameCamera::moveCamera(float x, float y, float z){
    this->view = glm::translate(this->view, glm::vec3(x, y, z));
}

void GameCamera::moveCamera(Uint32 key, float deltaTime){
    glm::vec3 velocity;
    this->updateSpeed(deltaTime);

    switch (key)
    {
    case SDLK_W:
        velocity = this->speed * this->front;
        break;
    case SDLK_S:
        velocity = -this->speed * this->front;
        break;
    case SDLK_A:
        velocity = -this->speed * glm::normalize(glm::cross(this->front, this->up));
        break;
    case SDLK_D:
        velocity = this->speed * glm::normalize(glm::cross(this->front, this->up));
        break;
    
    default:
        return;
    }

        this->pos += velocity;
        this->updateView();
}

glm::mat4 GameCamera::getCameraMatrix(){
    return this->proj * this->view * this->model;
}

void GameCamera::updateView(){
    this->view = glm::lookAt(this->pos, this->pos + this->front, this->up);
}

GameCamera::~GameCamera() {}

