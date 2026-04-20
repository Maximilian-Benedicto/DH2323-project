#ifndef MODEL_H
#define MODEL_H

#include <glm/glm.hpp>
#include <vector>

/**
 * @brief Simple class representing a triangle in 3D space, defined by its three vertices, normal vector, and color.
 */
class Triangle
{
public:
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec3 centroid;

    Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 color)
        : v0(v0), v1(v1), v2(v2), color(color)
    {
        ComputeNormal();
        ComputeCentroid();
    }

    void ComputeNormal()
    {
        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;
        normal = glm::normalize(glm::cross(e2, e1));
    }

    void ComputeCentroid()
    {
        centroid = (v0 + v1 + v2) / 3.0f;
    }
};

#endif