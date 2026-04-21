#ifndef LAMBERTIAN_SHADER_HPP
#define LAMBERTIAN_SHADER_HPP

// From lab2 with modifications

#include "Shader.hpp"
#include <vector>
#include <glm/glm.hpp>

/**
 * @brief Shader that implements Lambertian diffuse shading with hard shadows and a simple ambient term.
 */
class LambertianShader : public Shader
{
private:
    struct Intersection
    {
        glm::vec3 position;
        float distance;
        int triangleIndex;
        glm::vec2 uv;
    };

    glm::vec3 indirectLight;

    bool ClosestIntersection(glm::vec3 start, glm::vec3 dir, const BVH &bvh, const std::vector<Triangle> &triangles, Intersection &closestIntersection);
    glm::vec3 DirectLight(const Intersection &i, const BVH &bvh, const std::vector<Triangle> &triangles, const Light &light);
    bool SlabIntersection(const AABB &aabb, const glm::vec3 &start, const glm::vec3 &dir, float &tClose);

public:
    LambertianShader();
    void render(Uint32 *pixelBuffer, int width, int height, const BVH &bvh, const std::vector<Triangle> &triangles, const Light &light, const Camera &camera, std::atomic<bool> &killFlag) override;
};

#endif
