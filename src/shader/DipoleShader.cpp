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

    // Compute the cameras right and up vectors
    vec3 right = normalize(cross(vec3(0.0f, 1.0f, 0.0f), camera.direction));
    vec3 up = normalize(cross(camera.direction, right));

    // Apply camera roll and direction
    up = glm::rotate(up, camera.roll, camera.direction);
    right = glm::rotate(right, camera.roll, camera.direction);

    // Rays start at the camera position
    vec3 start = camera.position;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (shouldStopRenderThread)
                return;

            // Compute the ray direction for the current pixel
            float dx = (float)x - width / 2.0f;
            float dy = (float)y - height / 2.0f;
            vec3 dir = normalize(camera.direction * camera.focalLength +
                                 right * dx + up * dy);

            // Find the closest intersection of the ray with the scene
            Intersection closestHit;
            bool found = closestIntersection(start, dir, model, closestHit);

            vec3 multipleScatterColor(0.0f);
            vec3 singleScatterColor(0.0f);

            if (found) {
                // Light rays are move towards the camera
                vec3 viewDir = -dir;

                // Sample points for multiple scattering and calculate the multiple scattering contribution
                vector<DipoleSample> multipleScatterSamples =
                    vector<DipoleSample>(100);
                for (DipoleSample& sample : multipleScatterSamples)
                    sample = samplePointMultipleScattering(model, closestHit);
                multipleScatterColor = calculateMultipleScattering(
                    closestHit, model, light, multipleScatterSamples, viewDir);

                // Sample points for single scattering and calculate the single scattering contribution
                vector<DipoleSample> singleScatterSamples =
                    vector<DipoleSample>(100);
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
    if (model.bvh.nodesUsed == 0)
        return false;
    bool found = false;
    float closestDistance = std::numeric_limits<float>::infinity();

    // Traverse the BVH using a stack
    std::vector<int> stack;
    stack.push_back(model.bvh.rootNodeIdx);
    while (!stack.empty()) {
        int nodeIdx = stack.back();
        stack.pop_back();

        const BVHNode& node = model.bvh.bvhNodes[nodeIdx];

        // Skip this node if the ray doesnt intersect with a closer hit than the closest one found so far
        float nodeTClose;
        if (!slabIntersection(node.aabb, start, dir, nodeTClose))
            continue;
        if (nodeTClose > closestDistance)
            continue;

        // If this is a leaf node, check for intersection with the triangles in this node
        if (node.isLeaf()) {
            int first = node.leftFirst;
            int end = first + node.triCount;
            for (int i = first; i < end; ++i) {
                const Triangle& triangle = model.triangles[i];

                // Solve the ray-triangle intersection (lab 2)
                vec3 v0 = triangle.v0;
                vec3 v1 = triangle.v1;
                vec3 v2 = triangle.v2;
                vec3 e1 = v1 - v0;
                vec3 e2 = v2 - v0;
                vec3 b = start - v0;
                mat3 A(-dir, e1, e2);
                vec3 x = inverse(A) * b;

                // Get the uv coordinates and distance
                float t = x.x;
                float u = x.y;
                float v = x.z;

                // Check if the intersection is valid
                if (!(0 <= t && 0 <= u && 0 <= v && u + v <= 1))
                    continue;

                // Update the closest hit if this intersection is closer than the closest one found so far
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

        // Get the child nodes
        int leftIdx = node.leftFirst;
        int rightIdx = node.leftFirst + 1;
        const BVHNode& leftChild = model.bvh.bvhNodes[leftIdx];
        const BVHNode& rightChild = model.bvh.bvhNodes[rightIdx];

        // Check for intersection with the child nodes
        float leftClose;
        float rightClose;
        bool leftIntersect =
            slabIntersection(leftChild.aabb, start, dir, leftClose);
        bool rightIntersect =
            slabIntersection(rightChild.aabb, start, dir, rightClose);

        // Add the child nodes that intersect with the ray to the stack, prioritizing the closer one
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
    // https://en.wikipedia.org/wiki/Slab_method (with modifications)

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
    const Material material =
        model.triangles[closestHit.triangleIndex].material;

    // Exit point, direction and normal
    const vec3 xo = closestHit.position;
    const vec3 wo = normalize(viewDir);
    const vec3 no = model.triangles[closestHit.triangleIndex].normal;

    // Calculate the Fresnel transmittance at the exit point
    const float cosThetaO = glm::max(0.0f, glm::dot(no, wo));
    const float Fto =
        DipoleScattering::fresnelTransmittance(cosThetaO, material);

    for (const DipoleSample& sample : samples) {
        if (sample.triangleIndex < 0)
            continue;

        // Entry point, direction, and normal
        const vec3 xi = sample.position;
        const vec3 wi = normalize(light.position - xi);
        const vec3 ni = model.triangles[sample.triangleIndex].normal;

        // Check if the sample point is in shadow
        const vec3 r = light.position - xi;
        const vec3 start = xi + ni * 1e-4f;
        Intersection reverse;
        if (closestIntersection(start, r, model, reverse)) {
            if (length(start - reverse.position) < length(r))
                continue;  // In shadow, skip this sample
        }

        // Incident radiance at the sample point
        const vec3 Li = light.color / glm::max(dot(r, r), 1e-6f);

        // Calculate how much light enters the medium
        const float cosThetaI = glm::max(0.0f, glm::dot(ni, wi));
        const float Fti =
            DipoleScattering::fresnelTransmittance(cosThetaI, material);

        // Evaluate the BSSRDF
        const vec3 Rd = DipoleScattering::diffuseReflectance(
            DipoleScattering::scalarDistance(xi, xo), material);

        // Calculate the contribution from this sample [Equation 5]
        const vec3 contribution =
            cosThetaI * (float)(1 / M_PI) * Li * Fti * Rd * Fto;

        // Divide by the PDF of this sample
        result += contribution / sample.pdf;
    }

    // Final average over the number of samples
    result = result / (float)samples.size();

    return result;
}

// SINGLE SCATTERING FUNCTIONS:

DipoleShader::DipoleSample DipoleShader::samplePointSingleScattering(
    const Model& model, const Intersection& closestHit, const Light& light,
    const vec3& viewDir) {

    // Initialize random generator once per thread for better performance
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Exit point and direction
    const vec3 xo = closestHit.position;
    const vec3 wo = normalize(viewDir);

    // Get the material properties at the exit point
    const Material material =
        model.triangles[closestHit.triangleIndex].material;
    const vec3 sigma_t = material.sigma_a + material.sigma_s;

    // Choose a random color channel to sample based on
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

    // Calculate the PDF for the sampled distance
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
                -1};  // Ray doesnt exit the medium
    vec3 xi = shadowTest.position;

    // Calculate the distances from the scatter point to the entry and exit points
    float s_i = length(scatterPoint - xi);
    float s_o = length(scatterPoint - xo);

    return {xi, combined_pdf, s_i, s_o, shadowTest.triangleIndex};
}

vec3 DipoleShader::calculateSingleScattering(
    const Intersection& closestHit, const Model& model, const Light& light,
    const vector<DipoleSample>& samples, const vec3& viewDir) {

    vec3 result(0.0f);
    const Material material =
        model.triangles[closestHit.triangleIndex].material;

    // Exit direction and normal
    const vec3 wo = normalize(viewDir);
    const vec3 no = model.triangles[closestHit.triangleIndex].normal;

    // Calculate the Fresnel transmittance at the exit point
    const float cosThetaO = glm::max(0.0f, glm::dot(no, wo));
    const float Fto =
        DipoleScattering::fresnelTransmittance(cosThetaO, material);

    for (const DipoleSample& sample : samples) {
        if (sample.triangleIndex < 0)
            continue;

        // Entry point, direction and normal
        const vec3 xi = sample.position;
        const vec3 wi = normalize(light.position - xi);
        const vec3 ni = model.triangles[sample.triangleIndex].normal;

        // Check if the sample point is in shadow
        const vec3 r = light.position - xi;
        const vec3 start = xi + ni * 1e-4f;
        Intersection reverse;
        if (closestIntersection(start, r, model, reverse)) {
            if (length(start - reverse.position) < length(r))
                continue;  // In shadow, skip this sample
        }

        // Incident radiance at the sample point
        const vec3 Li = light.color / glm::max(dot(r, r), 1e-6f);

        // Calculate phase and material properties for the BSSRDF
        const float phase = DipoleScattering::phaseFunction(glm::dot(wi, wo));
        /// TODO: Calculate sigma_tc using geometry constant G
        const vec3 sigma_t = material.sigma_a + material.sigma_s;
        const vec3 transmittance_i = glm::exp(-sigma_t * sample.s_i);
        const vec3 transmittance_o = glm::exp(-sigma_t * sample.s_o);

        // Calculate the Fresnel transmittance at the exit point
        const float cosThetaI = glm::max(0.0f, glm::dot(ni, wi));
        const float Fti =
            DipoleScattering::fresnelTransmittance(cosThetaI, material);

        // Calculate the contribution from this sample [Equation 6]
        vec3 contribution = cosThetaI * material.sigma_s * Li * Fti * Fto *
                            phase * transmittance_i * transmittance_o;

        // Divide by the PDF of this sample
        result += contribution / sample.pdf;
    }

    // Final average over the number of samples
    return result / (float)samples.size();
}
