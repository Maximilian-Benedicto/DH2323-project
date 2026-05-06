#ifndef LAMBERTIAN_SHADER_HPP
#define LAMBERTIAN_SHADER_HPP

#include <glm/glm.hpp>
#include <vector>

#include "Model.hpp"
#include "Shader.hpp"

/// @brief Simple shader that implements Lambertian reflectance with hard shadows and indirect lighting.
class LambertianShader : public Shader {
   private:
    /// @brief Constant ambient term for indirect lighting.
    glm::vec3 indirectLight;

    /// @brief Compute the direct lighting at a point of intersection
    /// @param hit Information about the ray-triangle intersection for which to compute direct lighting.
    /// @param model Model containing the triangles in the scene.
    /// @param light Light source in the scene.
    /// @return RGB color of the direct lighting at the point of intersection.
    glm::vec3 directLight(const Intersection& hit, const Model& model, const Light& light);

    /// @brief Render a square region of the pixel buffer, used for multithreaded rendering.
    /// @param pixelBuffer Pointer to the pixel buffer to write the rendered image to.
    /// @param width Width of the pixel buffer.
    /// @param height Height of the pixel buffer.
    /// @param x1 Top-left x coordinate of the square region to render.
    /// @param y1 Top-left y coordinate of the square region to render.
    /// @param x2 Bottom-right x coordinate of the square region to render.
    /// @param y2 Bottom-right y coordinate of the square region to render.
    /// @param model Model containing the triangles to render.
    /// @param light Light source in the scene.
    /// @param camera Camera defining the viewpoint and viewing direction for rendering.
    /// @param shouldStopRenderThread Atomic flag to stop rendering.
    void renderSquare(Uint32* pixelBuffer, int width, int height, int x1, int y1, int x2, int y2,
                      const Model& model, const Light& light, const Camera& camera,
                      std::atomic<bool>& shouldStopRenderThread);

   public:
    /// @brief
    /// @param numberOfThreads
    /// @param indirectLight
    LambertianShader(int numberOfThreads, glm::vec3 indirectLight)
        : Shader(numberOfThreads), indirectLight(indirectLight) {}

    /// @brief Render the scene using this shader.
    /// @param pixelBuffer Pointer to a flat array of 32-bit RGBA values.
    /// @param width Width of the pixel buffer.
    /// @param height Height of the pixel buffer.
    /// @param model Model containing the triangles to render.
    /// @param light Light source in the scene.
    /// @param camera Camera defining the viewpoint and viewing direction for rendering.
    /// @param shouldStopRenderThread Atomic flag to stop rendering.
    void render(Uint32* pixelBuffer, int width, int height, const Model& model, const Light& light,
                const Camera& camera, std::atomic<bool>& shouldStopRenderThread) override;
};

#endif
