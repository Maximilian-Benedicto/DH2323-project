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
    Material material;

    /// @brief Construct a triangle with a given color and no texture.
    /// @param v0
    /// @param v1
    /// @param v2
    /// @param color
    Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 color) : v0(v0), v1(v1), v2(v2), color(color) {
        computeNormal();
        computeCentroid();
        material = Material::createMarble();
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
    Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2, size_t textureIdx,
             glm::vec3 color)
        : v0(v0), v1(v1), v2(v2), uv0(uv0), uv1(uv1), uv2(uv2), textureIdx(textureIdx), color(color) {
        computeNormal();
        computeCentroid();
        material = Material::createMarble();
    }

    /// @brief Compute the normal vector of the triangle based on its vertices.
    void computeNormal() {
        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;
        normal = glm::normalize(glm::cross(e2, e1));
    }

    /// @brief Compute the centroid of the triangle based on its vertices.
    void computeCentroid() {
        centroid = (v0 + v1 + v2) / 3.0f;
    }
};

#endif