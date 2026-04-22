

#include <glm/glm.hpp>
#include <vector>
#include "Triangle.hpp"
#include "CornellBox.hpp"

/**
 * @brief Build Cornell Box geometry and acceleration structures.
 */
void CornellBox::load() {
    triangles.clear();
    triangles.reserve(5 * 2 * 3);

    room();
    shortBlock();
    tallBlock();
    scaleToUnitCube();
    bvh = BVH(triangles);
}

void CornellBox::room() {
    // Corners of the Cornell Box
    const glm::vec3 A(kL, 0, 0);
    const glm::vec3 B(0, 0, 0);
    const glm::vec3 C(kL, 0, kL);
    const glm::vec3 D(0, 0, kL);
    const glm::vec3 E(kL, kL, 0);
    const glm::vec3 F(0, kL, 0);
    const glm::vec3 G(kL, kL, kL);
    const glm::vec3 H(0, kL, kL);

    // Floor:
    triangles.push_back(Triangle(C, B, A, kGreen));
    triangles.push_back(Triangle(C, D, B, kGreen));

    // Left wall
    triangles.push_back(Triangle(A, E, C, kPurple));
    triangles.push_back(Triangle(C, E, G, kPurple));

    // Right wall
    triangles.push_back(Triangle(F, B, D, kYellow));
    triangles.push_back(Triangle(H, F, D, kYellow));

    // Ceiling
    triangles.push_back(Triangle(E, F, G, kCyan));
    triangles.push_back(Triangle(F, H, G, kCyan));

    // Back wall
    triangles.push_back(Triangle(G, D, C, kWhite));
    triangles.push_back(Triangle(G, H, D, kWhite));
}

void CornellBox::shortBlock() {
    // Corners of the short block
    const glm::vec3 A = glm::vec3(290, 0, 114);
    const glm::vec3 B = glm::vec3(130, 0, 65);
    const glm::vec3 C = glm::vec3(240, 0, 272);
    const glm::vec3 D = glm::vec3(82, 0, 225);
    const glm::vec3 E = glm::vec3(290, 165, 114);
    const glm::vec3 F = glm::vec3(130, 165, 65);
    const glm::vec3 G = glm::vec3(240, 165, 272);
    const glm::vec3 H = glm::vec3(82, 165, 225);

    // Front
    triangles.push_back(Triangle(E, B, A, kRed));
    triangles.push_back(Triangle(E, F, B, kRed));

    // Front
    triangles.push_back(Triangle(F, D, B, kRed));
    triangles.push_back(Triangle(F, H, D, kRed));

    // BACK
    triangles.push_back(Triangle(H, C, D, kRed));
    triangles.push_back(Triangle(H, G, C, kRed));

    // LEFT
    triangles.push_back(Triangle(G, E, C, kRed));
    triangles.push_back(Triangle(E, A, C, kRed));

    // TOP
    triangles.push_back(Triangle(G, F, E, kRed));
    triangles.push_back(Triangle(G, H, F, kRed));
}

void CornellBox::tallBlock() {
    // Corners of the tall block
    const glm::vec3 A = glm::vec3(423, 0, 247);
    const glm::vec3 B = glm::vec3(265, 0, 296);
    const glm::vec3 C = glm::vec3(472, 0, 406);
    const glm::vec3 D = glm::vec3(314, 0, 456);
    const glm::vec3 E = glm::vec3(423, 330, 247);
    const glm::vec3 F = glm::vec3(265, 330, 296);
    const glm::vec3 G = glm::vec3(472, 330, 406);
    const glm::vec3 H = glm::vec3(314, 330, 456);

    // Front
    triangles.push_back(Triangle(E, B, A, kBlue));
    triangles.push_back(Triangle(E, F, B, kBlue));

    // Front
    triangles.push_back(Triangle(F, D, B, kBlue));
    triangles.push_back(Triangle(F, H, D, kBlue));

    // BACK
    triangles.push_back(Triangle(H, C, D, kBlue));
    triangles.push_back(Triangle(H, G, C, kBlue));

    // LEFT
    triangles.push_back(Triangle(G, E, C, kBlue));
    triangles.push_back(Triangle(E, A, C, kBlue));

    // TOP
    triangles.push_back(Triangle(G, F, E, kBlue));
    triangles.push_back(Triangle(G, H, F, kBlue));
}

/**
 * @brief Normalize Cornell Box coordinates to renderer unit space.
 * @details Scales from millimeter-style scene units to [-1,1]^3 and mirrors X/Y to match camera convention.
 */
void CornellBox::scaleToUnitCube() {
    // Scale to the volume [-1,1]^3
    for (Triangle &triangle : triangles) {
        triangle.v0 *= 2 / kL;
        triangle.v1 *= 2 / kL;
        triangle.v2 *= 2 / kL;

        triangle.v0 -= glm::vec3(1, 1, 1);
        triangle.v1 -= glm::vec3(1, 1, 1);
        triangle.v2 -= glm::vec3(1, 1, 1);

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