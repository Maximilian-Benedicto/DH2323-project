#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "glm/glm.hpp"

/// @brief Simple point light source defined by its position and color.
class Light {
   public:
    glm::vec3 position;
    glm::vec3 color;

    /// @brief Construct a light source with the given position and color.
    /// @param position
    /// @param color
    Light(glm::vec3 position, glm::vec3 color)
        : position(position), color(color) {}
};

#endif