#define private public
#include "LambertianShader.hpp"
#undef private

#include <cmath>
#include <iostream>

namespace {

bool approxEqual(float a, float b, float eps = 1e-4f) {
    return std::fabs(a - b) <= eps;
}

int runCase(
    const char* name,
    LambertianShader& shader,
    const AABB& box,
    const glm::vec3& start,
    const glm::vec3& dir,
    bool expectedHit,
    float expectedTClose,
    bool checkTClose)
{
    float tClose = 0.0f;
    bool hit = shader.SlabIntersection(box, start, dir, tClose);

    bool ok = (hit == expectedHit);
    if (ok && hit && checkTClose) {
        ok = approxEqual(tClose, expectedTClose);
    }

    std::cout << name << ": "
              << (ok ? "PASS" : "FAIL")
              << " (hit=" << hit << ", tClose=" << tClose << ")\n";

    return ok ? 0 : 1;
}

} // namespace

int main()
{
    LambertianShader shader;
    AABB box;
    box.min = glm::vec3(0.0f, 0.0f, 0.0f);
    box.max = glm::vec3(1.0f, 1.0f, 1.0f);

    int failures = 0;

    failures += runCase(
        "outside-hit",
        shader,
        box,
        glm::vec3(-1.0f, 0.5f, 0.5f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        true,
        1.0f,
        true);

    failures += runCase(
        "inside-hit-negative-tclose",
        shader,
        box,
        glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        true,
        -0.5f,
        true);

    failures += runCase(
        "outside-miss",
        shader,
        box,
        glm::vec3(-1.0f, 2.0f, 0.5f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        false,
        0.0f,
        false);

    failures += runCase(
        "parallel-outside-miss",
        shader,
        box,
        glm::vec3(2.0f, 0.5f, 0.5f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        false,
        0.0f,
        false);

    if (failures == 0) {
        std::cout << "\nAll SlabIntersection tests passed.\n";
        return 0;
    }

    std::cout << "\n" << failures << " SlabIntersection test(s) failed.\n";
    return 1;
}
