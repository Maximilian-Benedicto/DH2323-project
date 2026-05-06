#define GLM_ENABLE_EXPERIMENTAL  // Enable experimental features in GLM, needed for rotate() function

#include <cmath>
#include <glm/gtx/rotate_vector.hpp>
#include <random>
#include <thread>

#include "Camera.hpp"
#include "DipoleHelper.hpp"
#include "DipoleShader.hpp"
#include "Light.hpp"
#include "Triangle.hpp"

using namespace glm;
using namespace std;

void DipoleShader::render(Uint32* pixelBuffer, int width, int height, const Model& model,
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

void DipoleShader::renderSquare(
    Uint32* pixelBuffer, int width, int height, int x1, int y1, int x2, int y2, const Model& model,
    const Light& light, const Camera& camera,
    std::atomic<bool>& shouldStopRenderThread) {  // Compute the cameras right and up vectors
    vec3 right = normalize(cross(vec3(0.0f, 1.0f, 0.0f), camera.direction));
    vec3 up = normalize(cross(camera.direction, right));

    // Apply camera roll and direction
    up = glm::rotate(up, camera.roll, camera.direction);
    right = glm::rotate(right, camera.roll, camera.direction);

    // Rays start at the camera position
    vec3 start = camera.position;

    for (int y = y1; y < y2; ++y) {
        for (int x = x1; x < x2; ++x) {
            if (shouldStopRenderThread)
                return;

            // Compute the ray direction for the current pixel
            float dx = (float)x - width / 2.0f;
            float dy = (float)y - height / 2.0f;
            vec3 dir = normalize(camera.direction * camera.focalLength + right * dx + up * dy);

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
                    vector<DipoleSample>(MULTIPLE_SCATTER_SAMPLES);
                for (DipoleSample& sample : multipleScatterSamples)
                    sample = samplePointMultipleScattering(model, closestHit);
                multipleScatterColor = calculateMultipleScattering(closestHit, model, light,
                                                                   multipleScatterSamples, viewDir);

                // Sample points for single scattering and calculate the single scattering contribution
                vector<DipoleSample> singleScatterSamples =
                    vector<DipoleSample>(SINGLE_SCATTER_SAMPLES);
                for (DipoleSample& sample : singleScatterSamples)
                    sample = samplePointSingleScattering(model, closestHit, light, viewDir);
                singleScatterColor = calculateSingleScattering(closestHit, model, light,
                                                               singleScatterSamples, viewDir);
            }

            vec3 color(0.0f);
            if (mode == SINGLE_SCATTER)
                color = singleScatterColor;
            else if (mode == MULTIPLE_SCATTER)
                color = multipleScatterColor;
            else if (mode == FULL)
                color = singleScatterColor + multipleScatterColor;

            Uint8 r = Uint8(glm::clamp(255 * color.r, 0.f, 255.f));
            Uint8 g = Uint8(glm::clamp(255 * color.g, 0.f, 255.f));
            Uint8 b = Uint8(glm::clamp(255 * color.b, 0.f, 255.f));
            Uint32 rgba = (255 << 24) | (r << 16) | (g << 8) | b;
            pixelBuffer[y * width + x] = rgba;
        }
    }
}

// MULTIPLE SCATTERING FUNCTIONS:

