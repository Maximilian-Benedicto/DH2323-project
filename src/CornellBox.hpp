#ifndef CORNELL_BOX_HPP
#define CORNELL_BOX_HPP

#include "Model.hpp"
#include <glm/glm.hpp>

/**
 * @brief The Cornell Box model, which consists of a simple box-shaped room with two blocks in it.
 */
class CornellBox : public Model
{
private:
    // Colors
    const glm::vec3 kRed = glm::vec3(0.75f, 0.15f, 0.15f);
    const glm::vec3 kYellow = glm::vec3(0.75f, 0.75f, 0.15f);
    const glm::vec3 kGreen = glm::vec3(0.15f, 0.75f, 0.15f);
    const glm::vec3 kCyan = glm::vec3(0.15f, 0.75f, 0.75f);
    const glm::vec3 kBlue = glm::vec3(0.15f, 0.15f, 0.75f);
    const glm::vec3 kPurple = glm::vec3(0.75f, 0.15f, 0.75f);
    const glm::vec3 kWhite = glm::vec3(0.75f, 0.75f, 0.75f);

    // Length of Cornell Box side.
    const float kL = 555;

    // Shapes
    void room();
    void shortBlock();
    void tallBlock();

    // Scale the Cornell Box to fit in the volume [-1,1]^3
    void scaleToUnitCube();

public:
    void load() override;
};

#endif