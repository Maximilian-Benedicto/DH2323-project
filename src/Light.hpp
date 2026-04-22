#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "glm/glm.hpp"

/**
 * @brief Class representing a point light source in the scene.
 */
class Light {
   public:
    /** Position of the point light in world space. */
    glm::vec3 position;

    /** RGB radiant intensity scale used by shading equations. */
    glm::vec3 color;

    /**
     * @brief Construct a point light.
     * @param position Light position in world space.
     * @param color RGB intensity scale.
     */
    Light(glm::vec3 position, glm::vec3 color) : position(position), color(color) {}
};

#endif