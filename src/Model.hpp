#ifndef MODEL_HPP
#define MODEL_HPP

#include <vector>
#include "Triangle.hpp"
#include "BVH.hpp"

class Model {
   public:
    std::vector<Triangle> triangles;

    std::vector<Texture> textures;

    BVH bvh;

    virtual ~Model() = default;

    virtual void load() = 0;
};

#endif
