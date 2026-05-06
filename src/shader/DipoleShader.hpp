#ifndef DIPOLE_SHADER_HPP
#define DIPOLE_SHADER_HPP

#include <glm/glm.hpp>
#include <vector>

#include "Model.hpp"
#include "Shader.hpp"

class DipoleShader : public Shader {
   public:
    /// @brief Rendering modes for the dipole shader.
    enum Mode { SINGLE_SCATTER, MULTIPLE_SCATTER, FULL };

    Mode mode = FULL;
    int MULTIPLE_SCATTER_SAMPLES = 100;
    int SINGLE_SCATTER_SAMPLES = 100;
    int MAX_DISK_SAMPLE_ATTEMPTS = 5;

    /// @brief Construct a dipole shader with the given parameters.
    /// @param mode Rendering mode for the dipole shader (single scatter, multiple scatter, or full).
    /// @param threadsSquared Number of threads to use for rendering, squared (e.g., 10 means 100 threads).
    /// @param multipleScatterSamples Number of samples to use for estimating the multiple scattering contribution.
    /// @param singleScatterSamples Number of samples to use for estimating the single scattering contribution.
    DipoleShader(Mode mode, int threadsSquared, int multipleScatterSamples,
                 int singleScatterSamples)
        : Shader(threadsSquared),
          mode(mode),
          MULTIPLE_SCATTER_SAMPLES(multipleScatterSamples),
          SINGLE_SCATTER_SAMPLES(singleScatterSamples) {}

    /// @brief Render the current scene to the window using the dipole method
    /// @param pixelBuffer Pointer to the pixel buffer to write the rendered image to.
    /// @param width Width of the render resolution.
    /// @param height Height of the render resolution.
    /// @param model The model to render.
    /// @param light The light source in the scene.
    /// @param camera The camera to render from.
    /// @param shouldStopRenderThread Atomic flag to stop rendering.
    void render(Uint32* pixelBuffer, int width, int height, const Model& model,
                const Light& light, const Camera& camera,
                std::atomic<bool>& shouldStopRenderThread) override;

   private:
    /// @brief Intersection information for ray-scene intersection tests.
    struct Intersection {
        glm::vec3 position;
        float distance;
        int triangleIndex;
        glm::vec2 uv;
    };

    /// @brief Sample point and PDF for multiple scattering sampling.
    struct DipoleSample {
        glm::vec3 position;

        /// @brief PDF of sampling this point, used for weighting the contribution of this sample in the final color calculation.
        float pdf;

        /// @brief Distance from the scatter point to the entry point (xi)
        float s_i;

        /// @brief Distance from the scatter point to the exit point (xo)
        float s_o;

        /// @brief Index of the triangle that was intersected during sampling, or -1 if no intersection occurred.
        int triangleIndex;
    };

    /// @brief
    /// @param pixelBuffer
    /// @param x1
    /// @param y1
    /// @param x2
    /// @param y2
    /// @param model
    /// @param light
    /// @param camera
    /// @param shouldStopRenderThread
    void renderSquare(Uint32* pixelBuffer, int width, int height, int x1,
                      int y1, int x2, int y2, const Model& model,
                      const Light& light, const Camera& camera,
                      std::atomic<bool>& shouldStopRenderThread);

    /// @brief Find the closest intersection of a ray with the scene geometry.
    /// @param start Starting point of the ray.
    /// @param dir Direction of the ray.
    /// @param model The model to test for intersection.
    /// @param closestHit Output parameter that will be set to the closest intersection information if an intersection is found.
    /// @return True if an intersection is found, false otherwise.
    bool closestIntersection(glm::vec3 start, glm::vec3 dir, const Model& model,
                             Intersection& closestHit);

    /// @brief Perform slab intersection test between a ray and an axis-aligned bounding box (AABB).
    /// @param aabb The axis-aligned bounding box to test against.
    /// @param start Starting point of the ray.
    /// @param dir Direction of the ray.
    /// @param tClose Output parameter that will be set to the distance to the closest intersection with the AABB if an intersection is found.
    /// @return True if the ray intersects the AABB, false otherwise.
    bool slabIntersection(const AABB& aabb, const glm::vec3& start,
                          const glm::vec3& dir, float& tClose);

    /// @brief Sample an entry point for multiple scattering based on the dipole model.
    /// @param model The 3D model containing the triangles to sample from.
    /// @param closestHit The intersection of the ray with the scene at the exit point.
    /// @return Sampled point and PDF information for multiple scattering sampling.
    DipoleSample samplePointMultipleScattering(const Model& model,
                                               const Intersection& closestHit);

    /// @brief Calculate the multiple scattering contribution to the final color at the exit point based on the dipole model.
    /// @param closestHit The intersection of the ray with the scene at the exit point.
    /// @param model The 3D model containing the triangles and their material properties.
    /// @param light The light source in the scene.
    /// @param samples The sampled entry points and PDF information for multiple scattering sampling.
    /// @param viewDir The direction from the exit point towards the camera (view direction).
    /// @return Calculated multiple scattering contribution to the final color at the exit point.
    glm::vec3 calculateMultipleScattering(
        const Intersection& closestHit, const Model& model, const Light& light,
        const std::vector<DipoleSample>& samples, const glm::vec3& viewDir);

    /// @brief Sample an entry point for single scattering based on the dipole model.
    /// @param model The 3D model containing the triangles to sample from.
    /// @param closestHit The intersection of the ray with the scene at the exit point.
    /// @param light The light source in the scene.
    /// @param viewDir The direction from the exit point towards the camera (view direction).
    /// @return Sampled point and PDF information for single scattering sampling.
    DipoleSample samplePointSingleScattering(const Model& model,
                                             const Intersection& closestHit,
                                             const Light& light,
                                             const glm::vec3& viewDir);

    /// @brief Calculate the single scattering contribution to the final color at the exit point based on the dipole model.
    /// @param closestHit The intersection of the ray with the scene at the exit point.
    /// @param model The 3D model containing the triangles and their material properties.
    /// @param light The light source in the scene.
    /// @param samples The sampled entry points and PDF information for single scattering sampling.
    /// @param viewDir The direction from the exit point towards the camera (view direction).
    /// @return Calculated single scattering contribution to the final color at the exit point.
    glm::vec3 calculateSingleScattering(
        const Intersection& closestHit, const Model& model, const Light& light,
        const std::vector<DipoleSample>& samples, const glm::vec3& viewDir);
};

#endif
