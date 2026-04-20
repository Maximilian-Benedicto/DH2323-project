#include "DipoleShader.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "Triangle.hpp"
#include <iostream>

using namespace glm;
using namespace std;

DipoleShader::DipoleShader() {}

void DipoleShader::render(Uint32 *pixelBuffer, int width, int height, const BVH &bvh, const std::vector<Triangle> &triangles, const Light &light, const Camera &camera, std::atomic<bool> &killFlag)
{
    // Stub implementation for dipole, return black for now
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (killFlag)
                return;
            pixelBuffer[y * width + x] = 0xFF000000; // Black with alpha=255
        }
    }
}
