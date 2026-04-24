#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "glm/glm.hpp"

class Light {
   public:
    glm::vec3 position;

    glm::vec3 color;

    Light(glm::vec3 position, glm::vec3 color) : position(position), color(color) {}
};

#endif