#ifndef BVH_HPP
#define BVH_HPP

#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include <limits>

#include "Triangle.hpp"

/// @brief Axis-aligned bounding box defined by its minimum and maximum corners
struct AABB {
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::infinity());
    glm::vec3 max = glm::vec3(-std::numeric_limits<float>::infinity());

    /// @brief Expand the bounding box to include a point
    /// @param p
    void grow(glm::vec3 p);

    /// @brief Expand the bounding box to include another bounding box
    /// @param b
    void grow(const AABB &b);
};

/// @brief Bounding Volume Hierarchy node
struct BVHNode {
    AABB aabb;

    /// @brief If leaf node, the index of the first triangle in the leaf. If internal node, the index of the left child.
    int leftFirst;

    /// @brief If leaf node, the number of triangles in the leaf. If internal node, 0.
    int triCount;

    /// @brief Whether this node is a leaf node (contains triangles) or an internal node (contains child nodes).
    /// @return true if this node is a leaf node, false if it is an internal node.
    bool isLeaf() const {
        return triCount > 0;
    };
};

/// @brief Bounding Volume Hierarchy for accelerating ray-triangle intersection tests.
class BVH {
   public:
    /// @brief Flat array of BVH nodes. The root node is always at index 0, and the children of node i are at indices
    /// 2*i+1 and 2*i+2.
    std::vector<BVHNode> bvhNodes;

    /// @brief Index of the root node in bvhNodes.
    int rootNodeIdx = 0;

    /// @brief Number of nodes used in the BVH.
    int nodesUsed = 0;

    BVH() = default;
    ~BVH() = default;

    /// @brief Construct a BVH from a list of triangles.
    /// @param inputTriangles Triangles will be reordered in-place to improve spatial locality.
    BVH(std::vector<Triangle> &inputTriangles);

   private:
    /// @brief Compute the bounding box of a BVH node based on the triangles it contains.
    /// @param nodeIdx Index of the node to update the bounds of.
    /// @param triangles List of triangles in the scene, which are used to compute the bounds of the node.
    void updateNodeBounds(int nodeIdx, std::vector<Triangle> &triangles);

    /// @brief Recursively subdivide a BVH node into two child nodes until a leaf node contains at most 2 triangles.
    /// @param nodeIdx Index of the node to subdivide.
    /// @param triangles List of triangles in the scene.
    void subdivide(int nodeIdx, std::vector<Triangle> &triangles);
};

#endif