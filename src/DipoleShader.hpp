#ifndef DIPOLE_SHADER_HPP
#define DIPOLE_SHADER_HPP

#include "Shader.hpp"
#include <vector>
#include <glm/glm.hpp>

/**
 * @brief A shader that implements the dipole diffusion approximation for subsurface scattering.
 */
class DipoleShader : public Shader
{
public:
    DipoleShader();
    void render(Uint32 *pixelBuffer, int width, int height, const BVH &bvh, const std::vector<Triangle> &triangles, const Light &light, const Camera &camera, std::atomic<bool> &killFlag) override;
};

#endif
