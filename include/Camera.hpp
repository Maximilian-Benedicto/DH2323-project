#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera
{
public:
    glm::vec3 position;
    float focalLength;
    float pitch;
    float roll;
    float yaw;

    Camera(glm::vec3 position, float focalLength) : position(position), focalLength(focalLength), pitch(0), roll(0), yaw(0) {}
};

#endif