#ifndef MODEL_HPP
#define MODEL_HPP

#include <glm/glm.hpp>
#include <vector>

#include "BVH.hpp"
#include "Texture.hpp"
#include "Triangle.hpp"

/// @brief Base class for 3D models.
class Model {
   public:
    /// @brief List of triangles in the model.
    std::vector<Triangle> triangles;

    /// @brief List of textures used by the model.
    std::vector<Texture> textures;

    /// @brief Bounding Volume Hierarchy for accelerating ray-triangle intersection tests.
    BVH bvh;

    /// @brief Scale factor to apply to the model after loading.
    glm::vec3 scale = glm::vec3(1.0f);

    /// @brief Center the model at the origin and resize by provided scale factor
    void scaleAndCenter();

    /// @brief Construct a Model with a given scale factor.
    /// @param scale Scale factor to apply to the model after loading.
    Model(glm::vec3 scale) : scale(scale) {}

    virtual ~Model() = default;
    virtual void load() = 0;
};

#endif
