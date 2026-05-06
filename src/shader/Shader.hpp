#ifndef SHADER_HPP
#define SHADER_HPP

#include <SDL3/SDL.h>
#include <atomic>
#include <glm/glm.hpp>
#include <vector>

#include "BVH.hpp"
#include "Model.hpp"

class Triangle;
class Light;
class Camera;

/// @brief Base class for shaders.
class Shader {
   public:
    /// @brief Number of threads to use for rendering
    int NUM_THREADS = 16;

    /// @brief Information about a ray-triangle intersection
    struct Intersection {
        glm::vec3 position;
        float distance;
        int triangleIndex;
        glm::vec2 uv;
    };

    /// @brief
    /// @param numThreads
    Shader(int numThreads) : NUM_THREADS(numThreads) {};

    /// @brief Find the closest intersection of a ray with the scene.
    /// @param start Starting point of the ray.
    /// @param dir Direction of the ray.
    /// @param model The 3D model containing the triangles to test for intersection.
    /// @param closestHit Output parameter that will be set to the information about the closest intersection if an intersection is found.
    /// @return True if an intersection is found, false otherwise.
    bool closestIntersection(glm::vec3 start, glm::vec3 dir, const Model& model,
                             Intersection& closestHit);

    /// @brief Find the closest intersection of a ray with a axis-aligned bounding box (AABB).
    /// @param aabb The axis-aligned bounding box to test for intersection.
    /// @param start Starting point of the ray.
    /// @param dir Direction of the ray.
    /// @param tClose Output parameter that will be set to the distance to the closest intersection with the AABB if an intersection is found.
    /// @return True if an intersection is found, false otherwise.
    bool slabIntersection(const AABB& aabb, const glm::vec3& start, const glm::vec3& dir,
                          float& tClose);

    virtual ~Shader() = default;
    virtual void render(Uint32* pixelBuffer, int width, int height, const Model& model,
                        const Light& light, const Camera& camera,
                        std::atomic<bool>& shouldStopRenderThread) = 0;
};

#endif
