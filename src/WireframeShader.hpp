#ifndef WIREFRAME_SHADER_HPP
#define WIREFRAME_SHADER_HPP

#include <vector>
#include <glm/glm.hpp>

#include "Model.hpp"
#include "Shader.hpp"

/// @brief Simple shader that renders the scene in wireframe, optionally showing the BVH structure.
class WireframeShader : public Shader {
   private:
    /// @brief Draw a line between two points in screen space using Bresenham's algorithm.
    /// @param pixelBuffer Pointer to a flat array of 32-bit RGBA values representing the pixel buffer.
    /// @param width Width of the pixel buffer.
    /// @param height Height of the pixel buffer.
    /// @param x0 Starting x coordinate of the line.
    /// @param y0 Starting y coordinate of the line.
    /// @param x1 Ending x coordinate of the line.
    /// @param y1 Ending y coordinate of the line.
    /// @param color 32-bit RGBA color value to use for the line.
    void drawLine(Uint32 *pixelBuffer, int width, int height, int x0, int y0, int x1, int y1, Uint32 color);

   public:
    /// @brief Whether to visualize the BVH structure by drawing the bounding boxes of the BVH nodes in wireframe.
    bool isShowingBvh = false;

    /// @brief Construct a WireframeShader with the given mode.
    WireframeShader();

    /// @brief Render the scene using this shader.
    /// @param pixelBuffer Pointer to a flat array of 32-bit RGBA values.
    /// @param width Width of the pixel buffer.
    /// @param height Height of the pixel buffer.
    /// @param model Model containing the triangles to render.
    /// @param light Light source in the scene.
    /// @param camera Camera defining the viewpoint and viewing direction for rendering.
    /// @param shouldStopRenderThread Atomic flag to stop rendering.
    void render(Uint32 *pixelBuffer, int width, int height, const Model &model, const Light &light,
                const Camera &camera, std::atomic<bool> &shouldStopRenderThread) override;
};

#endif
