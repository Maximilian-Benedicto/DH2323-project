#ifndef BVH_HPP
#define BVH_HPP

#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include "Triangle.hpp"

/**
 * @brief Axis-Aligned Bounding Box (AABB)
 */
struct AABB
{
    // Minimum corner of the box (initialized to a very large value so it can be grown)
    glm::vec3 min = glm::vec3(1e30f);

    // Maximum corner of the box (initialized to a very small value so it can be grown)
    glm::vec3 max = glm::vec3(-1e30f);

    // Grow the AABB to include a point
    void grow(glm::vec3 p);

    // Grow the AABB to include another AABB
    void grow(const AABB &b);

    // Surface area calculation
    float area() const;
};

/**
 * @brief A flat BVH Node structure.
 */
struct BVHNode
{
    // Bounding box for this node
    AABB aabb;

    /**
     * If this is a leaf node, leftFirst is the index of the first triangle in the leaf, and triCount is the number of triangles in the leaf.
     * If this is an internal node, leftFirst is the index of the left child node, and triCount is 0.
     */
    int leftFirst, triCount;

    bool isLeaf() const { return triCount > 0; };
};

/**
 * @brief Bounding Volume Hierarchy (BVH) for accelerating ray-triangle intersection tests.
 */
class BVH
{
public:
    std::vector<BVHNode> bvhNodes;
    std::vector<Triangle> triangles;
    int rootNodeIdx = 0;
    int nodesUsed = 1;

    ~BVH() = default;

    /**
     * @brief Build the BVH from a list of triangles. This will reorder the triangle list to improve spatial locality.
     * @param inputTriangles The list of triangles to build the BVH from (rearranged in-place)
     */
    BVH(std::vector<Triangle> &inputTriangles);

private:
    /**
     * @brief Update the bounding box of a node based on the triangles it contains (for leaf nodes) or its children (for internal nodes).
     * @param nodeIdx The index of the node to update
     */
    void updateNodeBounds(int nodeIdx);

    /**
     * @brief Recursively subdivide a node into two child nodes. This will reorder the triangle list to improve spatial locality.
     * @param nodeIdx The index of the node to subdivide
     */
    void subdivide(int nodeIdx);
};

#endif