DipoleShader::DipoleSample DipoleShader::samplePointMultipleScattering(
    const Model& model, const Intersection& closestHit) {

    // Initialize random generator once per thread for better performance
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Get the material properties at the exit point
    const Material material = model.triangles[closestHit.triangleIndex].material;
    const float sigma_tr = DipoleScattering::average(material.sigma_tr);

    // Exit point and normal
    const vec3 xo = closestHit.position;
    const vec3 no = normalize(model.triangles[closestHit.triangleIndex].normal);

    // Find the two orthogonal vectors to the normal to find the tangent plane (for sampling points on the disk around the exit point)
    const vec3 helper = std::abs(no.x) > 0.9 ? vec3(0, 1, 0) : vec3(1, 0, 0);
    const vec3 tangent = normalize(cross(helper, no));
    const vec3 bitangent = normalize(cross(no, tangent));

    // Sample points on a disk around the exit point until we find one that intersects with the model
    for (int attempt = 0; attempt < MAX_DISK_SAMPLE_ATTEMPTS; ++attempt) {

        // Sample distance r and angle theta
        const float u1 = std::max(dist(gen), 1e-7f);
        const float u2 = dist(gen);

        // Clamp to minimum 1/sigma_t_prime as suggested in Jensen's paper
        const float sigma_t_prime = DipoleScattering::average(material.sigma_t_prime);
        const float r_min = 1.0f / std::max(sigma_t_prime, 1e-7f);

        // Sample r using the exponential distribution and theta uniformly
        const float r = r_min - std::log(u1) / sigma_tr;
        const float theta = 2.0f * (float)M_PI * u2;

        // Convert to world coordinates
        const vec3 offset = tangent * (r * std::cos(theta)) + bitangent * (r * std::sin(theta));

        // Calculate the ray from the sampled point towards the model
        // Radius based offset to handle curved surfaces
        const float heightOffset = std::max(r, 0.01f);
        const vec3 start = xo + offset + heightOffset * no;
        const vec3 dir = -no;

        // Check if this ray intersects with the model
        Intersection hit;
        if (!closestIntersection(start, dir, model, hit))
            continue;

        // Area PDF for the exponential radial sampling
        const float pdf_r = sigma_tr * std::exp(-sigma_tr * (r - r_min));
        const float pdf = pdf_r / (2.0f * (float)M_PI * r);

        return {hit.position, pdf, 0.0f, 0.0f, hit.triangleIndex};
    }

    return {vec3(0.0f), 0.0f, 0.0f, 0.0f, -1};
}

vec3 DipoleShader::calculateMultipleScattering(const Intersection& closestHit, const Model& model,
                                               const Light& light,
                                               const vector<DipoleSample>& samples,
                                               const vec3& viewDir) {

    vec3 result(0.0f);
    const Material material = model.triangles[closestHit.triangleIndex].material;

    // Exit point, direction and normal
    const vec3 xo = closestHit.position;
    const vec3 wo = normalize(viewDir);
    const vec3 no = model.triangles[closestHit.triangleIndex].normal;

    // Calculate the Fresnel transmittance at the exit point
    const float cosThetaO = glm::max(0.0f, glm::dot(no, wo));
    const float Fto = DipoleScattering::fresnelTransmittance(cosThetaO, material);

    for (const DipoleSample& sample : samples) {
        if (sample.triangleIndex < 0)
            continue;

        // Entry point, direction, and normal
        const vec3 xi = sample.position;
        const vec3 wi = normalize(light.position - xi);
        const vec3 ni = model.triangles[sample.triangleIndex].normal;

        // Check if the sample point is in shadow
        const vec3 lightRay = light.position - xi;
        Intersection reverse;
        if (closestIntersection(xi + ni * 1e-3f, normalize(lightRay), model, reverse)) {
            if (reverse.distance < length(lightRay))
                continue;  // In shadow, skip this sample
        }

        // Incident radiance at the sample point
        const vec3 Li = light.color / glm::max(length(lightRay) * length(lightRay), 1e-6f);

        // Calculate how much light enters the medium
        const float cosThetaI = glm::max(0.0f, glm::dot(ni, wi));
        const float Fti = DipoleScattering::fresnelTransmittance(cosThetaI, material);

        // Evaluate the BSSRDF
        const vec3 Rd = DipoleScattering::diffuseReflectance(
            DipoleScattering::scalarDistance(xi, xo), material);

        // Calculate the contribution from this sample
        const vec3 contribution = cosThetaI * (float)(1 / M_PI) * Li * Fti * Rd * Fto;

        // Divide by the PDF of this sample
        result += contribution / sample.pdf;
    }

    // Final average over the number of valid samples
    return result / (float)samples.size();
}

