#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <glm/glm.hpp>
#include <vector>

#include "Material.hpp"
#include "Texture.hpp"

/// @brief Triangle class representing a single triangle in the scene
class Triangle {
   public:
    /// @brief Vertices of the triangle in world space.
    glm::vec3 v0, v1, v2;

    /// @brief Texture coordinates for each vertex.
    glm::vec2 uv0, uv1, uv2;

    /// @brief Index of the texture to use for this triangle, or -1 if no texture.
    size_t textureIdx = -1;

    /// @brief Normal vector of the triangle, computed from the vertices.
    glm::vec3 normal;

    /// @brief Centroid of the triangle, computed from the vertices.
    glm::vec3 centroid;

    /// @brief Color of the triangle, used if no texture is applied.
    glm::vec3 color;

    /// @brief Material properties of the triangle, used for dipole method.
    Material material = Material::createChicken2();

    /// @brief Construct a triangle with a given color and no texture.
    /// @param v0
    /// @param v1
    /// @param v2
    /// @param color
    Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 color)
        : v0(v0), v1(v1), v2(v2), color(color) {
        computeNormal();
        computeCentroid();
    }

    /// @brief Construct a triangle with a given color and texture.
    /// @param v0
    /// @param v1
    /// @param v2
    /// @param uv0
    /// @param uv1
    /// @param uv2
    /// @param textureIdx
    /// @param color
    Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2,
             size_t textureIdx, glm::vec3 color)
        : v0(v0),
          v1(v1),
          v2(v2),
          uv0(uv0),
          uv1(uv1),
          uv2(uv2),
          textureIdx(textureIdx),
          color(color) {
        computeNormal();
        computeCentroid();
    }

    /// @brief Compute the normal vector of the triangle based on its vertices.
    void computeNormal() {
        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;
        normal = glm::normalize(glm::cross(e2, e1));
    }

    /// @brief Compute the centroid of the triangle based on its vertices.
    void computeCentroid() { centroid = (v0 + v1 + v2) / 3.0f; }

    /// @brief Compute the area of the triangle.
    /// @return The area of the triangle.
    float area() const { return 0.5f * glm::length(glm::cross(v1 - v0, v2 - v0)); }

    /// @brief Sample a point uniformly on the triangle's surface using barycentric coordinates.
    /// @param u1 A random number in the range [0, 1) used for sampling.
    /// @param u2 A random number in the range [0, 1) used for sampling.
    /// @return A point sampled uniformly on the triangle's surface.
    glm::vec3 samplePoint(float u1, float u2) const {
        float sqrtU1 = std::sqrt(u1);
        float b0 = 1.0f - sqrtU1;
        float b1 = u2 * sqrtU1;
        float b2 = 1.0f - b0 - b1;
        return v0 * b0 + v1 * b1 + v2 * b2;
    }
};

#endif