#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <algorithm>
#include <thread>
#include <atomic>

#include "include/Window.hpp"
#include "include/Triangle.hpp"
#include "include/Camera.hpp"
#include "include/Light.hpp"
#include "include/CornellBox.hpp"
#include "include/StanfordBunny.hpp"

#include "include/Shader.hpp"
#include "include/LambertianShader.hpp"
#include "include/DipoleShader.hpp"
#include "include/WireframeShader.hpp"

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
Shader *activeShader;
LambertianShader *lambertian;
DipoleShader *dipole;
WireframeShader *wireframe;

std::thread renderThread;
std::atomic<bool> killFlag(false);

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
void StopRenderThread();
void StartRenderThread();
void ResetCamera();

// ----------------------------------------------------------------------------
// MAIN LOOP

int main(int argc, char *argv[])
{
    window = new Window(screenWidth, screenHeight, 10, false);
    t = SDL_GetTicks(); // Set start value for timer.

    StanfordBunny::Load(triangles, 1);

    lambertian = new LambertianShader();
    dipole = new DipoleShader();
    wireframe = new WireframeShader();
    activeShader = lambertian;

    StartRenderThread();

    while (!window->quitEvent())
    {
        Update();
        Draw();
    }

    StopRenderThread();

    window->saveBMP("screenshot.bmp");

    delete lambertian;
    delete dipole;
    delete wireframe;
    delete window;

    return 0;
}

// ----------------------------------------------------------------------------
// FUNCTION DEFINITIONS

void StopRenderThread()
{
    if (renderThread.joinable())
    {
        killFlag = true;
        renderThread.join();
    }
}

void StartRenderThread()
{
    killFlag = false;
    int w, h;
    window->getRenderResolution(w, h);
    Uint32 *buffer = window->getPixelBuffer();
    renderThread = std::thread([w, h, buffer]()
                               { activeShader->render(buffer, w, h, triangles, light, camera, killFlag); });
}

void Update(void)
{
    int t2 = SDL_GetTicks();
    t = t2;

    const Uint8 *keystate = (const Uint8 *)SDL_GetKeyboardState(NULL);

    bool changed = false;
    bool resChanged = false;

    // Camera movement
    if (keystate[SDL_SCANCODE_W])
    {
        camera.position.z += cameraSpeed;
        changed = true;
    } // FORWARD
    if (keystate[SDL_SCANCODE_S])
    {
        camera.position.z -= cameraSpeed;
        changed = true;
    } // BACKWARD
    if (keystate[SDL_SCANCODE_A])
    {
        camera.position.x -= cameraSpeed;
        changed = true;
    } // LEFT
    if (keystate[SDL_SCANCODE_D])
    {
        camera.position.x += cameraSpeed;
        changed = true;
    } // RIGHT
    if (keystate[SDL_SCANCODE_SPACE])
    {
        camera.position.y += cameraSpeed;
        changed = true;
    } // UP
    if (keystate[SDL_SCANCODE_LSHIFT])
    {
        camera.position.y -= cameraSpeed;
        changed = true;
    } // DOWN

    // Camera rotation
    if (keystate[SDL_SCANCODE_UP])
    {
        camera.pitch += rotationSpeed;
        changed = true;
    } // LOOK UP
    if (keystate[SDL_SCANCODE_DOWN])
    {
        camera.pitch -= rotationSpeed;
        changed = true;
    } // LOOK DOWN
    if (keystate[SDL_SCANCODE_LEFT])
    {
        camera.yaw -= rotationSpeed;
        changed = true;
    } // TURN LEFT
    if (keystate[SDL_SCANCODE_RIGHT])
    {
        camera.yaw += rotationSpeed;
        changed = true;
    } // TURN RIGHT
    if (keystate[SDL_SCANCODE_Q])
    {
        camera.roll -= rotationSpeed;
        changed = true;
    } // ROLL LEFT
    if (keystate[SDL_SCANCODE_E])
    {
        camera.roll += rotationSpeed;
        changed = true;
    } // ROLL RIGHT

    // Reset camera
    if (keystate[SDL_SCANCODE_BACKSPACE])
    {
        ResetCamera();
        changed = true;
    } // RESET CAMERA

    // Light controls
    if (keystate[SDL_SCANCODE_J])
    {
        light.position.x -= lightSpeed;
        changed = true;
    } // LEFT
    if (keystate[SDL_SCANCODE_L])
    {
        light.position.x += lightSpeed;
        changed = true;
    } // RIGHT
    if (keystate[SDL_SCANCODE_U])
    {
        light.position.y += lightSpeed;
        changed = true;
    } // UP
    if (keystate[SDL_SCANCODE_O])
    {
        light.position.y -= lightSpeed;
        changed = true;
    } // DOWN
    if (keystate[SDL_SCANCODE_I])
    {
        light.position.z += lightSpeed;
        changed = true;
    } // FORWARD
    if (keystate[SDL_SCANCODE_K])
    {
        light.position.z -= lightSpeed;
        changed = true;
    } // BACKWARD

    // Shader switching
    static int lastShaderSwitchTime = 0;
    if (t - lastShaderSwitchTime > 200)
    {
        if (keystate[SDL_SCANCODE_1])
        {
            activeShader = lambertian;
            changed = true;
            lastShaderSwitchTime = t;
        }
        if (keystate[SDL_SCANCODE_2])
        {
            activeShader = dipole;
            changed = true;
            lastShaderSwitchTime = t;
        }
        if (keystate[SDL_SCANCODE_3])
        {
            activeShader = wireframe;
            changed = true;
            lastShaderSwitchTime = t;
        }
    }

    // Resolution changes
    static int lastResChangeTime = 0;
    if (t - lastResChangeTime > 200)
    {
        if (keystate[SDL_SCANCODE_EQUALS] || keystate[SDL_SCANCODE_KP_PLUS]) // + key
        {
            StopRenderThread();
            screenWidth = std::min(screenWidth + 100, 2000);
            screenHeight = std::min(screenHeight + 100, 2000);
            camera.focalLength = screenHeight / 2.0f;
            window->setRenderResolution(screenWidth, screenHeight);
            lastResChangeTime = t;
            resChanged = true;
            changed = false; // Priority to resChanged
        }
        else if (keystate[SDL_SCANCODE_MINUS] || keystate[SDL_SCANCODE_KP_MINUS]) // - key
        {
            StopRenderThread();
            screenWidth = std::max(screenWidth - 100, 100);
            screenHeight = std::max(screenHeight - 100, 100);
            camera.focalLength = screenHeight / 2.0f;
            window->setRenderResolution(screenWidth, screenHeight);
            lastResChangeTime = t;
            resChanged = true;
            changed = false; // Priority to resChanged
        }
    }

    if (resChanged)
    {
        StartRenderThread();
    }
    else if (changed)
    {
        StopRenderThread();
        StartRenderThread();
    }
}

void Draw()
{
    window->render();
    // SDL_Delay(16); // Roughly 60 FPS
}

void ResetCamera()
{
    camera.position = vec3(0, 0, -2);
    camera.pitch = 0;
    camera.roll = 0;
    camera.yaw = 0;
}
