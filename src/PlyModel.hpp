#ifndef PLY_MODEL_HPP
#define PLY_MODEL_HPP

#include "Model.hpp"

class PlyModel : public Model {
   private:
    std::string filename;

    void scaleToUnitCube();

   public:
    PlyModel(std::string filename) : filename(filename) {}

    void load() override;
};

#endif