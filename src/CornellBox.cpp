

#include <glm/glm.hpp>
#include <vector>
#include "Triangle.hpp"
#include "CornellBox.hpp"

/**
 * @brief Load the Cornell Box
 * @param triangles The vector to write the mesh triangles into
 */
void CornellBox::Load()
{
    triangles.clear();
    triangles.reserve(5 * 2 * 3);

    Room();
    ShortBlock();
    TallBlock();
    ScaleToUnitCube();
}

void CornellBox::Room()
{

    // Corners of the Cornell Box
    const glm::vec3 A(L, 0, 0);
    const glm::vec3 B(0, 0, 0);
    const glm::vec3 C(L, 0, L);
    const glm::vec3 D(0, 0, L);
    const glm::vec3 E(L, L, 0);
    const glm::vec3 F(0, L, 0);
    const glm::vec3 G(L, L, L);
    const glm::vec3 H(0, L, L);

    // Floor:
    triangles.push_back(Triangle(C, B, A, GREEN));
    triangles.push_back(Triangle(C, D, B, GREEN));

    // Left wall
    triangles.push_back(Triangle(A, E, C, PURPLE));
    triangles.push_back(Triangle(C, E, G, PURPLE));

    // Right wall
    triangles.push_back(Triangle(F, B, D, YELLOW));
    triangles.push_back(Triangle(H, F, D, YELLOW));

    // Ceiling
    triangles.push_back(Triangle(E, F, G, CYAN));
    triangles.push_back(Triangle(F, H, G, CYAN));

    // Back wall
    triangles.push_back(Triangle(G, D, C, WHITE));
    triangles.push_back(Triangle(G, H, D, WHITE));
}

void CornellBox::ShortBlock()
{

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
    triangles.push_back(Triangle(E, B, A, RED));
    triangles.push_back(Triangle(E, F, B, RED));

    // Front
    triangles.push_back(Triangle(F, D, B, RED));
    triangles.push_back(Triangle(F, H, D, RED));

    // BACK
    triangles.push_back(Triangle(H, C, D, RED));
    triangles.push_back(Triangle(H, G, C, RED));

    // LEFT
    triangles.push_back(Triangle(G, E, C, RED));
    triangles.push_back(Triangle(E, A, C, RED));

    // TOP
    triangles.push_back(Triangle(G, F, E, RED));
    triangles.push_back(Triangle(G, H, F, RED));
}

void CornellBox::TallBlock()
{
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
    triangles.push_back(Triangle(E, B, A, BLUE));
    triangles.push_back(Triangle(E, F, B, BLUE));

    // Front
    triangles.push_back(Triangle(F, D, B, BLUE));
    triangles.push_back(Triangle(F, H, D, BLUE));

    // BACK
    triangles.push_back(Triangle(H, C, D, BLUE));
    triangles.push_back(Triangle(H, G, C, BLUE));

    // LEFT
    triangles.push_back(Triangle(G, E, C, BLUE));
    triangles.push_back(Triangle(E, A, C, BLUE));

    // TOP
    triangles.push_back(Triangle(G, F, E, BLUE));
    triangles.push_back(Triangle(G, H, F, BLUE));
}

void CornellBox::ScaleToUnitCube()
{
    // Scale to the volume [-1,1]^3
    for (Triangle &triangle : triangles)
    {
        triangle.v0 *= 2 / L;
        triangle.v1 *= 2 / L;
        triangle.v2 *= 2 / L;

        triangle.v0 -= glm::vec3(1, 1, 1);
        triangle.v1 -= glm::vec3(1, 1, 1);
        triangle.v2 -= glm::vec3(1, 1, 1);

        triangle.v0.x *= -1;
        triangle.v1.x *= -1;
        triangle.v2.x *= -1;

        triangle.v0.y *= -1;
        triangle.v1.y *= -1;
        triangle.v2.y *= -1;

        triangle.ComputeNormal();
    }
}