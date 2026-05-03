#define GLM_ENABLE_EXPERIMENTAL  // Enable experimental features in GLM, needed for rotate() function

#include <cmath>
#include <glm/gtx/rotate_vector.hpp>
#include <random>

#include "Camera.hpp"
#include "DipoleHelper.hpp"
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
                vec3 viewDir = -dir;

                // Sample points for multiple scattering and calculate the multiple scattering contribution
                vector<DipoleSample> multipleScatterSamples =
                    vector<DipoleSample>(32);
                for (DipoleSample& sample : multipleScatterSamples)
                    sample = samplePointMultipleScattering(model, closestHit);
                multipleScatterColor = calculateMultipleScattering(
                    closestHit, model, light, multipleScatterSamples, viewDir);

                // Sample points for single scattering and calculate the single scattering contribution
                vector<DipoleSample> singleScatterSamples =
                    vector<DipoleSample>(32);
                for (DipoleSample& sample : singleScatterSamples)
                    sample = samplePointSingleScattering(model, closestHit,
                                                         light, viewDir);
                singleScatterColor = calculateSingleScattering(
                    closestHit, model, light, singleScatterSamples, viewDir);
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

// MULTIPLE SCATTERING FUNCTIONS:

DipoleShader::DipoleSample DipoleShader::samplePointMultipleScattering(
    const Model& model, const Intersection& closestHit) {

    // Initialize random generator once per thread for better performance
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Sample a triangle based on area
    float totalArea = 0.0f;
    int triangleIndex =
        DipoleScattering::sampleTriangleIndex(model, dist(gen), totalArea);
    if (triangleIndex < 0) {
        return {closestHit.position, 1.0f, 0.0f, 0.0f,
                closestHit.triangleIndex};
    }

    // Sample a point on the chosen triangle
    const Triangle& triangle = model.triangles[triangleIndex];
    vec3 sampledPos = triangle.samplePoint(dist(gen), dist(gen));
    float pdf = 1.0f / totalArea;

    return {sampledPos, pdf, 0.0f, 0.0f, triangleIndex};
}

vec3 DipoleShader::calculateMultipleScattering(
    const Intersection& closestHit, const Model& model, const Light& light,
    const vector<DipoleSample>& samples, const vec3& viewDir) {

    vec3 result(0.0f);
    const vec3 normal = model.triangles[closestHit.triangleIndex].normal;
    const Material material =
        model.triangles[closestHit.triangleIndex].material;
    const vec3 xo = closestHit.position;
    const vec3 wo = normalize(viewDir);

    for (const DipoleSample& sample : samples) {
        if (sample.triangleIndex < 0)
            continue;

        vec3 xi = sample.position;
        vec3 wi = normalize(light.position - xi);
        const vec3 normalXi = model.triangles[sample.triangleIndex].normal;

        // Check if the sample point is in shadow
        vec3 r = light.position - xi;
        vec3 start = xi + normalXi * 1e-4f;
        Intersection reverse;
        if (closestIntersection(start, r, model, reverse)) {
            if (length(start - reverse.position) < length(r))
                continue;  // In shadow, skip this sample
        }

        // Compute the incident radiance at the sample point
        float dist2 = glm::max(dot(r, r), 1e-6f);
        vec3 Li = light.color / dist2;

        // Calculate how much light enters the medium
        float cosThetaI = glm::max(0.0f, glm::dot(normalXi, wi));
        float Ft_xi =
            DipoleScattering::fresnelTransmittance(cosThetaI, material);
        glm::vec3 enteringIrradiance = Li * Ft_xi * cosThetaI;

        // Evaluate the BSSRDF
        glm::vec3 Rd = DipoleScattering::diffuseReflectance(
            DipoleScattering::scalarDistance(xi, xo), material);

        // Accumulate the contribution from this sample, weighted by the PDF
        result += (enteringIrradiance * Rd) / sample.pdf;
    }

    // Final Average and Normalization
    result = result / (float)samples.size();

    // Exit the medium and apply Fresnel at the exit point
    float cosThetaO = glm::max(0.0f, glm::dot(normal, wo));
    float Ft_xo = DipoleScattering::fresnelTransmittance(cosThetaO, material);

    // Divide by PI to convert radiant exitance to radiance[cite: 1]
    return result * (Ft_xo / 3.14159265f);
}

