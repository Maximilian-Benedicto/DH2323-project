#ifndef MODEL_HPP
#define MODEL_HPP

#include <vector>
#include "Triangle.hpp"
#include "BVH.hpp"

/**
 * @brief Abstract base class for 3D models, which consist of a collection of triangles.
 */
class Model {
   public:
    /** List of triangles in the model. */
    std::vector<Triangle> triangles;

    /**
     * Textures indexed by Triangle::textureIdx.
     * Implementations should preserve this index mapping while loading assets.
     */
    std::vector<Texture> textures;

    /** Bounding Volume Hierarchy used for accelerated intersection tests. */
    BVH bvh;

    virtual ~Model() = default;

    /**
     * @brief Load mesh data and build acceleration structures for this model.
     * @details Implementations populate triangles and optional textures, then rebuild bvh.
     */
    virtual void load() = 0;
};

#endif
