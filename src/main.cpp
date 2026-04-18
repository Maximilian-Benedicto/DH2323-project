#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <algorithm>
#include "include/Window.hpp"
#include "include/Triangle.hpp"
#include "include/Camera.hpp"
#include "include/Light.hpp"
#include "include/CornellBox.hpp"
#include "include/StanfordBunny.hpp"
#include "Lambertian.cpp"
#include "Dipole.cpp"

using namespace std;
using glm::vec3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

int screenWidth = 100;
int screenHeight = 100;
Window *window;
int t;

// Render variables
vector<Triangle> triangles;
Lambertian lambertian;
Dipole dipole;
Light light(vec3(0, -0.5, -0.7), 14.f * vec3(1, 1, 1)); // Omni-light
Camera camera(vec3(0, 0, -2), screenHeight / 2);

// Movement variables
float cameraSpeed = 0.1;
float rotationSpeed = M_PI / 24;
float lightSpeed = 0.05;

// ----------------------------------------------------------------------------
// FUNCTION DECLARATIONS

void Update(void);
void Draw(void);

// ----------------------------------------------------------------------------
// MAIN LOOP

int main(int argc, char *argv[])
{
    window = new Window(screenWidth, screenHeight, 10, false);
    t = SDL_GetTicks(); // Set start value for timer.

    StanfordBunny::Load(triangles, 1);

    while (!window->quitEvent())
    {
        Update();
        Draw();
    }
    window->saveBMP("screenshot.bmp");
    return 0;
}

// ----------------------------------------------------------------------------
// FUNCTION DEFINITIONS

void Update(void)
{

    // Compute frame time:
    int t2 = SDL_GetTicks();
    float dt = float(t2 - t);
    t = t2;
    cout << "Frame: " << screenHeight << "x" << screenWidth << " Time: " << dt << " ms" << endl;

    // Get keystrokes
    const Uint8 *keystate = (const Uint8 *)SDL_GetKeyboardState(NULL);

    // Camera movement
    if (keystate[SDL_SCANCODE_W])
        camera.position.z += cameraSpeed; // FORWARD
    if (keystate[SDL_SCANCODE_S])
        camera.position.z -= cameraSpeed; // BACKWARD
    if (keystate[SDL_SCANCODE_A])
        camera.position.x -= cameraSpeed; // LEFT
    if (keystate[SDL_SCANCODE_D])
        camera.position.x += cameraSpeed; // RIGHT
    if (keystate[SDL_SCANCODE_SPACE])
        camera.position.y += cameraSpeed; // UP
    if (keystate[SDL_SCANCODE_LSHIFT])
        camera.position.y -= cameraSpeed; // DOWN

    // Camera rotation
    if (keystate[SDL_SCANCODE_UP])
        camera.pitch += rotationSpeed; // LOOK UP
    if (keystate[SDL_SCANCODE_DOWN])
        camera.pitch -= rotationSpeed; // LOOK DOWN
    if (keystate[SDL_SCANCODE_LEFT])
        camera.yaw -= rotationSpeed; // TURN LEFT
    if (keystate[SDL_SCANCODE_RIGHT])
        camera.yaw += rotationSpeed; // TURN RIGHT
    if (keystate[SDL_SCANCODE_Q])
        camera.roll -= rotationSpeed; // ROLL LEFT
    if (keystate[SDL_SCANCODE_E])
        camera.roll += rotationSpeed; // ROLL RIGHT

    // Light controls
    if (keystate[SDL_SCANCODE_J])
        light.position.x -= lightSpeed; // LEFT
    if (keystate[SDL_SCANCODE_L])
        light.position.x += lightSpeed; // RIGHT
    if (keystate[SDL_SCANCODE_U])
        light.position.y += lightSpeed; // UP
    if (keystate[SDL_SCANCODE_O])
        light.position.y -= lightSpeed; // DOWN
    if (keystate[SDL_SCANCODE_I])
        light.position.z += lightSpeed; // FORWARD
    if (keystate[SDL_SCANCODE_K])
        light.position.z -= lightSpeed; // BACKWARD

    // Resolution controls (changes size of the pixel buffer with + and - keys)
    static int lastResChangeTime = 0;
    if (t - lastResChangeTime > 200)
    {
        if (keystate[SDL_SCANCODE_EQUALS] || keystate[SDL_SCANCODE_KP_PLUS]) // + key
        {
            screenWidth = std::min(screenWidth + 100, 2000);
            screenHeight = std::min(screenHeight + 100, 2000);
            camera.focalLength = screenHeight / 2.0f;
            window->setRenderResolution(screenWidth, screenHeight);
            lastResChangeTime = t;
        }
        else if (keystate[SDL_SCANCODE_MINUS] || keystate[SDL_SCANCODE_KP_MINUS]) // - key
        {
            screenWidth = std::max(screenWidth - 100, 100);
            screenHeight = std::max(screenHeight - 100, 100);
            camera.focalLength = screenHeight / 2.0f;
            window->setRenderResolution(screenWidth, screenHeight);
            lastResChangeTime = t;
        }
    }
}

void Draw()
{
    window->clearPixels();

    lambertian.render(triangles, light, camera, *window);

    window->render();
}
