#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <glm/glm.hpp>
#include "Material.hpp"

// Number of threads for parallel processing. Adjust based on your system's
// capabilities (will crash if set too high)
#define NUM_THREADS 16

// Screen resolution
#define SCREEN_WIDTH 100
#define SCREEN_HEIGHT 100

// Dipole subsurface scattering samples
#define DIPOLE_MULTIPLE_SCATTER_SAMPLES 100
#define DIPOLE_SINGLE_SCATTER_SAMPLES 100

// Anisotropy for subsurface scattering simulation (0.85 per Jensen et al. paper)
#define ANISOTROPY_FACTOR 0.85f

// Maximum number of triangles per leaf node in the BVH tree
#define BVH_MAX_TRIANGLES_PER_LEAF 2

// Default material for objects that don't specify one
#define DEFAULT_MATERIAL Material::createMarble()

// Camera configuration
#define CAMERA_INITIAL_POSITION glm::vec3(0, 0, -1) * 555.0f * 0.1f
#define CAMERA_INITIAL_DIRECTION glm::vec3(0, 0, 1)
#define CAMERA_INITIAL_ROLL 0
#define CAMERA_MOVE_SPEED 5.0f
#define CAMERA_ROTATE_SPEED (float)M_PI / 180.0f * 2.0f

// Light configuration
#define LIGHT_INITIAL_POSITION glm::vec3(0, -1, 0) * 250.0f * 0.1f
#define LIGHT_INITIAL_COLOR 1.4e5f * glm::vec3(1, 1, 1)
#define LIGHT_MOVE_SPEED 5.0f

#endif  // CONFIG_HPP