#include "Model.hpp"

void Model::scaleAndCenter() {

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