#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

/// @brief Simple pinhole camera model defined by its position, viewing direction, focal length, and roll angle.
class Camera {
   public:
    glm::vec3 position;
    glm::vec3 direction;
    float focalLength;
    float roll;

    /// @brief Construct a camera with the given position, viewing direction, and focal length.
    /// @param position
    /// @param direction
    /// @param focalLength
    Camera(glm::vec3 position, glm::vec3 direction, float focalLength)
        : position(position), direction(glm::normalize(direction)), focalLength(focalLength), roll(0) {}
};

#endif