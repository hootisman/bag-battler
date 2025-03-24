#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL3/SDL.h>


class GameCamera{
public:
    glm::vec3 pos, dir, right, up, front;
    float speed;


    GameCamera();
    void moveCamera(float, float, float);
    void moveCamera(Uint32, float);
    void updateSpeed(float);
    void rotateModel(float, float, float, float);
    glm::mat4 getCameraMatrix();
    ~GameCamera();
private:
    glm::mat4 model, view, proj;

    void updateView();
};

#endif