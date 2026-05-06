#ifndef MODEL_HPP
#define MODEL_HPP

#include <glm/glm.hpp>
#include <vector>

#include "BVH.hpp"
#include "Texture.hpp"
#include "Triangle.hpp"

/// @brief Abstract base class for 3D models.
class Model {
   public:
    std::vector<Triangle> triangles;
    std::vector<Texture> textures;
    BVH bvh;
    glm::vec3 scale = glm::vec3(1.0f);

    Model(glm::vec3 scale) : scale(scale) {}
    virtual ~Model() = default;

    virtual void load() = 0;
};

#endif
