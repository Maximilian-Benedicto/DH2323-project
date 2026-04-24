#include "LambertianShader.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "Triangle.hpp"
#include "Model.hpp"
#include "Texture.hpp"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

using namespace glm;
using namespace std;

LambertianShader::LambertianShader() : indirectLight(vec3(0.5f, 0.5f, 0.5f)) {}

bool LambertianShader::closestIntersection(vec3 start, vec3 dir, const Model &model, Intersection &closestHit) {
    bool found = false;
    if (model.bvh.nodesUsed == 0)
        return false;

    float closestDistance = std::numeric_limits<float>::infinity();
    std::vector<int> stack;
    stack.push_back(model.bvh.rootNodeIdx);

    while (!stack.empty()) {
        int nodeIdx = stack.back();
        stack.pop_back();

        const BVHNode &node = model.bvh.bvhNodes[nodeIdx];

        float nodeTClose;
        if (!slabIntersection(node.aabb, start, dir, nodeTClose))
            continue;
        if (nodeTClose > closestDistance)
            continue;

        if (node.isLeaf()) {
            int first = node.leftFirst;
            int end = first + node.triCount;
            for (int i = first; i < end; ++i) {
                const Triangle &triangle = model.triangles[i];
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
                if (!found || distance < closestDistance) {
                    closestHit = {position, distance, i, vec2(u, v)};
                    closestDistance = distance;
                    found = true;
                }
            }
            continue;
        }

        int leftIdx = node.leftFirst;
        int rightIdx = node.leftFirst + 1;
        const BVHNode &leftChild = model.bvh.bvhNodes[leftIdx];
        const BVHNode &rightChild = model.bvh.bvhNodes[rightIdx];

        float leftClose;
        float rightClose;
        bool leftIntersect = slabIntersection(leftChild.aabb, start, dir, leftClose);
        bool rightIntersect = slabIntersection(rightChild.aabb, start, dir, rightClose);

        if (leftIntersect && rightIntersect) {
            if (leftClose < rightClose) {
                stack.push_back(rightIdx);
                stack.push_back(leftIdx);
            } else {
                stack.push_back(leftIdx);
                stack.push_back(rightIdx);
            }
        } else if (leftIntersect) {
            stack.push_back(leftIdx);
        } else if (rightIntersect) {
            stack.push_back(rightIdx);
        }
    }

    return found;
}

vec3 LambertianShader::directLight(const Intersection &hit, const Model &model, const Light &light) {
    vec3 textureColor(1, 1, 1);
    size_t textureIdx = model.triangles[hit.triangleIndex].textureIdx;
    if (textureIdx != -1) {
        const Triangle &triangle = model.triangles[hit.triangleIndex];
        vec2 uv = triangle.uv0 * (1 - hit.uv.x - hit.uv.y) + triangle.uv1 * hit.uv.x + triangle.uv2 * hit.uv.y;
        textureColor = model.textures[textureIdx].sample(uv);
    }

    vec3 r = light.position - hit.position;
    vec3 nUnit = model.triangles[hit.triangleIndex].normal;
    vec3 start = hit.position + nUnit * 1e-4f;
    Intersection reverse;
    if (closestIntersection(start, r, model, reverse)) {
        if (length(start - reverse.position) < length(r))
            return indirectLight * textureColor;
    }

    vec3 B = light.color / (float)(4 * M_PI * pow(length(r), 2));
    vec3 rUnit = normalize(r);
    vec3 D = B * glm::max(dot(rUnit, nUnit), 0.0f);

    return (D + indirectLight) * textureColor;
}

void LambertianShader::render(Uint32 *pixelBuffer, int width, int height, const Model &model, const Light &light,
                              const Camera &camera, std::atomic<bool> &shouldStopRenderThread) {
    vec3 right = normalize(cross(vec3(0.0f, 1.0f, 0.0f), camera.direction));
    vec3 up = normalize(cross(camera.direction, right));

    up = glm::rotate(up, camera.roll, camera.direction);
    right = glm::rotate(right, camera.roll, camera.direction);

    vec3 start = camera.position;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (shouldStopRenderThread)
                return;

            float dx = (float)x - width / 2.0f;
            float dy = (float)y - height / 2.0f;
            vec3 dir = normalize(camera.direction * camera.focalLength + right * dx + up * dy);

            Intersection closestHit;
            bool found = closestIntersection(start, dir, model, closestHit);

            vec3 color(0, 0, 0);
            if (found)
                color = model.triangles[closestHit.triangleIndex].color * directLight(closestHit, model, light);

            Uint8 r = Uint8(glm::clamp(255 * color.r, 0.f, 255.f));
            Uint8 g = Uint8(glm::clamp(255 * color.g, 0.f, 255.f));
            Uint8 b = Uint8(glm::clamp(255 * color.b, 0.f, 255.f));
            Uint32 rgba = (255 << 24) | (r << 16) | (g << 8) | b;
            pixelBuffer[y * width + x] = rgba;
        }
    }
}

bool LambertianShader::slabIntersection(const AABB &aabb, const glm::vec3 &start, const glm::vec3 &dir, float &tClose) {
    const vec3 l = aabb.min;
    const vec3 h = aabb.max;
    vec3 tiLow;
    vec3 tiHigh;

    const float eps = 1e-8f;

    for (int i = 0; i < 3; i++) {
        if (abs(dir[i]) < eps) {
            if (start[i] < l[i] || start[i] > h[i])
                return false;

            tiLow[i] = -std::numeric_limits<float>::infinity();
            tiHigh[i] = std::numeric_limits<float>::infinity();
        } else {
            tiLow[i] = (l[i] - start[i]) / dir[i];
            tiHigh[i] = (h[i] - start[i]) / dir[i];
        }
    }

    vec3 tiClose;
    vec3 tiFar;

    for (size_t i = 0; i < 3; i++) {
        tiClose[i] = glm::min(tiLow[i], tiHigh[i]);
        tiFar[i] = glm::max(tiLow[i], tiHigh[i]);
    }

    float tNear = glm::max(tiClose.x, glm::max(tiClose.y, tiClose.z));
    float tFar = glm::min(tiFar.x, glm::min(tiFar.y, tiFar.z));

    tClose = glm::max(tNear, 0.0f);
    return tFar >= tNear && tFar >= 0.0f;
}
