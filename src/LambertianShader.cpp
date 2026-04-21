#include "LambertianShader.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "Triangle.hpp"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

using namespace glm;
using namespace std;

LambertianShader::LambertianShader() : indirectLight(vec3(0.5f, 0.5f, 0.5f)) {}

bool LambertianShader::ClosestIntersection(vec3 start, vec3 dir, const BVH &bvh, const vector<Triangle> &triangles, Intersection &closestIntersection)
{

    // Start at the root
    BVHNode node = bvh.bvhNodes[bvh.rootNodeIdx];

    bool leaf = node.isLeaf();

    while (!leaf)
    {
        // Get children
        BVHNode leftChild = bvh.bvhNodes[node.leftFirst];
        BVHNode rightChild = bvh.bvhNodes[node.leftFirst + 1];

        // Check if left child intersects with the ray

        bool found = false;

        for (size_t i = 0; i < triangles.size(); i++)
        {
            // Ray-triangle intersection algorithm
            Triangle triangle = triangles[i];
            vec3 v0 = triangle.v0;
            vec3 v1 = triangle.v1;
            vec3 v2 = triangle.v2;
            vec3 e1 = v1 - v0;
            vec3 e2 = v2 - v0;
            vec3 b = start - v0;
            mat3 A(-dir, e1, e2);
            vec3 x = inverse(A) * b;

            // Get distance t along the ray, and barycentric coordinate u and v
            float t = x.x;
            float u = x.y;
            float v = x.z;

            // Check if the intersection is valid
            if (!(0 <= t && 0 <= u && 0 <= v && u + v <= 1))
                continue;

            // Update closest intersection if this one is closer
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
}

vec3 LambertianShader::DirectLight(const Intersection &i, const BVH &bvh, const vector<Triangle> &triangles, const Light &light)
{
    // Check if the light is visible from the intersection point
    vec3 r = light.position - i.position;
    vec3 nUnit = triangles[i.triangleIndex].normal;
    vec3 start = i.position + nUnit * 1e-4f; // Offset start point to avoid self-intersection
    Intersection reverse;
    if (ClosestIntersection(start, r, bvh, triangles, reverse))
    {
        if (length(start - reverse.position) < length(r))
            return vec3(0, 0, 0);
    }

    // Compute direct light using Lambert's cosine law
    vec3 B = light.color / (float)(4 * M_PI * pow(length(r), 2));
    vec3 rUnit = normalize(r);
    vec3 D = B * glm::max(dot(rUnit, nUnit), 0.0f);

    return D;
}

void LambertianShader::render(Uint32 *pixelBuffer, int width, int height, const BVH &bvh, const std::vector<Triangle> &triangles, const Light &light, const Camera &camera, std::atomic<bool> &killFlag)
{
    // Compute camera basis vectors
    vec3 right = normalize(cross(vec3(0.0f, 1.0f, 0.0f), camera.direction));
    vec3 up = normalize(cross(camera.direction, right));

    // Apply roll rotation about camera direction
    up = glm::rotate(up, camera.roll, camera.direction);
    right = glm::rotate(right, camera.roll, camera.direction);

    vec3 start = camera.position;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            // Kill render thread if flag is set
            if (killFlag)
                return;

            // Compute ray direction for this pixel
            float dx = (float)x - width / 2.0f;
            float dy = (float)y - height / 2.0f;
            vec3 dir = normalize(camera.direction * camera.focalLength + right * dx + up * dy);

            // Find closest intersection of ray with scene
            Intersection closestIntersection;
            bool found = ClosestIntersection(start, dir, bvh, triangles, closestIntersection);

            // Compute color at intersection point
            vec3 color(0, 0, 0);
            if (found)
            {
                vec3 directL = DirectLight(closestIntersection, bvh, triangles, light);
                color = (triangles[closestIntersection.triangleIndex].color / (float)M_PI) * (directL + indirectLight);
            }

            // Write color to pixel buffer
            Uint8 r = Uint8(glm::clamp(255 * color.r, 0.f, 255.f));
            Uint8 g = Uint8(glm::clamp(255 * color.g, 0.f, 255.f));
            Uint8 b = Uint8(glm::clamp(255 * color.b, 0.f, 255.f));
            Uint32 rgba = (255 << 24) | (r << 16) | (g << 8) | b;
            pixelBuffer[y * width + x] = rgba;
        }
    }
}

bool LambertianShader::SlabIntersection(const AABB &aabb, const glm::vec3 &start, const glm::vec3 &dir, float &tClose)
{

    // Source (https://en.wikipedia.org/wiki/Slab_method)

    const vec3 l = aabb.min;
    const vec3 h = aabb.max;

    vec3 tiLow;
    vec3 tiHigh;

    for (int i = 0; i < 3; i++)
    {
        tiLow[i] = (l[i] - start[i]) / dir[i];
        tiHigh[i] = (h[i] - start[i]) / dir[i];
    }

    vec3 tiClose;
    vec3 tiFar;

    for (size_t i = 0; i < 3; i++)
    {
        tiClose[i] = glm::min(tiLow[i], tiHigh[i]);
        tiFar[i] = glm::max(tiLow[i], tiHigh[i]);
    }

        float tNear = glm::max(tiClose.x, glm::max(tiClose.y, tiClose.z));
        float tFar = glm::min(tiClose.x, glm::min(tiClose.y, tiClose.z));

    tClose = tNear;
    return tFar >= tNear;
}
