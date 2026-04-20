#ifndef WIREFRAME_SHADER_HPP
#define WIREFRAME_SHADER_HPP

#include "Shader.hpp"
#include <vector>
#include <glm/glm.hpp>

/**
 * @brief Shader that renders the wireframe of the triangles in the scene
 */
class WireframeShader : public Shader
{
private:
    void DrawLine(Uint32 *pixelBuffer, int width, int height, int x0, int y0, int x1, int y1, Uint32 color);

public:
    WireframeShader();
    void render(Uint32 *pixelBuffer, int width, int height, const std::vector<Triangle> &triangles, const Light &light, const Camera &camera, std::atomic<bool> &killFlag) override;
};

#endif
