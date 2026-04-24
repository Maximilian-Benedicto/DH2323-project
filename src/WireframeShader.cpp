#include "WireframeShader.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "Triangle.hpp"
#include "Model.hpp"
#include <iostream>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

using namespace glm;
using namespace std;

WireframeShader::WireframeShader() {}

void WireframeShader::drawLine(Uint32 *pixelBuffer, int width, int height, int x0, int y0, int x1, int y1,
                               Uint32 color) {
    const int dx = abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;

    while (true) {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height)

            pixelBuffer[y0 * width + x0] = color;

        int e2 = 2 * error;
        if (e2 >= dy) {
            if (x0 == x1)
                break;
            error = error + dy;
            x0 = x0 + sx;
        }

        if (e2 <= dx) {
            if (y0 == y1)
                break;
            error = error + dx;
            y0 = y0 + sy;
        }
    }
}

void WireframeShader::render(Uint32 *pixelBuffer, int width, int height, const Model &model, const Light &light,
                             const Camera &camera, std::atomic<bool> &shouldStopRenderThread) {
    for (int i = 0; i < width * height; ++i) {
        if (shouldStopRenderThread)
            return;
        pixelBuffer[i] = 0xFF000000;
    }

    vec3 right = normalize(cross(vec3(0.0f, 1.0f, 0.0f), camera.direction));
    vec3 up = normalize(cross(camera.direction, right));
    up = glm::rotate(up, camera.roll, camera.direction);

    mat4 viewMatrix = glm::lookAt(camera.position, camera.position + camera.direction, up);

    if (model.bvh.nodesUsed == 0)
        return;

    const int nodeCount = std::min<int>(model.bvh.nodesUsed, model.bvh.bvhNodes.size());
    for (int idx = 0; idx < nodeCount; ++idx) {
        const BVHNode &node = model.bvh.bvhNodes[idx];

        if (shouldStopRenderThread)
            return;

        if (!node.isLeaf() && isShowingBvh) {
            vec3 min = node.aabb.min;
            vec3 max = node.aabb.max;

            if (min.x > max.x || min.y > max.y || min.z > max.z)
                continue;
            if (isinf(min.x) || isinf(min.y) || isinf(min.z) || isinf(max.x) || isinf(max.y) || isinf(max.z))
                continue;

            vec3 v0(min.x, min.y, min.z);
            vec3 v1(max.x, min.y, min.z);
            vec3 v2(min.x, max.y, min.z);
            vec3 v3(max.x, max.y, min.z);
            vec3 v4(min.x, min.y, max.z);
            vec3 v5(max.x, min.y, max.z);
            vec3 v6(min.x, max.y, max.z);
            vec3 v7(max.x, max.y, max.z);

            vec3 pts[24] = {v0, v1, v2, v3, v4, v5, v6, v7, v0, v2, v1, v3,
                            v4, v6, v5, v7, v0, v4, v1, v5, v2, v6, v3, v7};

            float boxSize = length(max - min);
            Uint32 alpha = std::clamp((int)(boxSize * 100.0f), 20, 255);
            Uint32 lightColor = (alpha << 24) | 0x000000FF;

            for (int i = 0; i < 12; ++i) {
                vec4 p1Cam4 = viewMatrix * vec4(pts[i * 2], 1.0f);
                vec3 p1Cam = vec3(p1Cam4);
                vec4 p2Cam4 = viewMatrix * vec4(pts[i * 2 + 1], 1.0f);
                vec3 p2Cam = vec3(p2Cam4);

                if (p1Cam.z < -0.1f && p2Cam.z < -0.1f) {
                    float px1 = p1Cam.x / p1Cam.z * camera.focalLength + width / 2.0f;
                    float py1 = -p1Cam.y / p1Cam.z * camera.focalLength + height / 2.0f;
                    float px2 = p2Cam.x / p2Cam.z * camera.focalLength + width / 2.0f;
                    float py2 = -p2Cam.y / p2Cam.z * camera.focalLength + height / 2.0f;

                    if ((px1 < 0 && px2 < 0) || (px1 >= width && px2 >= width) || (py1 < 0 && py2 < 0) ||
                        (py1 >= height && py2 >= height))
                        continue;

                    drawLine(pixelBuffer, width, height, (int)px1, (int)py1, (int)px2, (int)py2, lightColor);
                }
            }
        } else {
            const size_t start = node.leftFirst;
            const size_t end = node.leftFirst + node.triCount;

            for (size_t i = start; i < end; i++) {
                vec3 v[3] = {model.triangles[i].v0, model.triangles[i].v1, model.triangles[i].v2};
                int p[3][2];
                bool outOfBounds[3] = {false, false, false};
                for (int j = 0; j < 3; ++j) {
                    vec4 vCam4 = viewMatrix * vec4(v[j], 1.0f);
                    vec3 vCam = vec3(vCam4);

                    if (vCam.z >= -0.1f) {
                        outOfBounds[j] = true;
                        continue;
                    }

                    float px = vCam.x / vCam.z * camera.focalLength + width / 2.0f;
                    float py = -vCam.y / vCam.z * camera.focalLength + height / 2.0f;
                    p[j][0] = (int)px;
                    p[j][1] = (int)py;
                }

                if (!outOfBounds[0] && !outOfBounds[1])
                    drawLine(pixelBuffer, width, height, p[0][0], p[0][1], p[1][0], p[1][1], 0xFF00FF00);
                if (!outOfBounds[1] && !outOfBounds[2])
                    drawLine(pixelBuffer, width, height, p[1][0], p[1][1], p[2][0], p[2][1], 0xFF00FF00);
                if (!outOfBounds[2] && !outOfBounds[0])
                    drawLine(pixelBuffer, width, height, p[2][0], p[2][1], p[0][0], p[0][1], 0xFF00FF00);
            }
        }
    }

    if (!shouldStopRenderThread) {
        float size = 0.1f;
        vec3 pts[6] = {light.position + vec3(size, 0.0f, 0.0f), light.position - vec3(size, 0.0f, 0.0f),
                       light.position + vec3(0.0f, size, 0.0f), light.position - vec3(0.0f, size, 0.0f),
                       light.position + vec3(0.0f, 0.0f, size), light.position - vec3(0.0f, 0.0f, size)};

        Uint32 lightColor = 0xFFFFFF00;

        for (int i = 0; i < 3; ++i) {
            vec4 p1Cam4 = viewMatrix * vec4(pts[i * 2], 1.0f);
            vec3 p1Cam = vec3(p1Cam4);
            vec4 p2Cam4 = viewMatrix * vec4(pts[i * 2 + 1], 1.0f);
            vec3 p2Cam = vec3(p2Cam4);

            if (p1Cam.z < -0.1f && p2Cam.z < -0.1f) {
                float px1 = p1Cam.x / p1Cam.z * camera.focalLength + width / 2.0f;
                float py1 = -p1Cam.y / p1Cam.z * camera.focalLength + height / 2.0f;
                float px2 = p2Cam.x / p2Cam.z * camera.focalLength + width / 2.0f;
                float py2 = -p2Cam.y / p2Cam.z * camera.focalLength + height / 2.0f;

                drawLine(pixelBuffer, width, height, (int)px1, (int)py1, (int)px2, (int)py2, lightColor);
            }
        }
    }
}
