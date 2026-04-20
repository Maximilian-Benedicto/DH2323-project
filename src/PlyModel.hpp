#ifndef STANFORDBUNNY_HPP
#define STANFORDBUNNY_HPP

#include "Model.hpp"

/**
 * @brief Class that can load a .ply model as a list of triangles.
 */
class PlyModel : public Model
{
private:
    std::string filename;

    // Scale the model to fit in the volume [-1,1]^3
    void ScaleToUnitCube();

public:
    PlyModel(std::string filename) : filename(filename) {}

    void Load() override;
};

#endif