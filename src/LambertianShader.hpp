#ifndef LAMBERTIAN_SHADER_HPP
#define LAMBERTIAN_SHADER_HPP

#include "Shader.hpp"
#include <vector>
#include <glm/glm.hpp>
#include "Model.hpp"

class LambertianShader : public Shader {
   private:
    struct Intersection {
        glm::vec3 position;
        float distance;
        int triangleIndex;
        glm::vec2 uv;
    };

    glm::vec3 indirectLight;

    bool closestIntersection(glm::vec3 start, glm::vec3 dir, const Model &model, Intersection &closestHit);
    glm::vec3 directLight(const Intersection &hit, const Model &model, const Light &light);
    bool slabIntersection(const AABB &aabb, const glm::vec3 &start, const glm::vec3 &dir, float &tClose);

   public:
    LambertianShader();
    void render(Uint32 *pixelBuffer, int width, int height, const Model &model, const Light &light,
                const Camera &camera, std::atomic<bool> &shouldStopRenderThread) override;
};

#endif
