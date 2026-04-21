#ifndef OBJMODEL_HPP
#define OBJMODEL_HPP

#include "Model.hpp"

/**
 * @brief Class that can load a .obj file as a list of triangles.
 */
class ObjModel : public Model
{
private:
    std::string filename;
    void ScaleToUnitCube();

public:
    ObjModel(const std::string &filename) : filename(filename) {}

    void Load() override;
};

#endif