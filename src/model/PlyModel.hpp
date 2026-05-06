#ifndef PLY_MODEL_HPP
#define PLY_MODEL_HPP

#include "Model.hpp"
#include "glm/glm.hpp"

/// @brief Model class for loading and representing 3D models from PLY files.
class PlyModel : public Model {
   private:
    /// @brief Filepath to the PLY file to load.
    std::string filename;

    /// @brief Center the model at the origin and resize by provided scale factor
    void scaleAndCenter();

   public:
    /// @brief Construct a PlyModel with a given filepath.
    /// @param scale Scale factor to apply to the model after loading.
    /// @param filename Filepath to the PLY file to load.
    PlyModel(glm::vec3 scale, std::string filename)
        : Model(scale), filename(filename) {}

    /// @brief Load the PLY model by parsing the PLY file, populating triangles, and building the BVH.
    void load() override;
};

#endif