
#include "BVH.hpp"
#include "Triangle.hpp"

void AABB::grow(glm::vec3 p) {
    min = glm::min(min, p);
    max = glm::max(max, p);
}

void AABB::grow(const AABB &b) {
    if (b.min.x != std::numeric_limits<float>::infinity()) {
        grow(b.min);
        grow(b.max);
    }
}

float AABB::area() const {
    glm::vec3 e = max - min;
    return e.x * e.y + e.y * e.z + e.z * e.x;
}

BVH::BVH(std::vector<Triangle> &triangles) {
    if (triangles.empty()) {
        nodesUsed = 0;
        return;
    }

    nodesUsed = 1;

    bvhNodes.resize(triangles.size() * 2 - 1);

    BVHNode &root = bvhNodes[rootNodeIdx];
    root.leftFirst = 0;
    root.triCount = triangles.size();

    updateNodeBounds(rootNodeIdx, triangles);
    subdivide(rootNodeIdx, triangles);
}

void BVH::updateNodeBounds(int nodeIdx, std::vector<Triangle> &triangles) {
    BVHNode &node = bvhNodes[nodeIdx];
    node.aabb.min = glm::vec3(std::numeric_limits<float>::infinity());
    node.aabb.max = glm::vec3(-std::numeric_limits<float>::infinity());

    for (int i = 0; i < node.triCount; i++) {
        const Triangle &leafTriangle = triangles[node.leftFirst + i];
        node.aabb.grow(leafTriangle.v0);
        node.aabb.grow(leafTriangle.v1);
        node.aabb.grow(leafTriangle.v2);
    }
}

void BVH::subdivide(int nodeIdx, std::vector<Triangle> &triangles) {
    BVHNode &node = bvhNodes[nodeIdx];

    if (node.triCount <= 2)
        return;

    AABB centroidBounds;
    for (int i = 0; i < node.triCount; i++) {
        centroidBounds.grow(triangles[node.leftFirst + i].centroid);
    }

    glm::vec3 extent = centroidBounds.max - centroidBounds.min;
    int axis = 0;
    if (extent.y > extent.x)
        axis = 1;
    if (extent.z > extent[axis])
        axis = 2;
    float splitPos = centroidBounds.min[axis] + extent[axis] * 0.5f;

    int i = node.leftFirst;
    int j = i + node.triCount - 1;
    while (i <= j) {
        if (triangles[i].centroid[axis] < splitPos) {
            i++;
        } else {
            std::swap(triangles[i], triangles[j--]);
        }
    }

    int leftCount = i - node.leftFirst;
    if (leftCount == 0 || leftCount == node.triCount)
        return;

    int leftChildIdx = nodesUsed++;
    int rightChildIdx = nodesUsed++;

    bvhNodes[leftChildIdx].leftFirst = node.leftFirst;
    bvhNodes[leftChildIdx].triCount = leftCount;

    bvhNodes[rightChildIdx].leftFirst = i;
    bvhNodes[rightChildIdx].triCount = node.triCount - leftCount;

    node.leftFirst = leftChildIdx;
    node.triCount = 0;

    updateNodeBounds(leftChildIdx, triangles);
    updateNodeBounds(rightChildIdx, triangles);

    subdivide(leftChildIdx, triangles);
    subdivide(rightChildIdx, triangles);
}
