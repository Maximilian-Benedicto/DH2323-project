#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <glm/glm.hpp>

/**
 * @brief Material class representing the optical properties of a material for subsurface scattering
 */
class Material
{
public:
    // Fundamental properties based on Jensen 2001
    glm::vec3 sigma_a;       // Absorption coefficient
    glm::vec3 sigma_s_prime; // Reduced scattering coefficient
    float eta;               // Relative index of refraction

    // Derived dipole properties
    glm::vec3 sigma_t_prime; // Reduced extinction coefficient
    glm::vec3 alpha_prime;   // Reduced albedo
    glm::vec3 sigma_tr;      // Effective transport coefficient
    glm::vec3 D;             // Diffusion constant
    float f_dr;              // Average diffuse Fresnel reflectance
    float A;                 // Internal reflection parameter

    // Dipole positions
    glm::vec3 z_r; // Real source depth
    glm::vec3 z_v; // Virtual source height

    Material(glm::vec3 sigma_a, glm::vec3 sigma_s_prime, float eta);
    Material() = default;

    // Recalculates all derived properties from the fundamentals
    void computeDerivedProperties();

    // Factory methods for measured presets from the Jensen 2001 table
    static Material createApple();
    static Material createChicken1();
    static Material createCream();
    static Material createKetchup();
    static Material createMarble();
    static Material createPotato();
    static Material createSkimMilk();
    static Material createSkin1();
    static Material createWholeMilk();
};

#endif
