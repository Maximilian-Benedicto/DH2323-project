#ifndef PLY_MODEL_HPP
#define PLY_MODEL_HPP

#include "Model.hpp"

/// @brief Model class for loading and representing 3D models from PLY files.
class PlyModel : public Model {
   private:
    /// @brief Filepath to the PLY file to load.
    std::string filename;

    /// @brief Center the model at the origin and resize by provided scale factor
    void scaleAndCenter();

   public:
    /// @brief Construct a PlyModel with a given filepath.
    /// @param filename Filepath to the PLY file to load.
    PlyModel(std::string filename) : filename(filename) {}

    /// @brief Load the PLY model by parsing the PLY file, populating triangles, and building the BVH.
    void load() override;
};

#endif