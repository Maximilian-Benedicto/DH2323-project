#ifndef CORNELLBOX_HPP
#define CORNELLBOX_HPP

#include "Model.hpp"
#include <glm/glm.hpp>

/**
 * @brief The Cornell Box model, which consists of a simple box-shaped room with two blocks in it.
 */
class CornellBox : public Model
{
private:
    // Colors
    const glm::vec3 RED = glm::vec3(0.75f, 0.15f, 0.15f);
    const glm::vec3 YELLOW = glm::vec3(0.75f, 0.75f, 0.15f);
    const glm::vec3 GREEN = glm::vec3(0.15f, 0.75f, 0.15f);
    const glm::vec3 CYAN = glm::vec3(0.15f, 0.75f, 0.75f);
    const glm::vec3 BLUE = glm::vec3(0.15f, 0.15f, 0.75f);
    const glm::vec3 PURPLE = glm::vec3(0.75f, 0.15f, 0.75f);
    const glm::vec3 WHITE = glm::vec3(0.75f, 0.75f, 0.75f);

    // Length of Cornell Box side.
    const float L = 555;

    // Shapes
    void Room();
    void ShortBlock();
    void TallBlock();

    // Scale the Cornell Box to fit in the volume [-1,1]^3
    void ScaleToUnitCube();

public:
    void Load() override;
};

#endif