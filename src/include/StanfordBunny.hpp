#ifndef STANFORDBUNNY_HPP
#define STANFORDBUNNY_HPP

#include "Triangle.hpp"
#include "happly.h"

/**
 * @brief A class that can load the Stanford Bunny as a list of triangles.
 */
class StanfordBunny
{
public:
    /**
     * @brief Load the stanford bunny
     * @param triangles The vector to write the bunny's triangles into
     * @param resolution Triangle count multiplier (1 = 1K triangles, 2 = 4K, 3 = 16K, 4 = 70K)
     */
    static void Load(std::vector<Triangle> &triangles, const int resolution)
    {
        using glm::vec3;
        using namespace std;

        // Construct filename
        string filename;
        if (resolution == 1)
            filename = "../model/bun_zipper_res4.ply";
        else if (resolution == 2)
            filename = "../model/bun_zipper_res3.ply";
        else if (resolution == 3)
            filename = "../model/bun_zipper_res2.ply";
        else if (resolution == 4)
            filename = "../model/bun_zipper.ply";
        else
            throw std::runtime_error("Invalid resolution argument provided");

        // Construct the data object by reading from file
        happly::PLYData plyIn(filename);

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
};

#endif