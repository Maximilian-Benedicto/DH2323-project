#ifndef IRAYTRACER_HPP
#define IRAYTRACER_HPP

#include <vector>

// Forward declarations to keep the interface header lean
class Triangle;
class Light;
class Camera;
class Window;

/**
 * @brief Abstract interface for a Ray Tracing engine.
 * Ensures any implementation (Whitted, Path Tracing, etc.)
 * adheres to the same input/output contract.
 */
class IRayTracer
{
public:
    virtual ~IRayTracer() = default;

    /**
     * @brief Renders the scene defined by the parameters into the provided Window.
     * @param triangles Const reference to the geometry buffer.
     * @param light     The light source configuration.
     * @param camera    The camera configuration.
     * @param outWindow Reference to the window where pixels are drawn.
     */
    virtual void render(
        const std::vector<Triangle> &triangles,
        const Light &light,
        const Camera &camera,
        Window &outWindow) = 0;
};

#endif