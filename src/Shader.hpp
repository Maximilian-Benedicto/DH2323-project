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

class Shader {
   public:
    virtual ~Shader() = default;

    virtual void render(Uint32 *pixelBuffer, int width, int height, const Model &model, const Light &light,
                        const Camera &camera, std::atomic<bool> &shouldStopRenderThread) = 0;
};

#endif
