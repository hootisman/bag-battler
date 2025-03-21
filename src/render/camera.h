#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class GameCamera{
public:
    glm::mat4 cameraMatrix;

    GameCamera();
    void moveCamera(float, float, float);
    void rotateModel(float, float, float, float);
    glm::mat4 getCameraMatrix();
    ~GameCamera();
private:
    glm::mat4 model, view, proj;
};

#endif