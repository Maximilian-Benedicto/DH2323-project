#ifndef DIPOLE_HELPER_HPP
#define DIPOLE_HELPER_HPP

#include <cmath>
#include <glm/glm.hpp>

#include "Material.hpp"
#include "Model.hpp"
#include "Triangle.hpp"

/// @brief Helper functions for the dipole shader.
namespace DipoleScattering {

/// @brief Anistropy parameter kept constant for all materials, since we don't have measured sigma_s data to compute it from.
constexpr float g = 0.8f;

/// @brief Calculate the Fresnel reflectance for a given angle and material.
/// @param cosTheta Cosine of the angle between the normal and the direction.
/// @param material Material properties of the surface.
/// @return Fresnel reflectance value.
inline float fresnelReflectance(float cosTheta, const Material& material) {
    float r0 = std::pow((1.0f - material.eta) / (1.0f + material.eta), 2.0f);
    return r0 + (1.0f - r0) * std::pow(1.0f - cosTheta, 5.0f);
}

/// @brief Calculate the Fresnel transmittance for a given angle and material.
/// @param cosTheta Cosine of the angle between the normal and the direction.
/// @param material Material properties of the surface.
/// @return Fresnel transmittance value.
inline float fresnelTransmittance(float cosTheta, const Material& material) {
    return 1.0f - fresnelReflectance(cosTheta, material);
}

/// @brief Calculate the scalar distance between two points in 3D space.
/// @param xi Entry point of the ray inside the medium.
/// @param xo Exit point of the ray from the medium.
/// @return Scalar distance between the entry and exit points.
inline float scalarDistance(glm::vec3 xi, glm::vec3 xo) {
    return glm::length(xi - xo);
}

/// @brief Calculate the positive distance from the exit point to the real/virtual source based on the dipole model.
/// @param r Distance from the entry point to the exit point.
/// @param material Material properties of the surface.
/// @return Per-channel distance from the exit point to the real source.
inline glm::vec3 distance(float r, const glm::vec3& z_source) {
    return glm::sqrt(r * r + z_source * z_source);
}

/// @brief Calculate the diffuse reflectance at a given distance from the exit point based on the dipole model.
/// @param r Distance from the entry point to the exit point.
/// @param material Material properties of the surface.
/// @return Per-channel diffuse reflectance at the given distance.
inline glm::vec3 diffuseReflectance(float r, const Material& material) {
    glm::vec3 alpha_term = material.alpha_prime / (float)(4.0f * M_PI);
    glm::vec3 z_r_term =
        material.z_r * (material.sigma_tr * distance(r, material.z_r) + 1.0f);
    glm::vec3 r_exp_term =
        glm::exp(-material.sigma_tr * distance(r, material.z_r)) /
        (material.sigma_t_prime *
         glm::pow(distance(r, material.z_r), glm::vec3(3.0f)));
    glm::vec3 z_v_term =
        material.z_v * (material.sigma_tr * distance(r, material.z_v) + 1.0f);
    glm::vec3 v_exp_term =
        glm::exp(-material.sigma_tr * distance(r, material.z_v)) /
        (material.sigma_t_prime *
         glm::pow(distance(r, material.z_v), glm::vec3(3.0f)));

    return alpha_term * (z_r_term * r_exp_term + z_v_term * v_exp_term);
}

/// @brief Calculate the phase function value for a given angle based on the Henyey-Greenstein phase function.
/// @details https://en.wikipedia.org/wiki/Henyey%E2%80%93Greenstein_phase_function
/// @param cosTheta Cosine of the angle between the incident and outgoing directions.
/// @return Phase function value for the given angle.
inline float phaseFunction(float cosTheta) {
    const float gSquared = g * g;
    float base = 1.0f + gSquared - 2.0f * g * cosTheta;
    return (1.0f - gSquared) / (4.0f * (float)M_PI * std::pow(base, 1.5f));
}

/// @brief Calculate the average of the three components of a vec3.
/// @param v The input vec3 for which to calculate the average of its components.
/// @return The average value of the three components of the input vec3.
inline float average(const glm::vec3& v) {
    return (v.x + v.y + v.z) / 3.0f;
}

}  // namespace DipoleScattering

#endif
