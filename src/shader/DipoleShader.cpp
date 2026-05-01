#define GLM_ENABLE_EXPERIMENTAL  // Enable experimental features in GLM, needed for rotate() function

#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <random>

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

DipoleShader::DipoleSample DipoleShader::samplePointMultipleScattering(
    const Model& model, const Intersection& closestHit) {

    // Initialize random generator once per thread for better performance
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Get hit information
    const vec3 normal = model.triangles[closestHit.triangleIndex].normal;
    const vec3 sigma_tr =
        model.triangles[closestHit.triangleIndex].material.sigma_tr;
    const vec3 center = closestHit.position;

    // Pick R, G, or B based on a uniform random number
    float channel_u = dist(gen);

    float chosen_sigma_tr;
    if (channel_u < 0.3333f) {
        chosen_sigma_tr = sigma_tr.x;  // Red
    } else if (channel_u < 0.6666f) {
        chosen_sigma_tr = sigma_tr.y;  // Green
    } else {
        chosen_sigma_tr = sigma_tr.z;  // Blue
    }

    // Sample distance using the chosen sigma_tt
    float u = std::max(dist(gen), 1e-7f);  // Avoid log(0)
    float d = std::log(u) / chosen_sigma_tr;

    // Calculate the combined distribution fucntion for all channels
    float pdf_R = sigma_tr.x * std::exp(-sigma_tr.x * d);
    float pdf_G = sigma_tr.y * std::exp(-sigma_tr.y * d);
    float pdf_B = sigma_tr.z * std::exp(-sigma_tr.z * d);

    float combined_pdf = (pdf_R + pdf_G + pdf_B) / 3.0f;

    // 4. Transform distance 'd' into a 3D position on the plane
    vec3 helper = (std::abs(normal.x) > 0.9f) ? vec3{0, 1, 0} : vec3{1, 0, 0};
    vec3 tangentU = normalize(cross(helper, normal));
    vec3 tangentV = cross(normal, tangentU);

    float theta = 2.0f * M_PI * dist(gen);
    float localX = d * std::cos(theta);
    float localY = d * std::sin(theta);

    vec3 sampledPos = center + (tangentU * localX) + (tangentV * localY);

    return {sampledPos, combined_pdf};
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

        // Calculate Incident Radiance (Li) and direction (wi)
        vec3 xi = sample.position;
        vec3 wi = normalize(light.position - xi);

        // Check if the sample point is in shadow
        vec3 r = light.position - xi;
        vec3 nUnit = normal;
        vec3 start = xi + nUnit * 1e-4f;
        Intersection reverse;
        if (closestIntersection(start, r, model, reverse)) {
            if (length(start - reverse.position) < length(r))
                continue;  // In shadow, skip this sample
        }

        // Compute the incident radiance at the sample point
        float dist2 = glm::max(dot(r, r), 1e-6f);
        vec3 Li = light.color / dist2;

        // Calculate how much light enters the medium
        float cosThetaI = glm::max(0.0f, glm::dot(normal, wi));
        float Ft_xi = fresnelTransmittance(cosThetaI, material);
        glm::vec3 enteringIrradiance = Li * Ft_xi * cosThetaI;

        // Evaluate the BSSRDF
        glm::vec3 Rd = diffuseReflectance(scalarDistance(xi, xo), material);

        // Accumulate the contribution from this sample, weighted by the PDF
        result += (enteringIrradiance * Rd) / sample.pdf;
    }

    // Final Average and Normalization
    result = result / (float)samples.size();

    // Exit the medium and apply Fresnel at the exit point
    float cosThetaO = glm::max(0.0f, glm::dot(normal, wo));
    float Ft_xo = fresnelTransmittance(cosThetaO, material);

    // Divide by PI to convert radiant exitance to radiance[cite: 1]
    return result * (Ft_xo / 3.14159265f);
}

DipoleShader::DipoleSample DipoleShader::samplePointSingleScattering(
    const Model& model, const Intersection& closestHit, const Light& light,
    const vec3& viewDir) {

    // Initialize random generator once per thread for better performance
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    const vec3 xo = closestHit.position;
    const vec3 wo = normalize(viewDir);

    // Compute extinction sigma_t = sigma_a + sigma_s (recover sigma_s from sigma_s_prime)
    const vec3 sigma_a =
        model.triangles[closestHit.triangleIndex].material.sigma_a;
    const vec3 sigma_s_prime =
        model.triangles[closestHit.triangleIndex].material.sigma_s_prime;
    const vec3 g = vec3(0.8f);
    const vec3 sigma_s = sigma_s_prime / (1.0f - g);
    const vec3 sigma_t = sigma_a + sigma_s;

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
    float u = std::max(dist(gen), 1e-7f);
    float d = std::log(u) / chosen_sigma_t;

    float pdf_R = sigma_t.x * std::exp(-sigma_t.x * d);
    float pdf_G = sigma_t.y * std::exp(-sigma_t.y * d);
    float pdf_B = sigma_t.z * std::exp(-sigma_t.z * d);
    float combined_pdf = (pdf_R + pdf_G + pdf_B) / 3.0f;

    // Calculate the xi position
    const vec3 start = xo + wo * d;
    const vec3 dir = normalize(light.position - start);
    Intersection shadowTest;
    if (!closestIntersection(start, dir, model, shadowTest))
        return {vec3(0.0f), 1.0f};  // No intersection, return zero contribution
    vec3 xi = shadowTest.position;

    return {xi, combined_pdf};
}

vec3 DipoleShader::calculateSingleScattering(
    const Intersection& closestHit, const Model& model, const Light& light,
    const vector<DipoleSample>& samples, const vec3& viewDir) {

    return vec3(0.0f);  // Placeholder implementation, to be filled in later
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
