#include "render/camera.h"



GameCamera::GameCamera(){
    this->yaw = -90.0f;
    this->pitch = 0.0f;

    /* Camera Speed*/
    this->speed = 0.05f;

    /* Camera position */
    this->pos = glm::vec3(0.0f, 0.0f, 3.0f);

    /* Camera right vector */
    glm::vec3 direction = glm::normalize(this->pos - glm::vec3(0, 0, 0));
    this->right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction));

    /* Camera up vector*/
    this->up = glm::cross(direction, this->right);
    this->front = glm::vec3(0.0f, 0.0f, -1.0f);

    /* Camera Direction*/

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
    this->speed = 0.004f * deltaTime;
}

void GameCamera::updateDirection(float deltaYaw, float deltaPitch){
    this->yaw += deltaYaw * 0.1f;
    this->pitch += -deltaPitch * 0.1f;

    

    glm::vec3 direction;

    direction.x = SDL_cosf(glm::radians(this->yaw)) * SDL_cosf(glm::radians(this->pitch));
    direction.y = SDL_sinf(glm::radians(this->pitch));
    direction.z = SDL_sinf(glm::radians(this->yaw)) * SDL_cosf(glm::radians(this->pitch));

    this->front = glm::normalize(direction);
    this->updateView();
}

void GameCamera::dynamicMove(float deltaTime){
    if(this->movementFlags.none()) return;
    glm::vec3 velocity = glm::vec3(0.0f);
    this->updateSpeed(deltaTime);

    //why did i do this
    if(this->movementFlags[3]){
        //W key
        velocity += this->speed * this->front;
    }

    if(this->movementFlags[2]){
        //A key
        velocity += -this->speed * glm::normalize(glm::cross(this->front, this->up));
    }

    if(this->movementFlags[1]){
        //S key
        velocity += -this->speed * this->front;
    }

    if(this->movementFlags[0]){
        //D key
        velocity += this->speed * glm::normalize(glm::cross(this->front, this->up));
    }

    this->pos += velocity;
    this->updateView();
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

