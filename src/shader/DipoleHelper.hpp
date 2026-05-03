#ifndef DIPOLE_HELPER_HPP
#define DIPOLE_HELPER_HPP

#include <cmath>
#include <glm/glm.hpp>

#include "Material.hpp"
#include "Model.hpp"
#include "Triangle.hpp"

namespace DipoleScattering {

/// @brief Anistropy parameter kept constant for all materials, since we don't have measured sigma_s data to compute it from.
constexpr float g = 0.8f;

/// @brief
/// @param cosTheta
/// @param material
/// @return
inline float fresnelReflectance(float cosTheta, const Material& material) {
    float r0 = std::pow((1.0f - material.eta) / (1.0f + material.eta), 2.0f);
    return r0 + (1.0f - r0) * std::pow(1.0f - cosTheta, 5.0f);
}

/// @brief
/// @param cosTheta
/// @param material
/// @return
inline float fresnelTransmittance(float cosTheta, const Material& material) {
    return 1.0f - fresnelReflectance(cosTheta, material);
}

/// @brief
/// @param xi
/// @param xo
/// @return
inline float scalarDistance(glm::vec3 xi, glm::vec3 xo) {
    return glm::length(xi - xo);
}

/// @brief
/// @param r
/// @param material
/// @return
inline glm::vec3 positiveDistance(float r, const Material& material) {
    glm::vec3 r_squared = glm::vec3(r * r);
    glm::vec3 z_r_squared = material.z_r * material.z_r;
    return glm::sqrt(r_squared + z_r_squared);
}

/// @brief
/// @param r
/// @param material
/// @return
inline glm::vec3 negativeDistance(float r, const Material& material) {
    glm::vec3 r_squared = glm::vec3(r * r);
    glm::vec3 z_v_squared = material.z_v * material.z_v;
    return glm::sqrt(r_squared + z_v_squared);
}

/// @brief
/// @param r
/// @param material
/// @return
inline glm::vec3 diffuseReflectance(float r, const Material& material) {
    glm::vec3 alpha_term = material.alpha_prime / (float)(4.0f * M_PI);
    glm::vec3 z_r_term =
        material.z_r *
        (material.sigma_tr * positiveDistance(r, material) + 1.0f);
    glm::vec3 r_exp_term =
        glm::exp(-material.sigma_tr * positiveDistance(r, material)) /
        (material.sigma_t_prime *
         glm::pow(positiveDistance(r, material), glm::vec3(3.0f)));
    glm::vec3 z_v_term =
        material.z_v *
        (material.sigma_tr * negativeDistance(r, material) + 1.0f);
    glm::vec3 v_exp_term =
        glm::exp(-material.sigma_tr * negativeDistance(r, material)) /
        (material.sigma_t_prime *
         glm::pow(negativeDistance(r, material), glm::vec3(3.0f)));

    return alpha_term * (z_r_term * r_exp_term + z_v_term * v_exp_term);
}

/// @brief
/// @param cosTheta
/// @return
inline float phaseFunction(float cosTheta) {
    const float g2 = g * g;
    float denom = 1.0f + g2 - 2.0f * g * cosTheta;
    return (1.0f - g2) / (4.0f * (float)M_PI * std::pow(denom, 1.5f));
}

/// @brief
/// @param model
/// @param u
/// @param totalArea
/// @return
inline int sampleTriangleIndex(const Model& model, float u, float& totalArea) {
    // Total area of the model used for sampling PDF
    totalArea = 0.0f;
    for (const Triangle& triangle : model.triangles)
        totalArea += triangle.area();
    if (totalArea <= 0.0f)
        return -1;

    // Sample a triangle based on area
    float target = u * totalArea;
    float running = 0.0f;
    for (size_t i = 0; i < model.triangles.size(); ++i) {
        running += model.triangles[i].area();
        if (running >= target)
            return (int)i;
    }

    // Return the last triangle.
    return (int)(model.triangles.size() - 1);
}

}  // namespace DipoleScattering

#endif
