#ifndef LAMBERTIAN_SHADER_HPP
#define LAMBERTIAN_SHADER_HPP

#include <glm/glm.hpp>
#include <vector>

#include "Model.hpp"
#include "Shader.hpp"

/// @brief Simple shader that implements Lambertian reflectance with hard shadows and indirect lighting.
class LambertianShader : public Shader {
   private:
    /// @brief Information about a ray-triangle intersection
    struct Intersection {
        glm::vec3 position;
        float distance;
        int triangleIndex;
        glm::vec2 uv;
    };

    /// @brief Constant ambient term for indirect lighting.
    glm::vec3 indirectLight;

    /// @brief Find the closest intersection of a ray with the triangles in the scene using the BVH for acceleration.
    /// @param start Starting point of the ray.
    /// @param dir Direction of the ray.
    /// @param model Model containing the triangles to intersect with.
    /// @param closestHit Output parameter that will be populated with information about the closest intersection.
    /// @return true if an intersection is found, false otherwise.
    bool closestIntersection(glm::vec3 start, glm::vec3 dir, const Model& model,
                             Intersection& closestHit);

    /// @brief Check if a ray intersects an axis-aligned bounding box using the slab method.
    /// @param aabb Axis-aligned bounding box to test for intersection.
    /// @param start Starting point of the ray.
    /// @param dir Direction of the ray.
    /// @param tClose Distance to the closest intersection with the bounding box.
    /// @return true if the ray intersects the bounding box, false otherwise.
    bool slabIntersection(const AABB& aabb, const glm::vec3& start,
                          const glm::vec3& dir, float& tClose);

    /// @brief Compute the direct lighting at a point of intersection
    /// @param hit Information about the ray-triangle intersection for which to compute direct lighting.
    /// @param model Model containing the triangles in the scene.
    /// @param light Light source in the scene.
    /// @return RGB color of the direct lighting at the point of intersection.
    glm::vec3 directLight(const Intersection& hit, const Model& model,
                          const Light& light);

   public:
    /// @brief Construct a LambertianShader with the given constant ambient term for indirect lighting.
    LambertianShader();

    /// @brief Render the scene using this shader.
    /// @param pixelBuffer Pointer to a flat array of 32-bit RGBA values.
    /// @param width Width of the pixel buffer.
    /// @param height Height of the pixel buffer.
    /// @param model Model containing the triangles to render.
    /// @param light Light source in the scene.
    /// @param camera Camera defining the viewpoint and viewing direction for rendering.
    /// @param shouldStopRenderThread Atomic flag to stop rendering.
    void render(Uint32* pixelBuffer, int width, int height, const Model& model,
                const Light& light, const Camera& camera,
                std::atomic<bool>& shouldStopRenderThread) override;
};

#endif
