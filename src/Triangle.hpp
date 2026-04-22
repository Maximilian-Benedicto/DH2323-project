#ifndef MODEL_H
#define MODEL_H

#include <glm/glm.hpp>
#include <vector>
#include "Material.hpp"
#include "Texture.hpp"

/**
 * @brief Simple class representing a triangle in 3D space, defined by its three vertices, normal vector, color and centroid.
 */
class Triangle
{
public:
    // Triangle vertices
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;

    // Texture coordinates
    glm::vec2 uv0;
    glm::vec2 uv1;
    glm::vec2 uv2;

    // Texture index for this triangle, -1 if no texture
    size_t textureIdx = -1;

    // Triangle normal, color and centroid
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec3 centroid;

    // Material properties for subsurface scattering
    Material material;

    Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 color)
        : v0(v0), v1(v1), v2(v2), color(color)
    {
        ComputeNormal();
        ComputeCentroid();
        material = Material::Marble();
    }

    Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2, size_t textureIdx, glm::vec3 color)
        : v0(v0), v1(v1), v2(v2), uv0(uv0), uv1(uv1), uv2(uv2), textureIdx(textureIdx), color(color)
    {
        ComputeNormal();
        ComputeCentroid();
        material = Material::Marble();
    }

    void
    ComputeNormal()
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