#ifndef OBJ_MODEL_HPP
#define OBJ_MODEL_HPP

#include "Model.hpp"

/// @brief Model class for loading and representing 3D models from OBJ files.
class ObjModel : public Model {
   private:
    /// @brief Filepath to the OBJ file to load.
    std::string filename;

    /// @brief Scale the model so that it fits within a unit cube centered at the origin.
    void scaleToUnitCube();

   public:
    /// @brief Construct an ObjModel with a given filepath.
    /// @param filename Filepath to the OBJ file to load.
    ObjModel(const std::string &filename) : filename(filename) {}

    /// @brief Load the OBJ model by parsing the OBJ file, populating triangles and textures, and building the BVH.
    void load() override;
};

#endif