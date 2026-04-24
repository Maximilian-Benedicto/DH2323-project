#ifndef CORNELL_BOX_HPP
#define CORNELL_BOX_HPP

#include <glm/glm.hpp>

#include "Model.hpp"

/// @brief Simple hardcoded Cornell Box scene with two blocks and a light source on the ceiling.
/// Provided by DH2323 course staff in the labs.
class CornellBox : public Model {
   private:
    const glm::vec3 kRed = glm::vec3(0.75f, 0.15f, 0.15f);
    const glm::vec3 kYellow = glm::vec3(0.75f, 0.75f, 0.15f);
    const glm::vec3 kGreen = glm::vec3(0.15f, 0.75f, 0.15f);
    const glm::vec3 kCyan = glm::vec3(0.15f, 0.75f, 0.75f);
    const glm::vec3 kBlue = glm::vec3(0.15f, 0.15f, 0.75f);
    const glm::vec3 kPurple = glm::vec3(0.75f, 0.15f, 0.75f);
    const glm::vec3 kWhite = glm::vec3(0.75f, 0.75f, 0.75f);

    /// @brief Length of the Cornell Box in all dimensions (the box is a cube).
    const float kL = 555;

    /// @brief Add the walls, floor, and ceiling of the Cornell Box to the scene as triangles.
    void room();

    /// @brief Add the short block in the Cornell Box to the scene as triangles.
    void shortBlock();

    /// @brief Add the tall block in the Cornell Box to the scene as triangles.
    void tallBlock();

    /// @brief Scale the Cornell Box so that it fits within a unit cube centered at the origin.
    void scaleToUnitCube();

   public:
    /// @brief Load the Cornell Box scene by populating triangles and building the BVH.
    void load() override;
};

#endif