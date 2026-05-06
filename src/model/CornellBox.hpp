#ifndef CORNELL_BOX_HPP
#define CORNELL_BOX_HPP

#include <glm/glm.hpp>

#include "Model.hpp"

/// @brief Simple Cornell Box scene with two blocks.
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

    /// @brief Add the walls, floor, and ceiling of the Cornell Box to the scene as triangles.
    void room();

    /// @brief Add the short block in the Cornell Box to the scene as triangles.
    void shortBlock();

    /// @brief Add the tall block in the Cornell Box to the scene as triangles.
    void tallBlock();

   public:
    /// @brief Load the Cornell Box scene by populating triangles and building the BVH.
    void load() override;

    /// @brief Construct a CornellBox model with a given scale factor.
    /// @param scale Scale factor to apply to the model after loading.
    CornellBox(glm::vec3 scale) : Model(scale) {}
};

#endif