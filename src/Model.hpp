#ifndef MODEL_HPP
#define MODEL_HPP

#include <vector>
#include "Triangle.hpp"
#include "BVH.hpp"

/**
 * @brief Abstract base class for 3D models, which consist of a collection of triangles.
 */
class Model
{
public:
    // List of triangles in the model
    std::vector<Triangle> triangles;

    // List of textures used by the model, indexed by the textureIdx field in Triangle
    std::vector<Texture> textures;

    // Bounding Volume Hierarchy for accelerating ray-triangle intersection tests
    BVH bvh;

    virtual ~Model() = default;

    /**
     * @brief Load the triangles into the vector.
     */
    virtual void Load() = 0;
};

#endif
