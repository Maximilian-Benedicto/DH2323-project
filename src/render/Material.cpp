#include <cmath>

#include "Material.hpp"

namespace {
// TODO: replace with measured sigma_s data to avoid implicit g defaults.
constexpr float kDefaultG = 0.8f;
}  // namespace

Material::Material(glm::vec3 sigma_a, glm::vec3 sigma_s_prime,
                   glm::vec3 sigma_s, float eta)
    : sigma_a(sigma_a),
      sigma_s_prime(sigma_s_prime),
      sigma_s(sigma_s),
      eta(eta) {
    computeDerivedProperties();
}

void Material::computeDerivedProperties() {
    f_dr = glm::vec3(-1.440f / (eta * eta) + 0.710f / eta + 0.668f +
                     0.0636f * eta);

    A = (1.0f + f_dr) / (1.0f - f_dr);

    sigma_t_prime = sigma_a + sigma_s_prime;
    alpha_prime = sigma_s_prime / sigma_t_prime;
    sigma_tr = glm::sqrt(3.0f * sigma_a * sigma_t_prime);
    D = 1.0f / (3.0f * sigma_t_prime);

    z_r = 1.0f / sigma_t_prime;
    z_v = z_r + 4.0f * A * D;
}

Material Material::createApple() {
    const glm::vec3 sigma_s_prime(2.29f, 2.39f, 1.97f);
    const glm::vec3 sigma_s = sigma_s_prime / (1.0f - kDefaultG);
    return Material(glm::vec3(0.0030f, 0.0034f, 0.0460f), sigma_s_prime,
                    sigma_s, 1.3f);
}

Material Material::createChicken1() {
    const glm::vec3 sigma_s_prime(0.15f, 0.21f, 0.38f);
    const glm::vec3 sigma_s = sigma_s_prime / (1.0f - kDefaultG);
    return Material(glm::vec3(0.0150f, 0.0770f, 0.1900f), sigma_s_prime,
                    sigma_s, 1.3f);
}

Material Material::createCream() {
    const glm::vec3 sigma_s_prime(7.38f, 5.47f, 3.15f);
    const glm::vec3 sigma_s = sigma_s_prime / (1.0f - kDefaultG);
    return Material(glm::vec3(0.0002f, 0.0028f, 0.0163f), sigma_s_prime,
                    sigma_s, 1.3f);
}

Material Material::createKetchup() {
    const glm::vec3 sigma_s_prime(0.18f, 0.07f, 0.03f);
    const glm::vec3 sigma_s = sigma_s_prime / (1.0f - kDefaultG);
    return Material(glm::vec3(0.0610f, 0.9700f, 1.4500f), sigma_s_prime,
                    sigma_s, 1.3f);
}

Material Material::createMarble() {
    const glm::vec3 sigma_s_prime(2.19f, 2.62f, 3.00f);
    const glm::vec3 sigma_s = sigma_s_prime / (1.0f - kDefaultG);
    return Material(glm::vec3(0.0021f, 0.0041f, 0.0071f), sigma_s_prime,
                    sigma_s, 1.5f);
}

Material Material::createPotato() {
    const glm::vec3 sigma_s_prime(0.68f, 0.70f, 0.55f);
    const glm::vec3 sigma_s = sigma_s_prime / (1.0f - kDefaultG);
    return Material(glm::vec3(0.0024f, 0.0090f, 0.1200f), sigma_s_prime,
                    sigma_s, 1.3f);
}

Material Material::createSkimMilk() {
    const glm::vec3 sigma_s_prime(0.70f, 1.22f, 1.90f);
    const glm::vec3 sigma_s = sigma_s_prime / (1.0f - kDefaultG);
    return Material(glm::vec3(0.0014f, 0.0025f, 0.0142f), sigma_s_prime,
                    sigma_s, 1.3f);
}

Material Material::createSkin1() {
    const glm::vec3 sigma_s_prime(0.74f, 0.88f, 1.01f);
    const glm::vec3 sigma_s = sigma_s_prime / (1.0f - kDefaultG);
    return Material(glm::vec3(0.0320f, 0.1700f, 0.4800f), sigma_s_prime,
                    sigma_s, 1.3f);
}

Material Material::createWholeMilk() {
    const glm::vec3 sigma_s_prime(2.55f, 3.21f, 3.77f);
    const glm::vec3 sigma_s = sigma_s_prime / (1.0f - kDefaultG);
    return Material(glm::vec3(0.0011f, 0.0024f, 0.0140f), sigma_s_prime,
                    sigma_s, 1.3f);
}
