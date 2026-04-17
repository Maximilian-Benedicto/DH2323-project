
#include "Model.h"
#include "happly.h"

void Load(std::vector<Triangle> &triangles)
{
    using glm::vec3;
    using namespace std;

    // Construct the data object by reading from file
    happly::PLYData plyIn("../model/bun_zipper_res4.ply");

    // Get mesh-style data from the object
    vector<array<double, 3>> vPos = plyIn.getVertexPositions();
    vector<vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();

    // Write to triangle vector
    for (vector<size_t> face : fInd)
    {
        vec3 v0((float)vPos[face[0]][0], (float)vPos[face[0]][1], (float)vPos[face[0]][2]);
        vec3 v1((float)vPos[face[1]][0], (float)vPos[face[1]][1], (float)vPos[face[1]][2]);
        vec3 v2((float)vPos[face[2]][0], (float)vPos[face[2]][1], (float)vPos[face[2]][2]);
        triangles.emplace_back(Triangle(v0, v1, v2, vec3(1.0f, 1.0f, 1.0f)));
    }

    // Scale to the volume [-1,1]^3
    for (Triangle &t : triangles)
    {
        t.v0 *= vec3(10.f, -10.f, -10.f);
        t.v1 *= vec3(10.f, -10.f, -10.f);
        t.v2 *= vec3(10.f, -10.f, -10.f);
        t.v0 += vec3(0.f, 1.f, 0.f);
        t.v1 += vec3(0.f, 1.f, 0.f);
        t.v2 += vec3(0.f, 1.f, 0.f);
    }
}