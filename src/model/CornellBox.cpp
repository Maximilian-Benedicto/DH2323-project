

#include <glm/glm.hpp>
#include <vector>

#include "BVH.hpp"
#include "CornellBox.hpp"
#include "Triangle.hpp"

void CornellBox::load() {
    triangles.clear();
    triangles.reserve(5 * 2 * 3);

    room();
    shortBlock();
    tallBlock();
    scaleAndCenter();
    bvh = BVH(triangles);
}

void CornellBox::room() {
    const glm::vec3 A(555, 0, 0);
    const glm::vec3 B(0, 0, 0);
    const glm::vec3 C(555, 0, 555);
    const glm::vec3 D(0, 0, 555);
    const glm::vec3 E(555, 555, 0);
    const glm::vec3 F(0, 555, 0);
    const glm::vec3 G(555, 555, 555);
    const glm::vec3 H(0, 555, 555);

    triangles.push_back(Triangle(C, B, A, kGreen));
    triangles.push_back(Triangle(C, D, B, kGreen));

    triangles.push_back(Triangle(A, E, C, kPurple));
    triangles.push_back(Triangle(C, E, G, kPurple));

    triangles.push_back(Triangle(F, B, D, kYellow));
    triangles.push_back(Triangle(H, F, D, kYellow));

    triangles.push_back(Triangle(E, F, G, kCyan));
    triangles.push_back(Triangle(F, H, G, kCyan));

    triangles.push_back(Triangle(G, D, C, kWhite));
    triangles.push_back(Triangle(G, H, D, kWhite));
}

void CornellBox::shortBlock() {
    const glm::vec3 A = glm::vec3(290, 0, 114);
    const glm::vec3 B = glm::vec3(130, 0, 65);
    const glm::vec3 C = glm::vec3(240, 0, 272);
    const glm::vec3 D = glm::vec3(82, 0, 225);
    const glm::vec3 E = glm::vec3(290, 165, 114);
    const glm::vec3 F = glm::vec3(130, 165, 65);
    const glm::vec3 G = glm::vec3(240, 165, 272);
    const glm::vec3 H = glm::vec3(82, 165, 225);

    triangles.push_back(Triangle(E, B, A, kRed));
    triangles.push_back(Triangle(E, F, B, kRed));

    triangles.push_back(Triangle(F, D, B, kRed));
    triangles.push_back(Triangle(F, H, D, kRed));

    triangles.push_back(Triangle(H, C, D, kRed));
    triangles.push_back(Triangle(H, G, C, kRed));

    triangles.push_back(Triangle(G, E, C, kRed));
    triangles.push_back(Triangle(E, A, C, kRed));

    triangles.push_back(Triangle(G, F, E, kRed));
    triangles.push_back(Triangle(G, H, F, kRed));
}

void CornellBox::tallBlock() {
    const glm::vec3 A = glm::vec3(423, 0, 247);
    const glm::vec3 B = glm::vec3(265, 0, 296);
    const glm::vec3 C = glm::vec3(472, 0, 406);
    const glm::vec3 D = glm::vec3(314, 0, 456);
    const glm::vec3 E = glm::vec3(423, 330, 247);
    const glm::vec3 F = glm::vec3(265, 330, 296);
    const glm::vec3 G = glm::vec3(472, 330, 406);
    const glm::vec3 H = glm::vec3(314, 330, 456);

    triangles.push_back(Triangle(E, B, A, kBlue));
    triangles.push_back(Triangle(E, F, B, kBlue));

    triangles.push_back(Triangle(F, D, B, kBlue));
    triangles.push_back(Triangle(F, H, D, kBlue));

    triangles.push_back(Triangle(H, C, D, kBlue));
    triangles.push_back(Triangle(H, G, C, kBlue));

    triangles.push_back(Triangle(G, E, C, kBlue));
    triangles.push_back(Triangle(E, A, C, kBlue));

    triangles.push_back(Triangle(G, F, E, kBlue));
    triangles.push_back(Triangle(G, H, F, kBlue));
}

void CornellBox::scaleAndCenter() {

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