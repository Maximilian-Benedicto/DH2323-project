#include "include/WireframeShader.hpp"
#include "include/Camera.hpp"
#include "include/Light.hpp"
#include "include/Triangle.hpp"
#include <iostream>
#include <algorithm>

using namespace glm;
using namespace std;

WireframeShader::WireframeShader() {}

mat3 WireframeShader::RotationMatrix(float pitch, float roll, float yaw)
{
    mat3 Rx = mat3{
        1.0f, 0.0f, 0.0f,
        0.0f, cos(pitch), -sin(pitch),
        0.0f, sin(pitch), cos(pitch)};

    mat3 Ry = mat3{
        cos(yaw), 0.0f, sin(yaw),
        0.0f, 1.0f, 0.0f,
        -sin(yaw), 0.0f, cos(yaw)};

    mat3 Rz = mat3{
        cos(roll), -sin(roll), 0.0f,
        sin(roll), cos(roll), 0.0f,
        0.0f, 0.0f, 1.0f};

    return Ry * Rz * Rx;
}

void WireframeShader::DrawLine(Uint32 *pixelBuffer, int width, int height, int x0, int y0, int x1, int y1, Uint32 color)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true)
    {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height)
        {
            pixelBuffer[y0 * width + x0] = color;
        }
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void WireframeShader::render(Uint32 *pixelBuffer, int width, int height, const std::vector<Triangle> &triangles, const Light &light, const Camera &camera, std::atomic<bool> &killFlag)
{
    // Clear buffer to black
    for (int i = 0; i < width * height; ++i)
    {
        if (killFlag)
            return;
        pixelBuffer[i] = 0xFF000000;
    }

    mat3 R = RotationMatrix(camera.pitch, camera.roll, camera.yaw);
    mat3 RT = transpose(R); // Inverse rotation to move points into camera space

    for (size_t i = 0; i < triangles.size(); ++i)
    {
        if (killFlag)
            return;

        vec3 v[3] = {triangles[i].v0, triangles[i].v1, triangles[i].v2};
        int p[3][2];
        bool outOfBounds[3] = {false, false, false};

        for (int j = 0; j < 3; ++j)
        {
            // Translate to camera center and rotate
            vec3 vCam = RT * (v[j] - camera.position);

            // Only draw if in front of camera
            if (vCam.z <= 0.1f)
            {
                outOfBounds[j] = true;
                continue;
            }

            // Project
            float px = vCam.x / vCam.z * camera.focalLength + width / 2.0f;
            float py = vCam.y / vCam.z * camera.focalLength + height / 2.0f;
            p[j][0] = (int)px;
            p[j][1] = (int)py;
        }

        // Draw lines
        if (!outOfBounds[0] && !outOfBounds[1])
            DrawLine(pixelBuffer, width, height, p[0][0], p[0][1], p[1][0], p[1][1], 0xFF00FF00);
        if (!outOfBounds[1] && !outOfBounds[2])
            DrawLine(pixelBuffer, width, height, p[1][0], p[1][1], p[2][0], p[2][1], 0xFF00FF00);
        if (!outOfBounds[2] && !outOfBounds[0])
            DrawLine(pixelBuffer, width, height, p[2][0], p[2][1], p[0][0], p[0][1], 0xFF00FF00);
    }
}
