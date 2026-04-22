#ifndef DIPOLE_SHADER_HPP
#define DIPOLE_SHADER_HPP

#include "Shader.hpp"
#include "Model.hpp"
#include <vector>
#include <glm/glm.hpp>

/**
 * @brief A shader that implements the dipole diffusion approximation for subsurface scattering.
 */
class DipoleShader : public Shader {
   public:
    /**
     * @brief Different visualization modes for the dipole shader.
     */
    enum Mode {
        NORMAL,            // Visualize surface normals
        SINGLE_SCATTER,    // Visualize single scattering component
        MULTIPLE_SCATTER,  // Visualize multiple scattering component
        FRESNEL,           // Visualize Fresnel reflectance
        FULL               // Visualize full dipole model
    };

    /**
     * @brief Current visualization mode.
     */
    Mode mode;

    DipoleShader();
    void render(Uint32 *pixelBuffer, int width, int height, const Model &model, const Light &light,
                const Camera &camera, std::atomic<bool> &shouldStopRenderThread) override;

   private:
    // Raytracing functions

    /**
     * @brief Struct to hold information about an intersection between a ray and the model
     */
    struct Intersection {
        glm::vec3 position;
        float distance;
        int triangleIndex;
        glm::vec2 uv;
    };

    /**
     * @brief Find the closest intersection of a ray with the model. Returns true if an intersection is found, false
     * otherwise.
     */
    bool closestIntersection(glm::vec3 start, glm::vec3 dir, const Model &model, Intersection &closestHit);

    /**
     * @brief Find the intersection of a ray with an axis-aligned bounding box.
     */
    bool slabIntersection(const AABB &aabb, const glm::vec3 &start, const glm::vec3 &dir, float &tClose);

    // Dipole model functions

    // Fresnel and phase functions
    float fresnelReflectance(float cosTheta);
    float fresnelTransmittance(float cosTheta);
    float fresnel(glm::vec3 wo, glm::vec3 wi);
    float phaseFunction(float cosTheta);

    // Distance and geometry functions
    float scalarDistance(glm::vec3 xi, glm::vec3 xo);
    float positiveDistance(float r);
    float negativeDistance(float r);
    float diffuseReflectance(float r);

    // dipole components
    float multipleScattering(glm::vec3 xi, glm::vec3 w1, glm::vec3 xo, glm::vec3 w0);
    glm::vec3 outgoingRadiance(glm::vec3 xo, glm::vec3 wo, glm::vec3 xi, glm::vec3 w1, glm::vec3 wop, glm::vec3 wip);
    glm::vec3 incidentRadiance(glm::vec3 xi, glm::vec3 wi);

    // single scattering component

    float exponential(glm::vec3 xi, glm::vec3 xo);
    float combinedExtinctionCoefficient(glm::vec3 xi, glm::vec3 xo);
    glm::vec3 singleScattering(glm::vec3 xo, glm::vec3 wo);

    float geometryFactor(glm::vec3 wop, glm::vec3 wip);
};

#endif
