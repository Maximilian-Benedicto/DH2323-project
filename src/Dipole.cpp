
#include "include/IRayTracer.hpp"
#include "include/Camera.hpp"
#include "include/Light.hpp"
#include "include/Triangle.hpp"
#include "include/Window.hpp"

using namespace std;

class Dipole : public IRayTracer
{
public:
    void render(const std::vector<Triangle> &triangles, const Light &light, const Camera &camera, Window &outWindow) override {}
};