// SINGLE SCATTERING FUNCTIONS:

DipoleShader::DipoleSample DipoleShader::samplePointSingleScattering(
    const Model& model, const Intersection& closestHit, const Light& light,
    const vec3& viewDir) {

    // Initialize random generator once per thread for better performance
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    const vec3 xo = closestHit.position;
    const vec3 wo = normalize(viewDir);

    const Material material =
        model.triangles[closestHit.triangleIndex].material;
    const vec3 sigma_t = material.sigma_a + material.sigma_s;

    // Importance-sample distance per-channel (similar to multiple scattering sampling)
    float channel_u = dist(gen);
    float chosen_sigma_t;
    if (channel_u < 0.3333f)
        chosen_sigma_t = sigma_t.x;
    else if (channel_u < 0.6666f)
        chosen_sigma_t = sigma_t.y;
    else
        chosen_sigma_t = sigma_t.z;

    // Sample distance using the chosen sigma_t
    chosen_sigma_t = std::max(chosen_sigma_t, 1e-6f);
    float u = std::max(dist(gen), 1e-7f);
    float d = -std::log(u) / chosen_sigma_t;

    float pdf_R = sigma_t.x * std::exp(-sigma_t.x * d);
    float pdf_G = sigma_t.y * std::exp(-sigma_t.y * d);
    float pdf_B = sigma_t.z * std::exp(-sigma_t.z * d);
    float combined_pdf = (pdf_R + pdf_G + pdf_B) / 3.0f;

    // Calculate the xi position
    const vec3 scatterPoint = xo - wo * d;
    const vec3 dir = normalize(light.position - scatterPoint);
    Intersection shadowTest;
    if (!closestIntersection(scatterPoint, dir, model, shadowTest))
        return {vec3(0.0f), 1.0f, 0.0f, 0.0f,
                -1};  // No intersection, return zero contribution
    vec3 xi = shadowTest.position;

    float s_i = length(scatterPoint - xi);
    float s_o = length(scatterPoint - xo);

    return {xi, combined_pdf, s_i, s_o, shadowTest.triangleIndex};
}

vec3 DipoleShader::calculateSingleScattering(
    const Intersection& closestHit, const Model& model, const Light& light,
    const vector<DipoleSample>& samples, const vec3& viewDir) {

    vec3 result(0.0f);
    const vec3 normal = model.triangles[closestHit.triangleIndex].normal;
    const Material material =
        model.triangles[closestHit.triangleIndex].material;

    // Exit point direction
    const vec3 wo = normalize(viewDir);

    for (const DipoleSample& sample : samples) {
        if (sample.triangleIndex < 0)
            continue;

        // Entry point and direction
        const vec3 xi = sample.position;
        const vec3 wi = normalize(light.position - xi);
        const vec3 normalXi = model.triangles[sample.triangleIndex].normal;

        // Check if the sample point is in shadow
        vec3 r = light.position - xi;
        vec3 start = xi + normalXi * 1e-4f;
        Intersection reverse;
        if (closestIntersection(start, r, model, reverse)) {
            if (length(start - reverse.position) < length(r))
                continue;  // In shadow, skip this sample
        }

        // Compute the incident radiance at the sample point
        float dist2 = glm::max(dot(r, r), 1e-6f);
        vec3 Li = light.color / dist2;

        float cosTheta = glm::dot(wi, wo);
        float phase = DipoleScattering::phaseFunction(cosTheta);

        const vec3 sigma_t = material.sigma_a + material.sigma_s;
        vec3 transmittance_i = glm::exp(-sigma_t * sample.s_i);
        vec3 transmittance_o = glm::exp(-sigma_t * sample.s_o);

        float cosThetaI = glm::max(0.0f, glm::dot(normalXi, wi));
        float Ft_xi =
            DipoleScattering::fresnelTransmittance(cosThetaI, material);
        vec3 enteringIrradiance = Li * Ft_xi * cosThetaI;

        float cosThetaO = glm::max(0.0f, glm::dot(normal, wo));
        float Ft_xo =
            DipoleScattering::fresnelTransmittance(cosThetaO, material);

        vec3 contribution = enteringIrradiance * Ft_xo *
                            (material.sigma_s * phase) * transmittance_i *
                            transmittance_o;
        result += contribution / sample.pdf;
    }

    return result / (float)samples.size();
}
