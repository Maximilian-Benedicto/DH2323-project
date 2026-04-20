#ifndef MODEL_HPP
#define MODEL_HPP

#include <vector>
#include "Triangle.hpp"

/**
 * @brief Abstract base class for 3D models, which consist of a collection of triangles.
 */
class Model
{
public:
    std::vector<Triangle> triangles;

    virtual ~Model() = default;

    /**
     * @brief Load the triangles into the vector.
     */
    virtual void Load() = 0;
};

#endif
