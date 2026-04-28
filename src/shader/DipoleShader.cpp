#define GLM_ENABLE_EXPERIMENTAL  // Enable experimental features in GLM, needed for rotate() function

#include <glm/gtx/rotate_vector.hpp>
#include <iostream>

#include "Camera.hpp"
#include "DipoleShader.hpp"
#include "Light.hpp"
#include "Triangle.hpp"

using namespace glm;
using namespace std;

DipoleShader::DipoleShader() {}

void DipoleShader::render(Uint32* pixelBuffer, int width, int height,
                          const Model& model, const Light& light,
                          const Camera& camera,
                          std::atomic<bool>& shouldStopRenderThread) {
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
            vec3 dir = normalize(camera.direction * camera.focalLength +
                                 right * dx + up * dy);

            Intersection closestHit;
            bool found = closestIntersection(start, dir, model, closestHit);

            vec3 multipleScatterColor(0.0f);
            vec3 singleScatterColor(0.0f);

            if (found) {
                const Triangle& hitTriangle =
                    model.triangles[closestHit.triangleIndex];
                multipleScatterColor =
                    multipleScattering(closestHit.position, -dir,
                                       closestHit.position, -dir, hitTriangle);
                singleScatterColor =
                    singleScattering(closestHit.position, -dir);
            }

            vec3 color = multipleScatterColor + singleScatterColor;

            Uint8 r = Uint8(glm::clamp(255 * color.r, 0.f, 255.f));
            Uint8 g = Uint8(glm::clamp(255 * color.g, 0.f, 255.f));
            Uint8 b = Uint8(glm::clamp(255 * color.b, 0.f, 255.f));
            Uint32 rgba = (255 << 24) | (r << 16) | (g << 8) | b;
            pixelBuffer[y * width + x] = rgba;
        }
    }
}

bool DipoleShader::closestIntersection(vec3 start, vec3 dir, const Model& model,
                                       Intersection& closestHit) {
    bool found = false;
    if (model.bvh.nodesUsed == 0)
        return false;

    float closestDistance = std::numeric_limits<float>::infinity();
    std::vector<int> stack;
    stack.push_back(model.bvh.rootNodeIdx);

    while (!stack.empty()) {
        int nodeIdx = stack.back();
        stack.pop_back();

        const BVHNode& node = model.bvh.bvhNodes[nodeIdx];

        float nodeTClose;
        if (!slabIntersection(node.aabb, start, dir, nodeTClose))
            continue;
        if (nodeTClose > closestDistance)
            continue;

        if (node.isLeaf()) {
            int first = node.leftFirst;
            int end = first + node.triCount;
            for (int i = first; i < end; ++i) {
                const Triangle& triangle = model.triangles[i];
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
        const BVHNode& leftChild = model.bvh.bvhNodes[leftIdx];
        const BVHNode& rightChild = model.bvh.bvhNodes[rightIdx];

        float leftClose;
        float rightClose;
        bool leftIntersect =
            slabIntersection(leftChild.aabb, start, dir, leftClose);
        bool rightIntersect =
            slabIntersection(rightChild.aabb, start, dir, rightClose);

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

bool DipoleShader::slabIntersection(const AABB& aabb, const glm::vec3& start,
                                    const glm::vec3& dir, float& tClose) {
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

float DipoleShader::scalarDistance(vec3 xi, vec3 xo) {
    return length(xi - xo);
}

vec3 DipoleShader::positiveDistance(float r, const Material& material) {
    vec3 r_squared = vec3(r * r);
    vec3 z_r_squared = material.z_r * material.z_r;
    return sqrt(r_squared + z_r_squared);
}

vec3 DipoleShader::negativeDistance(float r, const Material& material) {
    vec3 r_squared = vec3(r * r);
    vec3 z_v_squared = material.z_v * material.z_v;
    return sqrt(r_squared + z_v_squared);
}

vec3 DipoleShader::diffuseReflectance(float r, const Material& material) {
    vec3 alpha_term = material.alpha_prime / (float)(4.0f * M_PI);
    vec3 z_r_term = material.sigma_tr * positiveDistance(r, material) + 1.0f;
    vec3 r_exp_term = exp(-material.sigma_tr * positiveDistance(r, material)) /
                      (material.sigma_t_prime *
                       pow(positiveDistance(r, material), vec3(3.0f)));
    vec3 z_v_term = material.z_v *
                    (material.sigma_tr * negativeDistance(r, material) + 1.0f);
    vec3 v_exp_term = exp(-material.sigma_tr * negativeDistance(r, material)) /
                      (material.sigma_t_prime *
                       glm::pow(negativeDistance(r, material), vec3(3.0f)));

    return alpha_term * (z_r_term * r_exp_term + z_v_term * v_exp_term);
}

float DipoleShader::fresnelReflectance(float cosTheta,
                                       const Material& material) {
    float r0 = pow((1.0f - material.eta) / (1.0f + material.eta), 2.0f);
    return r0 + (1.0f - r0) * pow(1.0f - cosTheta, 5.0f);
}

float DipoleShader::fresnelTransmittance(float cosTheta,
                                         const Material& material) {
    return 1.0f - fresnelReflectance(cosTheta, material);
}

vec3 DipoleShader::multipleScattering(vec3 xi, vec3 wi, vec3 xo, vec3 w0,
                                      const Triangle& triangle) {
    float cosTheta_i = dot(wi, triangle.normal);
    float fresnel_i = fresnelTransmittance(cosTheta_i, triangle.material);
    float cosTheta_o = dot(w0, triangle.normal);
    float fresnel_o = fresnelTransmittance(cosTheta_o, triangle.material);

    return ((float)(1.0f / M_PI) * fresnel_i * fresnel_o) *
           diffuseReflectance(scalarDistance(xi, xo), triangle.material);
}

vec3 DipoleShader::outgoingRadiance(vec3 xo, vec3 wo, vec3 xi, vec3 wi,
                                    vec3 wop, vec3 wip) {
    return vec3(0.0f);
}

vec3 DipoleShader::incidentRadiance(vec3 xi, vec3 wi) {
    return vec3(0.0f);
}

float DipoleShader::fresnel(vec3 wo, vec3 wi) {
    return 0.0f;
}

float DipoleShader::exponential(vec3 xo, vec3 xi) {
    return 0.0f;
}

float DipoleShader::geometryFactor(vec3 wop, vec3 wip) {
    return 0.0f;
}

vec3 DipoleShader::singleScattering(vec3 xo, vec3 wo) {
    return vec3(0.0f);
}

float DipoleShader::phaseFunction(float cosTheta) {
    return 0.0f;
}
