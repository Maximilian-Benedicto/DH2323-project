#ifndef CORNELL_BOX_HPP
#define CORNELL_BOX_HPP

#include "Model.hpp"
#include <glm/glm.hpp>

class CornellBox : public Model {
   private:
    const glm::vec3 kRed = glm::vec3(0.75f, 0.15f, 0.15f);
    const glm::vec3 kYellow = glm::vec3(0.75f, 0.75f, 0.15f);
    const glm::vec3 kGreen = glm::vec3(0.15f, 0.75f, 0.15f);
    const glm::vec3 kCyan = glm::vec3(0.15f, 0.75f, 0.75f);
    const glm::vec3 kBlue = glm::vec3(0.15f, 0.15f, 0.75f);
    const glm::vec3 kPurple = glm::vec3(0.75f, 0.15f, 0.75f);
    const glm::vec3 kWhite = glm::vec3(0.75f, 0.75f, 0.75f);

    const float kL = 555;

    void room();
    void shortBlock();
    void tallBlock();

    void scaleToUnitCube();

   public:
    void load() override;
};

#endif