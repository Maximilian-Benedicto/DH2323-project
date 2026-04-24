#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <glm/glm.hpp>

class Material {
   public:
    glm::vec3 sigma_a;
    glm::vec3 sigma_s_prime;
    float eta;

    glm::vec3 sigma_t_prime;
    glm::vec3 alpha_prime;
    glm::vec3 sigma_tr;
    glm::vec3 D;
    glm::vec3 f_dr;
    glm::vec3 A;

    glm::vec3 z_r;
    glm::vec3 z_v;

    Material(glm::vec3 sigma_a, glm::vec3 sigma_s_prime, float eta);
    Material() = default;

    void computeDerivedProperties();

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
