#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

/**
 * @brief Simple camera class that holds the position and orientation of the camera in 3D space.
 */
class Camera {
   public:
    /** Camera origin in world space. */
    glm::vec3 position;

    /** Forward viewing direction in world space (always normalized on construction). */
    glm::vec3 direction;

    /** Distance from camera origin to image plane in pixel units. */
    float focalLength;

    /** Roll angle in radians around the forward axis. */
    float roll;

    /**
     * @brief Construct a camera from position and forward direction.
     * @param position Camera origin in world space.
     * @param direction Forward direction (normalized internally).
     * @param focalLength Image-plane distance in pixel units.
     */
    Camera(glm::vec3 position, glm::vec3 direction, float focalLength)
        : position(position), direction(glm::normalize(direction)), focalLength(focalLength), roll(0) {}
};

#endif