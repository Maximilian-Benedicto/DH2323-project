#ifndef OBJ_MODEL_HPP
#define OBJ_MODEL_HPP

#include "Model.hpp"

/// @brief Model class for loading and representing 3D models from OBJ files.
class ObjModel : public Model {
   private:
    /// @brief Filepath to the OBJ file to load.
    std::string filename;

   public:
    /// @brief Construct an ObjModel with a given filepath.
    /// @param scale Scale factor to apply to the model after loading.
    /// @param filename Filepath to the OBJ file to load.
    ObjModel(glm::vec3 scale, const std::string& filename) : Model(scale), filename(filename) {}

    /// @brief Load the OBJ model by parsing the OBJ file, populating triangles and textures, and building the BVH.
    void load() override;
};

#endif