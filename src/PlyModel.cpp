
#include "happly.h"
#include "PlyModel.hpp"
#include "Triangle.hpp"
#include <algorithm>
#include <glm/glm.hpp>
#include <vector>

/**
 * @brief Load the stanford bunny
 * @param triangles The vector to write the bunny's triangles into
 * @param resolution Triangle count multiplier (1 = 1K triangles, 2 = 4K, 3 = 16K, 4 = 70K)
 */
void PlyModel::Load()
{
    using glm::vec3;
    using namespace std;

    // Construct the data object by reading from file
    happly::PLYData plyIn(filename);

    // Get mesh data from the object
    vector<array<double, 3>> vPos = plyIn.getVertexPositions();
    vector<vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();

    // Write to triangle vector
    triangles.clear();
    triangles.reserve(fInd.size());
    for (vector<size_t> face : fInd)
    {
        vec3 v0((float)vPos[face[2]][0], (float)vPos[face[2]][1], (float)vPos[face[2]][2]);
        vec3 v1((float)vPos[face[1]][0], (float)vPos[face[1]][1], (float)vPos[face[1]][2]);
        vec3 v2((float)vPos[face[0]][0], (float)vPos[face[0]][1], (float)vPos[face[0]][2]);

        triangles.push_back(Triangle(v0, v1, v2, vec3(1.0f, 1.0f, 1.0f)));
    }

    ScaleToUnitCube();
    bvh = BVH(triangles);
}

void PlyModel::ScaleToUnitCube()
{
    if (triangles.empty())
        return;

    // 1. Find bounding box to calculate center and max axis length
    glm::vec3 minPos(1e9f);
    glm::vec3 maxPos(-1e9f);
    for (const Triangle &triangle : triangles)
    {
        minPos = glm::min(minPos, glm::min(triangle.v0, glm::min(triangle.v1, triangle.v2)));
        maxPos = glm::max(maxPos, glm::max(triangle.v0, glm::max(triangle.v1, triangle.v2)));
    }

    glm::vec3 center = (minPos + maxPos) * 0.5f;
    glm::vec3 size = maxPos - minPos;
    float minAxisLength = std::min({size.x, size.y, size.z});
    float scaleSize = 2.0f / minAxisLength;

    // Scale to the volume [-1,1]^3
    for (Triangle &triangle : triangles)
    {
        // Center the model and scale it uniformly
        triangle.v0 = (triangle.v0 - center) * scaleSize;
        triangle.v1 = (triangle.v1 - center) * scaleSize;
        triangle.v2 = (triangle.v2 - center) * scaleSize;

        // Flip X and Y
        triangle.v0.x *= -1;
        triangle.v1.x *= -1;
        triangle.v2.x *= -1;

        triangle.v0.y *= -1;
        triangle.v1.y *= -1;
        triangle.v2.y *= -1;

        // Recalculate normals
        triangle.ComputeNormal();

        // Recalculate centroids
        triangle.ComputeCentroid();
    }
}