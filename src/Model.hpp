#ifndef MODEL_HPP
#define MODEL_HPP

#include <vector>
#include "Triangle.hpp"

class Model
{
public:
    std::vector<Triangle> triangles;

    virtual ~Model() = default;

    /**
     * @brief load the triangles into the vector.
     */
    virtual void Load() = 0;
};

#endif
