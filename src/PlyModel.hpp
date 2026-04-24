#ifndef PLY_MODEL_HPP
#define PLY_MODEL_HPP

#include "Model.hpp"

/// @brief Model class for loading and representing 3D models from PLY files.
class PlyModel : public Model {
   private:
    /// @brief Filepath to the PLY file to load.
    std::string filename;

    /// @brief Scale the model so that it fits within a unit cube centered at the origin.
    void scaleToUnitCube();

   public:
    /// @brief Construct a PlyModel with a given filepath.
    /// @param filename Filepath to the PLY file to load.
    PlyModel(std::string filename) : filename(filename) {}

    /// @brief Load the PLY model by parsing the PLY file, populating triangles, and building the BVH.
    void load() override;
};

#endif