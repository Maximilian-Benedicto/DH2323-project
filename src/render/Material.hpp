#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <glm/glm.hpp>

/// @brief Material properties for subsurface scattering (based on Jensen et al. dipole model).
class Material {
   public:
    /// @brief Anisotropy factor for the Henyey-Greenstein phase function. Set to 0.8 by default.
    static constexpr float g = 0.8f;

    /// @brief Absorption coefficient
    glm::vec3 sigma_a;

    /// @brief Reduced scattering coefficient
    glm::vec3 sigma_s_prime;

    /// @brief Scattering coefficient
    glm::vec3 sigma_s;

    /// @brief Relative index of refraction
    float eta;

    /// @brief Extinction coefficient
    glm::vec3 sigma_t_prime;

    /// @brief Albedo
    glm::vec3 alpha_prime;

    /// @brief Effective extinction coefficient
    glm::vec3 sigma_tr;

    /// @brief Diffusion constant
    glm::vec3 D;

    /// @brief Diffuse fresnel reflectance
    glm::vec3 f_dr;

    /// @brief Fresnel constant
    glm::vec3 A;

    /// @brief Distance from surface to real source
    glm::vec3 z_r;

    /// @brief Distance from surface to virtual source
    glm::vec3 z_v;

    /// @brief Construct a material
    /// @param sigma_a Absorption coefficient
    /// @param sigma_s_prime Reduced scattering coefficient
    /// @param eta Relative index of refraction
    Material(glm::vec3 sigma_a, glm::vec3 sigma_s_prime, glm::vec3 sigma_s, float eta);
    Material() = default;

    /// @brief Compute derived properties for this material based on the input parameters.
    void computeDerivedProperties();

    static Material createApple();
    static Material createChicken1();
    static Material createChicken2();
    static Material createCream();
    static Material createKetchup();
    static Material createMarble();
    static Material createPotato();
    static Material createSkimmilk();
    static Material createSkin1();
    static Material createSkin2();
    static Material createSpectralon();
    static Material createWholemilk();
};

#endif
