#ifndef SHADER_HPP
#define SHADER_HPP

#include <vector>
#include <atomic>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

class Triangle;
class Light;
class Camera;

/**
 * @brief Abstract base class for shaders
 */
class Shader
{
public:
    virtual ~Shader() = default;

    /**
     * @brief Render the scene using this shader, periodically check the killFlag and return if it is set to true.
     */
    virtual void render(Uint32 *pixelBuffer, int width, int height, const std::vector<Triangle> &triangles, const Light &light, const Camera &camera, std::atomic<bool> &killFlag) = 0;
};

#endif
