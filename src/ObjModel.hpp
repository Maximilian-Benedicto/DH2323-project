#ifndef OBJ_MODEL_HPP
#define OBJ_MODEL_HPP

#include "Model.hpp"

/**
 * @brief Class that can load a .obj file as a list of triangles.
 */
class ObjModel : public Model
{
private:
    std::string filename;
    void scaleToUnitCube();

public:
    ObjModel(const std::string &filename) : filename(filename) {}

    void load() override;
};

#endif