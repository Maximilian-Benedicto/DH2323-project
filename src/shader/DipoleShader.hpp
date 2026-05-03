#ifndef DIPOLE_SHADER_HPP
#define DIPOLE_SHADER_HPP

#include <glm/glm.hpp>
#include <vector>

#include "Model.hpp"
#include "Shader.hpp"

class DipoleShader : public Shader {
   public:
    /// @brief Rendering modes for the dipole shader.
    enum Mode { NORMAL, SINGLE_SCATTER, MULTIPLE_SCATTER, FRESNEL, FULL };
    Mode mode;

    /// @brief Construct a dipole shader.
    DipoleShader();

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
        float pdf;
        float s_i;
        float s_o;
        int triangleIndex;
    };

    /// @brief
    /// @param start
    /// @param dir
    /// @param model
    /// @param closestHit
    /// @return
    bool closestIntersection(glm::vec3 start, glm::vec3 dir, const Model& model,
                             Intersection& closestHit);

    /// @brief
    /// @param aabb
    /// @param start
    /// @param dir
    /// @param tClose
    /// @return
    bool slabIntersection(const AABB& aabb, const glm::vec3& start,
                          const glm::vec3& dir, float& tClose);

    /// @brief
    /// @param model
    /// @param closestHit
    /// @return
    DipoleSample samplePointMultipleScattering(const Model& model,
                                               const Intersection& closestHit);
    /// @brief
    /// @param closestHit
    /// @param model
    /// @param light
    /// @param samples
    /// @param viewDir
    /// @return
    glm::vec3 calculateMultipleScattering(
        const Intersection& closestHit, const Model& model, const Light& light,
        const std::vector<DipoleSample>& samples, const glm::vec3& viewDir);

    /// @brief
    /// @param model
    /// @param closestHit
    /// @param light
    /// @param viewDir
    /// @return
    DipoleSample samplePointSingleScattering(const Model& model,
                                             const Intersection& closestHit,
                                             const Light& light,
                                             const glm::vec3& viewDir);

    /// @brief
    /// @param closestHit
    /// @param model
    /// @param light
    /// @param samples
    /// @param viewDir
    /// @return
    glm::vec3 calculateSingleScattering(
        const Intersection& closestHit, const Model& model, const Light& light,
        const std::vector<DipoleSample>& samples, const glm::vec3& viewDir);
};

#endif
