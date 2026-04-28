#ifndef MODEL_HPP
#define MODEL_HPP

#include <vector>

#include "BVH.hpp"
#include "Triangle.hpp"

/// @brief Abstract base class for 3D models.
class Model {
   public:
    std::vector<Triangle> triangles;
    std::vector<Texture> textures;
    BVH bvh;

    virtual ~Model() = default;

    virtual void load() = 0;
};

#endif
