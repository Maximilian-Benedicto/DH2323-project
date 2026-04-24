#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

class Camera {
   public:
    glm::vec3 position;

    glm::vec3 direction;

    float focalLength;

    float roll;

    Camera(glm::vec3 position, glm::vec3 direction, float focalLength)
        : position(position), direction(glm::normalize(direction)), focalLength(focalLength), roll(0) {}
};

#endif