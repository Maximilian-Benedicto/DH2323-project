#ifndef MODEL_H
#define MODEL_H

#include <glm/glm.hpp>
#include <vector>

// Used to describe a triangular surface:
class Triangle
{
public:
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;
    glm::vec3 normal;
    glm::vec3 color;

    Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 color)
        : v0(v0), v1(v1), v2(v2), color(color)
    {
        ComputeNormal();
    }

    void ComputeNormal()
    {
        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;
        normal = glm::normalize(glm::cross(e2, e1));
    }
};

void Load(std::vector<Triangle> &triangles);

#endif