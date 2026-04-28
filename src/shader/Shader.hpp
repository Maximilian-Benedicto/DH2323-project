#ifndef SHADER_HPP
#define SHADER_HPP

#include <SDL3/SDL.h>
#include <atomic>
#include <glm/glm.hpp>
#include <vector>

#include "BVH.hpp"
#include "Model.hpp"

class Triangle;
class Light;
class Camera;

/// @brief Abstract base class for shaders.
class Shader {
   public:
    virtual ~Shader() = default;

    virtual void render(Uint32* pixelBuffer, int width, int height,
                        const Model& model, const Light& light,
                        const Camera& camera,
                        std::atomic<bool>& shouldStopRenderThread) = 0;
};

#endif
