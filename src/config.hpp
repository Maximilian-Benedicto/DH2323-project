#ifndef CONFIG_HPP
#define CONFIG_HPP

#define NUM_THREADS 16
#define DIPOLE_MULTIPLE_SCATTER_SAMPLES 100
#define DIPOLE_SINGLE_SCATTER_SAMPLES 100
#define SCREEN_WIDTH 100
#define SCREEN_HEIGHT 100

#include <glm/glm.hpp>
#include "Material.hpp"

const Material DEFAULT_MATERIAL = Material::createMarble();

const glm::vec3 CAMERA_INITIAL_POSITION = glm::vec3(0, 0, -1) * 555.0f * 0.1f;
const glm::vec3 CAMERA_INITIAL_DIRECTION = glm::vec3(0, 0, 1);
const float CAMERA_INITIAL_ROLL = 0;
const float CAMERA_MOVE_SPEED = 5.0f;
const float CAMERA_ROTATE_SPEED = M_PI / 180.0f * 2.0f;

const glm::vec3 LIGHT_INITIAL_POSITION = glm::vec3(0, -1, 0) * 250.0f * 0.1f;
const glm::vec3 LIGHT_INITIAL_COLOR = 1.4e4f * glm::vec3(1, 1, 1);
const float LIGHT_MOVE_SPEED = 5.0f;

#endif  // CONFIG_HPP