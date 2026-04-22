#ifndef SHADER_HPP
#define SHADER_HPP

#include <vector>
#include <atomic>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "BVH.hpp"
#include "Model.hpp"

class Triangle;
class Light;
class Camera;

/**
 * @brief Abstract base class for shaders
 */
class Shader {
   public:
    virtual ~Shader() = default;

    /**
     * @brief Render one full frame for the given model/camera/light state.
     * @param pixelBuffer Output buffer in ARGB8888 format.
     * @param width Pixel buffer width in pixels.
     * @param height Pixel buffer height in pixels.
     * @param model Active scene model.
     * @param light Active point light.
     * @param camera Active camera.
     * @param shouldStopRenderThread Cooperative cancellation signal set by the main thread.
     */
    virtual void render(Uint32 *pixelBuffer, int width, int height, const Model &model, const Light &light,
                        const Camera &camera, std::atomic<bool> &shouldStopRenderThread) = 0;
};

#endif
