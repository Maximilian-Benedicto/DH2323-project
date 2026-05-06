
#define GLM_ENABLE_EXPERIMENTAL  // Enable experimental features in GLM, needed for rotate() function

#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <thread>

#include "Camera.hpp"
#include "LambertianShader.hpp"
#include "Light.hpp"
#include "Model.hpp"
#include "Texture.hpp"
#include "Triangle.hpp"

using namespace glm;
using namespace std;

void LambertianShader::render(Uint32* pixelBuffer, int width, int height, const Model& model,
                              const Light& light, const Camera& camera,
                              std::atomic<bool>& shouldStopRenderThread) {

    // Render the image in 10 squares to allow for better multithreading
    int numSquaresX = (int)sqrt(NUM_THREADS);
    int numSquaresY = (int)sqrt(NUM_THREADS);
    int squareWidth = width / numSquaresX;
    int squareHeight = height / numSquaresY;
    vector<thread> threads;
    for (int i = 0; i < numSquaresX; ++i) {
        for (int j = 0; j < numSquaresY; ++j) {
            int x1 = i * squareWidth;
            int y1 = j * squareHeight;
            int x2 = (i + 1 == numSquaresX) ? width : (i + 1) * squareWidth;
            int y2 = (j + 1 == numSquaresY) ? height : (j + 1) * squareHeight;

            // Launch a thread to render this square of the image
            threads.emplace_back(std::thread([this, x1, y1, x2, y2, width, height, pixelBuffer,
                                              &model, &light, &camera, &shouldStopRenderThread]() {
                this->renderSquare(pixelBuffer, width, height, x1, y1, x2, y2, model, light, camera,
                                   shouldStopRenderThread);
            }));
        }
    }

    for (thread& t : threads)
        t.join();
}

void LambertianShader::renderSquare(Uint32* pixelBuffer, int width, int height, int x1, int y1,
                                    int x2, int y2, const Model& model, const Light& light,
                                    const Camera& camera,
                                    std::atomic<bool>& shouldStopRenderThread) {
    // Compute the camera's right and up vectors
    vec3 right = normalize(cross(vec3(0.0f, 1.0f, 0.0f), camera.direction));
    vec3 up = normalize(cross(camera.direction, right));

    // Apply camera roll
    up = glm::rotate(up, camera.roll, camera.direction);
    right = glm::rotate(right, camera.roll, camera.direction);

    // Cast rays from the camera
    vec3 start = camera.position;

    for (int y = y1; y < y2; ++y) {
        for (int x = x1; x < x2; ++x) {
            if (shouldStopRenderThread)
                return;

            // Compute the ray direction for this pixel
            float dx = (float)x - width / 2.0f;
            float dy = (float)y - height / 2.0f;
            vec3 dir = normalize(camera.direction * camera.focalLength + right * dx + up * dy);

            // Find the closest intersection of the ray with the scene
            Intersection closestHit;
            bool found = closestIntersection(start, dir, model, closestHit);

            // Compute the color for this pixel based on the direct lighting at the intersection point
            vec3 color(0, 0, 0);
            if (found)
                color = model.triangles[closestHit.triangleIndex].color *
                        directLight(closestHit, model, light);

            // Convert the color to RGBA format and write it to the pixel buffer
            Uint8 r = Uint8(glm::clamp(255 * color.r, 0.f, 255.f));
            Uint8 g = Uint8(glm::clamp(255 * color.g, 0.f, 255.f));
            Uint8 b = Uint8(glm::clamp(255 * color.b, 0.f, 255.f));
            Uint32 rgba = (255 << 24) | (r << 16) | (g << 8) | b;
            pixelBuffer[y * width + x] = rgba;
        }
    }
}

vec3 LambertianShader::directLight(const Intersection& hit, const Model& model,
                                   const Light& light) {
    // Sample the texture color at the intersection point, if a texture is applied to the triangle
    vec3 textureColor(1, 1, 1);
    size_t textureIdx = model.triangles[hit.triangleIndex].textureIdx;
    if (textureIdx != (size_t)-1) {
        const Triangle& triangle = model.triangles[hit.triangleIndex];
        vec2 uv = triangle.uv0 * (1 - hit.uv.x - hit.uv.y) + triangle.uv1 * hit.uv.x +
                  triangle.uv2 * hit.uv.y;
        textureColor = model.textures[textureIdx].sample(uv);
    }

    // Check if the hit point is in shadow
    vec3 r = light.position - hit.position;
    vec3 nUnit = model.triangles[hit.triangleIndex].normal;
    vec3 start = hit.position + nUnit * 1e-4f;
    Intersection reverse;
    if (closestIntersection(start, r, model, reverse)) {
        if (length(start - reverse.position) < length(r))
            return indirectLight * textureColor;  // In shadow, only indirect light contributes
    }

    // Compute the direct lighting at the intersection point using the Lambertian reflectance model
    vec3 B = light.color / (float)(4 * M_PI * pow(length(r), 2));
    vec3 rUnit = normalize(r);
    vec3 D = B * glm::max(dot(rUnit, nUnit), 0.0f);

    return (D + indirectLight) * textureColor;
}
