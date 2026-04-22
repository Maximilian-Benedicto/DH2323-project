
#include "BVH.hpp"
#include "Triangle.hpp"

void AABB::grow(glm::vec3 p)
{
    min = glm::min(min, p);
    max = glm::max(max, p);
}

void AABB::grow(const AABB &b)
{
    if (b.min.x != std::numeric_limits<float>::infinity()) // Only grow if b is valid
    {
        grow(b.min);
        grow(b.max);
    }
}

float AABB::area() const
{
    glm::vec3 e = max - min;
    return e.x * e.y + e.y * e.z + e.z * e.x;
}

BVH::BVH(std::vector<Triangle> &triangles)
{
    if (triangles.empty())
    {
        nodesUsed = 0;
        return;
    }

    nodesUsed = 1;

    // Allocate worst-case node count: 2N - 1
    bvhNodes.resize(triangles.size() * 2 - 1);

    // Initialize the root node
    BVHNode &root = bvhNodes[rootNodeIdx];
    root.leftFirst = 0;
    root.triCount = triangles.size();

    updateNodeBounds(rootNodeIdx, triangles);
    subdivide(rootNodeIdx, triangles);
}

void BVH::updateNodeBounds(int nodeIdx, std::vector<Triangle> &triangles)
{
    BVHNode &node = bvhNodes[nodeIdx];
    node.aabb.min = glm::vec3(std::numeric_limits<float>::infinity());
    node.aabb.max = glm::vec3(-std::numeric_limits<float>::infinity());

    // Loop over triangles in this node and grow the AABB to include them
    for (int i = 0; i < node.triCount; i++)
    {
        const Triangle &leafTriangle = triangles[node.leftFirst + i];
        node.aabb.grow(leafTriangle.v0);
        node.aabb.grow(leafTriangle.v1);
        node.aabb.grow(leafTriangle.v2);
    }
}

void BVH::subdivide(int nodeIdx, std::vector<Triangle> &triangles)
{
    BVHNode &node = bvhNodes[nodeIdx];

    // Terminate recursion if we have a small number of triangles
    if (node.triCount <= 2)
        return;

    // Find the bounding box of the triangle centroids
    AABB centroidBounds;
    for (int i = 0; i < node.triCount; i++)
    {
        centroidBounds.grow(triangles[node.leftFirst + i].centroid);
    }

    // Split along the longest axis of the centroid bounds
    glm::vec3 extent = centroidBounds.max - centroidBounds.min;
    int axis = 0;
    if (extent.y > extent.x)
        axis = 1;
    if (extent.z > extent[axis])
        axis = 2;
    float splitPos = centroidBounds.min[axis] + extent[axis] * 0.5f;

    // Start at each end of the list and swap triangles that are on the wrong side of the split
    int i = node.leftFirst;
    int j = i + node.triCount - 1;
    while (i <= j)
    {
        if (triangles[i].centroid[axis] < splitPos)
        {
            i++;
        }
        else
        {
            std::swap(triangles[i], triangles[j--]);
        }
    }

    // Handle edge case: if all triangles ended up on one side, stop subdividing
    int leftCount = i - node.leftFirst;
    if (leftCount == 0 || leftCount == node.triCount)
        return;

    // Create child nodes
    int leftChildIdx = nodesUsed++;
    int rightChildIdx = nodesUsed++;

    bvhNodes[leftChildIdx].leftFirst = node.leftFirst;
    bvhNodes[leftChildIdx].triCount = leftCount;

    bvhNodes[rightChildIdx].leftFirst = i;
    bvhNodes[rightChildIdx].triCount = node.triCount - leftCount;

    // Convert current node into an internal node
    node.leftFirst = leftChildIdx;
    node.triCount = 0;

    // Update bounds and recurse
    updateNodeBounds(leftChildIdx, triangles);
    updateNodeBounds(rightChildIdx, triangles);

    subdivide(leftChildIdx, triangles);
    subdivide(rightChildIdx, triangles);
}
