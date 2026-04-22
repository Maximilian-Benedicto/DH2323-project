#ifndef DIPOLE_SHADER_HPP
#define DIPOLE_SHADER_HPP

#include "Shader.hpp"
#include "Model.hpp"
#include <vector>
#include <glm/glm.hpp>

/**
 * @brief A shader that implements the dipole diffusion approximation for subsurface scattering.
 */
class DipoleShader : public Shader
{
public:
    DipoleShader();
    void render(Uint32 *pixelBuffer, int width, int height, const Model &model, const Light &light, const Camera &camera, std::atomic<bool> &killFlag) override;
private:
    float ScalarDistance (vec3 xi, vec3 xo);
    float PositiveDistance (float r);
    float NegativeDistance (float r);
    float DiffuseReflectance (float r);
    float FresnelReflectance(float cosTheta);
    float FresnelTransmittance(float cosTheta);
    float MultipleScattering (vec3 xi, vec3 w1, vec3 xo, vec3 w0);
    vec3 OutgoingRadiance (vec3 xo, vec3 wo, vec3 xi, vec3 w1, vec3 wop, vec3 wip);
    vec3 IncidentRadiance (vec3 xi, vec3 wi);
    float Fresnel (vec3 wo, vec3 wi);
    float Exponential (vec3 xo, vec3 xi);
    float GeometryFactor (vec3 n, vec3 wo, vec3 wi);
    vec3 SingleScattering(vec3 xo, vec3 wo);
    float PhaseFunction (float cosTheta);
};

#endif
