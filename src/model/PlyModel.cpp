
#include <algorithm>
#include <glm/glm.hpp>
#include <vector>

#include "PlyModel.hpp"
#include "Triangle.hpp"
#include "happly.h"

void PlyModel::load() {
    using glm::vec3;
    using namespace std;

    happly::PLYData plyIn(filename);

    vector<array<double, 3>> vPos = plyIn.getVertexPositions();
    vector<vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();

    triangles.clear();
    triangles.reserve(fInd.size());

    // Create triangles from the vertex positions and face indices (order is flipped to maintain correct winding)
    for (vector<size_t> face : fInd) {
        vec3 v0((float)vPos[face[2]][0], (float)vPos[face[2]][1],
                (float)vPos[face[2]][2]);
        vec3 v1((float)vPos[face[1]][0], (float)vPos[face[1]][1],
                (float)vPos[face[1]][2]);
        vec3 v2((float)vPos[face[0]][0], (float)vPos[face[0]][1],
                (float)vPos[face[0]][2]);

        triangles.push_back(Triangle(v0, v1, v2, vec3(1.0f, 1.0f, 1.0f)));
    }

    scaleAndCenter();
    bvh = BVH(triangles);
}

void PlyModel::scaleAndCenter() {

    AABB bounds;
    for (const Triangle& triangle : triangles) {
        bounds.grow(triangle.v0);
        bounds.grow(triangle.v1);
        bounds.grow(triangle.v2);
    }

    for (Triangle& triangle : triangles) {
        // Center the model at the origin
        const glm::vec3 center = (bounds.min + bounds.max) / 2.0f;
        triangle.v0 -= center;
        triangle.v1 -= center;
        triangle.v2 -= center;

        // Resize the model by the provided scale factor
        triangle.v0 *= scale;
        triangle.v1 *= scale;
        triangle.v2 *= scale;

        triangle.computeNormal();

        triangle.computeCentroid();
    }
}