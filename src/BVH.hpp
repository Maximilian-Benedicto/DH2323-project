#ifndef BVH_HPP
#define BVH_HPP

#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include <limits>
#include "Triangle.hpp"

struct AABB {
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::infinity());

    glm::vec3 max = glm::vec3(-std::numeric_limits<float>::infinity());

    void grow(glm::vec3 p);

    void grow(const AABB &b);

    float area() const;
};

struct BVHNode {
    AABB aabb;

    int leftFirst, triCount;

    bool isLeaf() const {
        return triCount > 0;
    };
};

class BVH {
   public:
    std::vector<BVHNode> bvhNodes;

    int rootNodeIdx = 0;

    int nodesUsed = 0;

    BVH() = default;
    ~BVH() = default;

    BVH(std::vector<Triangle> &inputTriangles);

   private:
    void updateNodeBounds(int nodeIdx, std::vector<Triangle> &triangles);

    void subdivide(int nodeIdx, std::vector<Triangle> &triangles);
};

#endif