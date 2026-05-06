#include "Shader.hpp"

using namespace glm;

bool Shader::closestIntersection(vec3 start, vec3 dir, const Model& model,
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
        bool leftIntersect = slabIntersection(leftChild.aabb, start, dir, leftClose);
        bool rightIntersect = slabIntersection(rightChild.aabb, start, dir, rightClose);

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

bool Shader::slabIntersection(const AABB& aabb, const vec3& start, const vec3& dir, float& tClose) {
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
        tiClose[i] = min(tiLow[i], tiHigh[i]);
        tiFar[i] = max(tiLow[i], tiHigh[i]);
    }

    float tNear = max(tiClose.x, max(tiClose.y, tiClose.z));
    float tFar = min(tiFar.x, min(tiFar.y, tiFar.z));

    tClose = max(tNear, 0.0f);
    return tFar >= tNear && tFar >= 0.0f;
}