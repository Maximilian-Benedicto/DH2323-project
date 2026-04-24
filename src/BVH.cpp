
#include "BVH.hpp"
#include "Triangle.hpp"

void AABB::grow(glm::vec3 p) {
    min = glm::min(min, p);
    max = glm::max(max, p);
}

void AABB::grow(const AABB &b) {
    // Only grow if b is valid
    if (b.min.x != std::numeric_limits<float>::infinity()) {
        grow(b.min);
        grow(b.max);
    }
}

BVH::BVH(std::vector<Triangle> &triangles) {
    if (triangles.empty()) {
        nodesUsed = 0;
        return;
    }
    nodesUsed = 1;

    // Allocate upper bound on number of nodes in the BVH.
    bvhNodes.resize(triangles.size() * 2 - 1);

    // Initialize the root node to contain all triangles
    BVHNode &root = bvhNodes[rootNodeIdx];
    root.leftFirst = 0;
    root.triCount = triangles.size();

    // Recursively build the BVH
    updateNodeBounds(rootNodeIdx, triangles);
    subdivide(rootNodeIdx, triangles);
}

void BVH::updateNodeBounds(int nodeIdx, std::vector<Triangle> &triangles) {
    BVHNode &node = bvhNodes[nodeIdx];
    node.aabb.min = glm::vec3(std::numeric_limits<float>::infinity());
    node.aabb.max = glm::vec3(-std::numeric_limits<float>::infinity());

    // Grow the bounding box to include all triangles in this node
    for (int i = 0; i < node.triCount; i++) {
        const Triangle &leafTriangle = triangles[node.leftFirst + i];
        node.aabb.grow(leafTriangle.v0);
        node.aabb.grow(leafTriangle.v1);
        node.aabb.grow(leafTriangle.v2);
    }
}

void BVH::subdivide(int nodeIdx, std::vector<Triangle> &triangles) {
    BVHNode &node = bvhNodes[nodeIdx];

    // Stop subdividing at 2 triangles per leaf node
    if (node.triCount <= 2)
        return;

    // Compute the bounding box of the triangle centroids to determine the split axis and position
    AABB centroidBounds;
    for (int i = 0; i < node.triCount; i++) {
        centroidBounds.grow(triangles[node.leftFirst + i].centroid);
    }

    // Choose the axis with the largest extent to split along
    glm::vec3 extent = centroidBounds.max - centroidBounds.min;
    int axis = 0;
    if (extent.y > extent.x)
        axis = 1;
    if (extent.z > extent[axis])
        axis = 2;
    float splitPos = centroidBounds.min[axis] + extent[axis] * 0.5f;

    // Swap triangles at the wrong side of the split
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
