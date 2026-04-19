
#include "happly.h"
#include "StanfordBunny.hpp"
#include "Triangle.hpp"
#include <glm/glm.hpp>
#include <vector>

/**
 * @brief Load the stanford bunny
 * @param triangles The vector to write the bunny's triangles into
 * @param resolution Triangle count multiplier (1 = 1K triangles, 2 = 4K, 3 = 16K, 4 = 70K)
 */
void StanfordBunny::Load()
{
    using glm::vec3;
    using namespace std;

    // Construct filename
    const string filename = [this]() -> string
    {
        switch (resolution)
        {
        case (Resolution::FULL):
            return "../model/bun_zipper.ply";
        case (Resolution::HIGH):
            return "../model/bun_zipper_res2.ply";
        case (Resolution::MEDIUM):
            return "../model/bun_zipper_res3.ply";
        case (Resolution::LOW):
            return "../model/bun_zipper_res4.ply";
        default:
            return "../model/bun_zipper_res4.ply";
        }
    }();

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
}

void StanfordBunny::ScaleToUnitCube()
{
    // Scale to the volume [-1,1]^3
    for (Triangle &triangle : triangles)
    {
        // 1. Scale by 10
        triangle.v0 *= 10.0f;
        triangle.v1 *= 10.0f;
        triangle.v2 *= 10.0f;

        // 2. Flip X and Y
        triangle.v0.x *= -1;
        triangle.v1.x *= -1;
        triangle.v2.x *= -1;

        triangle.v0.y *= -1;
        triangle.v1.y *= -1;
        triangle.v2.y *= -1;

        // 3. Translate down by 1 unit
        triangle.v0 += glm::vec3(0.0f, 1.0f, 0.0f);
        triangle.v1 += glm::vec3(0.0f, 1.0f, 0.0f);
        triangle.v2 += glm::vec3(0.0f, 1.0f, 0.0f);

        // 4. Recalculate normals
        triangle.ComputeNormal();
    }
}