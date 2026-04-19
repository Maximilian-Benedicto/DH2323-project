#include "include/LambertianShader.hpp"
#include "include/Camera.hpp"
#include "include/Light.hpp"
#include "include/Triangle.hpp"
#include <iostream>

using namespace glm;
using namespace std;

LambertianShader::LambertianShader() : indirectLight(vec3(0.5f, 0.5f, 0.5f)) {}

bool LambertianShader::ClosestIntersection(vec3 start, vec3 dir, const vector<Triangle> &triangles, Intersection &closestIntersection)
{
    bool found = false;

    for (size_t i = 0; i < triangles.size(); i++)
    {
        Triangle triangle = triangles[i];
        vec3 v0 = triangle.v0;
        vec3 v1 = triangle.v1;
        vec3 v2 = triangle.v2;
        vec3 e1 = v1 - v0;
        vec3 e2 = v2 - v0;
        vec3 b = start - v0;
        mat3 A(-dir, e1, e2);
        vec3 x = inverse(A) * b;

        float t = x.x;
        float u = x.y;
        float v = x.z;

        if (!(0 <= t && 0 <= u && 0 <= v && u + v <= 1))
            continue;

        vec3 position = start + dir * t;
        float distance = length(dir * t);

        if (!found || distance < closestIntersection.distance)
        {
            closestIntersection = {position, distance, (int)i};
            found = true;
        }
    }

    return found;
}

vec3 LambertianShader::DirectLight(const Intersection &i, const vector<Triangle> &triangles, const Light &light)
{
    vec3 r = light.position - i.position;
    vec3 nUnit = triangles[i.triangleIndex].normal;
    vec3 start = i.position + nUnit * 1e-4f;

    Intersection reverse;
    if (ClosestIntersection(start, r, triangles, reverse))
    {
        if (length(start - reverse.position) < length(r))
            return vec3(0, 0, 0);
    }

    vec3 B = light.color / (float)(4 * M_PI * pow(length(r), 2));
    vec3 rUnit = normalize(r);
    vec3 D = B * glm::max(dot(rUnit, nUnit), 0.0f);

    return D;
}

mat3 LambertianShader::RotationMatrix(float pitch, float roll, float yaw)
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

void LambertianShader::render(Uint32 *pixelBuffer, int width, int height, const std::vector<Triangle> &triangles, const Light &light, const Camera &camera, std::atomic<bool> &killFlag)
{
    mat3 R = RotationMatrix(camera.pitch, camera.roll, camera.yaw);
    vec3 start = camera.position;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (killFlag)
                return;
            vec3 dir((float)x - width / 2.0f, (float)y - height / 2.0f, camera.focalLength);
            dir = dir * R;

            Intersection closestIntersection;
            bool found = ClosestIntersection(start, dir, triangles, closestIntersection);

            vec3 color(0, 0, 0);
            if (found)
            {
                vec3 directL = DirectLight(closestIntersection, triangles, light);
                color = triangles[closestIntersection.triangleIndex].color * (directL + indirectLight);
            }
            Uint8 r = Uint8(glm::clamp(255 * color.r, 0.f, 255.f));
            Uint8 g = Uint8(glm::clamp(255 * color.g, 0.f, 255.f));
            Uint8 b = Uint8(glm::clamp(255 * color.b, 0.f, 255.f));
            Uint32 rgba = (255 << 24) | (r << 16) | (g << 8) | b;
            pixelBuffer[y * width + x] = rgba;
        }
    }
}
