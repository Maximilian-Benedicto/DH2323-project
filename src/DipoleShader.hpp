#ifndef DIPOLE_SHADER_HPP
#define DIPOLE_SHADER_HPP

#include "Shader.hpp"
#include "Model.hpp"
#include <vector>
#include <glm/glm.hpp>

class DipoleShader : public Shader {
   public:
    enum Mode { NORMAL, SINGLE_SCATTER, MULTIPLE_SCATTER, FRESNEL, FULL };

    Mode mode;

    DipoleShader();
    void render(Uint32 *pixelBuffer, int width, int height, const Model &model, const Light &light,
                const Camera &camera, std::atomic<bool> &shouldStopRenderThread) override;

   private:
    struct Intersection {
        glm::vec3 position;
        float distance;
        int triangleIndex;
        glm::vec2 uv;
    };

    bool closestIntersection(glm::vec3 start, glm::vec3 dir, const Model &model, Intersection &closestHit);

    bool slabIntersection(const AABB &aabb, const glm::vec3 &start, const glm::vec3 &dir, float &tClose);

    float fresnelReflectance(float cosTheta, const Material &material);
    float fresnelTransmittance(float cosTheta, const Material &material);
    float fresnel(glm::vec3 wo, glm::vec3 wi);
    float phaseFunction(float cosTheta);

    float scalarDistance(glm::vec3 xi, glm::vec3 xo);
    glm::vec3 positiveDistance(float r, const Material &material);
    glm::vec3 negativeDistance(float r, const Material &material);
    glm::vec3 diffuseReflectance(float r, const Material &material);

    glm::vec3 multipleScattering(glm::vec3 xi, glm::vec3 wi, glm::vec3 xo, glm::vec3 w0, const Triangle &triangle);

    float exponential(glm::vec3 xi, glm::vec3 xo);
    float combinedExtinctionCoefficient(glm::vec3 xi, glm::vec3 xo);
    glm::vec3 singleScattering(glm::vec3 xo, glm::vec3 wo);
    glm::vec3 outgoingRadiance(glm::vec3 xo, glm::vec3 wo, glm::vec3 xi, glm::vec3 wi, glm::vec3 wop, glm::vec3 wip);
    glm::vec3 incidentRadiance(glm::vec3 xi, glm::vec3 wi);

    float geometryFactor(glm::vec3 wop, glm::vec3 wip);
};

#endif