// SINGLE SCATTERING FUNCTIONS:

DipoleShader::DipoleSample DipoleShader::samplePointSingleScattering(const Model& model,
                                                                     const Intersection& closestHit,
                                                                     const Light& light,
                                                                     const vec3& viewDir) {

    // Initialize random generator once per thread for better performance
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Exit point and direction
    const vec3 xo = closestHit.position;
    const vec3 wo = normalize(viewDir);

    // Get the material properties at the exit point
    const Material material = model.triangles[closestHit.triangleIndex].material;
    const float sigma_t = DipoleScattering::average(material.sigma_a + material.sigma_s);

    // Sample distance using the average sigma_t across the three channels
    const float u = std::max(dist(gen), 1e-7f);
    const float d = -std::log(u) / std::max(sigma_t, 1e-7f);

    // Calculate the PDF for the sampled distance
    const float pdf = sigma_t * std::exp(-sigma_t * d);

    // Calculate the xi position
    const vec3 scatterPoint = xo - wo * d;
    const vec3 dir = normalize(light.position - scatterPoint);
    Intersection shadowTest;
    if (!closestIntersection(scatterPoint, dir, model, shadowTest))
        return {vec3(0.0f), 1.0f, 0.0f, 0.0f, -1};  // Ray doesnt exit the medium
    vec3 xi = shadowTest.position;

    // Calculate the distances from the scatter point to the entry and exit points
    float s_i = length(scatterPoint - xi);
    float s_o = length(scatterPoint - xo);

    return {xi, pdf, s_i, s_o, shadowTest.triangleIndex};
}

vec3 DipoleShader::calculateSingleScattering(const Intersection& closestHit, const Model& model,
                                             const Light& light,
                                             const vector<DipoleSample>& samples,
                                             const vec3& viewDir) {

    vec3 result(0.0f);
    const Material material = model.triangles[closestHit.triangleIndex].material;

    // Exit direction and normal
    const vec3 wo = normalize(viewDir);
    const vec3 no = model.triangles[closestHit.triangleIndex].normal;

    // Calculate the Fresnel transmittance at the exit point
    const float cosThetaO = glm::max(0.0f, glm::dot(no, wo));
    const float Fto = DipoleScattering::fresnelTransmittance(cosThetaO, material);

    for (const DipoleSample& sample : samples) {
        if (sample.triangleIndex < 0)
            continue;

        // Entry point, direction and normal
        const vec3 xi = sample.position;
        const vec3 r_vec = light.position - xi;
        const float distToLight = length(r_vec);
        const vec3 wi = r_vec / distToLight;
        const vec3 ni = model.triangles[sample.triangleIndex].normal;

        // Check if the sample point is in shadow
        const vec3 start = xi + ni * 1e-3f;  // larger offset
        Intersection reverse;
        if (closestIntersection(start, wi, model, reverse)) {
            if (reverse.distance < distToLight)
                continue;  // In shadow, skip this sample
        }

        // Incident radiance at the sample point
        const vec3 Li = light.color / glm::max(distToLight * distToLight, 1e-6f);

        // Calculate phase and material properties for the BSSRDF
        const float phase = DipoleScattering::phaseFunction(glm::dot(wi, wo));
        const vec3 sigma_t = material.sigma_a + material.sigma_s;
        const vec3 transmittance_i = glm::exp(-sigma_t * sample.s_i);
        const vec3 transmittance_o = glm::exp(-sigma_t * sample.s_o);

        // Calculate the Fresnel transmittance at the entry point
        const float cosThetaI = glm::max(0.0f, glm::dot(ni, wi));
        const float Fti = DipoleScattering::fresnelTransmittance(cosThetaI, material);

        // Calculate the contribution from this sample
        vec3 contribution =
            material.sigma_s * Li * Fti * Fto * phase * transmittance_i * transmittance_o;

        // Divide by the PDF of this sample
        result += contribution / sample.pdf;
    }

    // Final average over the number of samples
    return result / (float)samples.size();
}
