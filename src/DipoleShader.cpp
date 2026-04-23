#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include "DipoleShader.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "Triangle.hpp"
#include <iostream>

using namespace glm;
using namespace std;

DipoleShader::DipoleShader() {}

/**
 * @brief Render a diagnostic dipole frame.
 * @details Current implementation visualizes hit normals while dipole scattering equations are stubbed.
 */
void DipoleShader::render(Uint32 *pixelBuffer, int width, int height, const Model &model, const Light &light,
                          const Camera &camera, std::atomic<bool> &shouldStopRenderThread) {
    // Compute camera basis vectors
    vec3 right = normalize(cross(vec3(0.0f, 1.0f, 0.0f), camera.direction));
    vec3 up = normalize(cross(camera.direction, right));

    // Apply roll rotation about camera direction
    up = glm::rotate(up, camera.roll, camera.direction);
    right = glm::rotate(right, camera.roll, camera.direction);

    vec3 start = camera.position;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Stop rendering early when a new frame is requested.
            if (shouldStopRenderThread)
                return;

            // Compute ray direction for this pixel
            float dx = (float)x - width / 2.0f;
            float dy = (float)y - height / 2.0f;
            vec3 dir = normalize(camera.direction * camera.focalLength + right * dx + up * dy);

            // Find closest intersection of ray with scene
            Intersection closestHit;
            bool found = closestIntersection(start, dir, model, closestHit);

            // TODO: call dipole model functions to compute color
            // For now, just visualize the normal of the intersected triangle
            vec3 color(0.0f, 0.0f, 0.0f);
            if (found)
                color = model.triangles[closestHit.triangleIndex].normal;

            // Write color to pixel buffer
            Uint8 r = Uint8(glm::clamp(255 * color.r, 0.f, 255.f));
            Uint8 g = Uint8(glm::clamp(255 * color.g, 0.f, 255.f));
            Uint8 b = Uint8(glm::clamp(255 * color.b, 0.f, 255.f));
            Uint32 rgba = (255 << 24) | (r << 16) | (g << 8) | b;
            pixelBuffer[y * width + x] = rgba;
        }
    }
}

/**
 * @brief Find the nearest ray hit using iterative BVH traversal.
 * @param start Ray origin.
 * @param dir Ray direction.
 * @param model Scene model containing triangles and BVH.
 * @param closestHit Output nearest hit when true is returned.
 * @return True if any triangle is intersected.
 */
bool DipoleShader::closestIntersection(vec3 start, vec3 dir, const Model &model, Intersection &closestHit) {
    bool found = false;
    if (model.bvh.nodesUsed == 0)
        return false;

    float closestDistance = std::numeric_limits<float>::infinity();
    std::vector<int> stack;
    stack.push_back(model.bvh.rootNodeIdx);

    // Iterative BVH traversal using a stack
    while (!stack.empty()) {
        int nodeIdx = stack.back();
        stack.pop_back();

        const BVHNode &node = model.bvh.bvhNodes[nodeIdx];

        // Skip nodes that dont intersect or are farther than the closest intersection found so far
        float nodeTClose;
        if (!slabIntersection(node.aabb, start, dir, nodeTClose))
            continue;
        if (nodeTClose > closestDistance)
            continue;

        // If this is a leaf node, check for intersection with each triangle
        if (node.isLeaf()) {
            int first = node.leftFirst;
            int end = first + node.triCount;
            for (int i = first; i < end; ++i) {
                // Solve ray-triangle intersection in barycentric coordinates.
                const Triangle &triangle = model.triangles[i];
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
                if (!found || distance < closestDistance) {
                    // Update closest intersection
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
            // Traverse the closer child first to increase chances of early termination
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

/**
 * @brief Test ray/AABB overlap with the slab method.
 * @param aabb Axis-aligned bounding box.
 * @param start Ray origin.
 * @param dir Ray direction.
 * @param tClose Earliest non-negative entry distance along the ray.
 * @return True when the ray overlaps the AABB in front of the origin.
 */
bool DipoleShader::slabIntersection(const AABB &aabb, const glm::vec3 &start, const glm::vec3 &dir, float &tClose) {
    // Source (https://en.wikipedia.org/wiki/Slab_method)

    // Get the low and high corners of the AABB
    const vec3 l = aabb.min;
    const vec3 h = aabb.max;
    vec3 tiLow;
    vec3 tiHigh;

    // Avoid division by zero by treating very small directions as parallel to the slab planes
    const float eps = 1e-8f;

    // Compute intersection t values for each pair of planes
    for (int i = 0; i < 3; i++) {
        // If the ray is parallel to the planes, check if the start point is between them
        if (abs(dir[i]) < eps) {
            if (start[i] < l[i] || start[i] > h[i])
                return false;

            // Set t values to -inf and +inf so they won't affect the final intersection interval
            tiLow[i] = -std::numeric_limits<float>::infinity();
            tiHigh[i] = std::numeric_limits<float>::infinity();
        } else {
            // Compute t values for the planes
            tiLow[i] = (l[i] - start[i]) / dir[i];
            tiHigh[i] = (h[i] - start[i]) / dir[i];
        }
    }

    vec3 tiClose;
    vec3 tiFar;

    // Get the near and far t values for the intersection interval
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
    return 0.0f;
}

float DipoleShader::positiveDistance(float r) {
    return 0.0f;
}

float DipoleShader::negativeDistance(float r) {
    return 0.0f;
}

float DipoleShader::diffuseReflectance(float r) {
    return 0.0f;
}

float DipoleShader::fresnelReflectance(float cosTheta) {
    return 0.0f;
}

float DipoleShader::fresnelTransmittance(float cosTheta) {
    return 0.0f;
}

float DipoleShader::multipleScattering(vec3 xi, vec3 wi, vec3 xo, vec3 w0) {
    return 0.0f;
}

vec3 DipoleShader::outgoingRadiance(vec3 xo, vec3 wo, vec3 xi, vec3 wi, vec3 wop, vec3 wip) {
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
