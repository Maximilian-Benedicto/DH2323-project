
#include "IRayTracer.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "Triangle.hpp"
#include "Window.hpp"

using namespace std;

class Dipole : public IRayTracer
{
public:
    void render(const std::vector<Triangle> &triangles, const Light &light, const Camera &camera, Window &outWindow) override {}
};