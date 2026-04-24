
#include "happly.h"
#include "PlyModel.hpp"
#include "Triangle.hpp"
#include <algorithm>
#include <glm/glm.hpp>
#include <vector>

void PlyModel::load() {
    using glm::vec3;
    using namespace std;

    happly::PLYData plyIn(filename);

    vector<array<double, 3>> vPos = plyIn.getVertexPositions();
    vector<vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();

    triangles.clear();
    triangles.reserve(fInd.size());
    for (vector<size_t> face : fInd) {
        vec3 v0((float)vPos[face[2]][0], (float)vPos[face[2]][1], (float)vPos[face[2]][2]);
        vec3 v1((float)vPos[face[1]][0], (float)vPos[face[1]][1], (float)vPos[face[1]][2]);
        vec3 v2((float)vPos[face[0]][0], (float)vPos[face[0]][1], (float)vPos[face[0]][2]);

        triangles.push_back(Triangle(v0, v1, v2, vec3(1.0f, 1.0f, 1.0f)));
    }

    scaleToUnitCube();
    bvh = BVH(triangles);
}

void PlyModel::scaleToUnitCube() {
    if (triangles.empty())
        return;

    glm::vec3 minPos(1e9f);
    glm::vec3 maxPos(-1e9f);
    for (const Triangle &triangle : triangles) {
        minPos = glm::min(minPos, glm::min(triangle.v0, glm::min(triangle.v1, triangle.v2)));
        maxPos = glm::max(maxPos, glm::max(triangle.v0, glm::max(triangle.v1, triangle.v2)));
    }

    glm::vec3 center = (minPos + maxPos) * 0.5f;
    glm::vec3 size = maxPos - minPos;
    float minAxisLength = std::min({size.x, size.y, size.z});
    float scaleSize = 2.0f / minAxisLength;

    for (Triangle &triangle : triangles) {
        triangle.v0 = (triangle.v0 - center) * scaleSize;
        triangle.v1 = (triangle.v1 - center) * scaleSize;
        triangle.v2 = (triangle.v2 - center) * scaleSize;

        triangle.v0.x *= -1;
        triangle.v1.x *= -1;
        triangle.v2.x *= -1;

        triangle.v0.y *= -1;
        triangle.v1.y *= -1;
        triangle.v2.y *= -1;

        triangle.computeNormal();

        triangle.computeCentroid();
    }
}