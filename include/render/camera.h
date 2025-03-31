#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL3/SDL.h>
#include <bitset>


class GameCamera{
public:
    glm::vec3 pos, right, up, front;
    std::bitset<4> movementFlags;
    float speed;
    float yaw, pitch;


    GameCamera();
    void moveCamera(float, float, float);
    void moveCamera(Uint32, float);
    void dynamicMove(float);    //bitflag param
    void updateSpeed(float);
    void updateDirection(float, float);
    void rotateModel(float, float, float, float);
    glm::mat4 getCameraMatrix();
    ~GameCamera();
private:
    glm::mat4 model, view, proj;

    void updateView();
};

#endif