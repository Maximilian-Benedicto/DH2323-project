#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL // For rotate_vector
#include <glm/gtx/rotate_vector.hpp>
#include <algorithm>
#include <thread>
#include <atomic>
#include <memory>

#include "Window.hpp"
#include "Triangle.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "CornellBox.hpp"
#include "PlyModel.hpp"
#include "Shader.hpp"
#include "LambertianShader.hpp"
#include "DipoleShader.hpp"
#include "WireframeShader.hpp"
#include "ObjModel.hpp"

using namespace std;
using glm::vec3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

// Window variables
int screenWidth = 100;
int screenHeight = 100;
Window *window;
int t;

// Scene variables (unique pointers for polymorphism))
vector<unique_ptr<Shader>> shaders;
vector<unique_ptr<Model>> models;
size_t activeModelIdx;
size_t activeShaderIdx;

// Rendering thread variables
std::thread renderThread;
std::atomic<bool> killFlag(false);

// Camera and light variables
Light light(glm::vec3(0, -0.5, -0.7), 14.f * glm::vec3(1, 1, 1));
Camera camera(glm::vec3(0, 0, -2), glm::vec3(0, 0, 1), screenHeight / 2.0f);

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
    t = SDL_GetTicks();

    // Load models
    models = vector<unique_ptr<Model>>();
    models.push_back(make_unique<CornellBox>());
    models.push_back(make_unique<PlyModel>("model/bunasd_zipper.ply"));
    models.push_back(make_unique<ObjModel>("model/sponza/sponza.obj"));
    for (size_t i = 0; i < models.size(); i++)
        try
        {
            models[i]->Load();
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            models.erase(models.begin() + i);
            i--; // Adjust index after erasing
        }

    // Initialize shaders
    shaders = vector<unique_ptr<Shader>>();
    shaders.push_back(make_unique<WireframeShader>());
    shaders.push_back(make_unique<DipoleShader>());
    shaders.push_back(make_unique<LambertianShader>());

    // Set the active model and shader
    activeModelIdx = 0;
    activeShaderIdx = 0;

    StartRenderThread();

    while (!window->quitEvent())
    {
        Update();
        Draw();
    }

    StopRenderThread();

    // Save screenshot
    const string filename = "screenshot_" + to_string(time(nullptr)) + ".bmp";
    window->saveBMP(filename.c_str());

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
                               { shaders[activeShaderIdx]->render(buffer, w, h, *models[activeModelIdx], light, camera, killFlag); });
}

void Update(void)
{
    int t2 = SDL_GetTicks();
    t = t2;

    const Uint8 *keystate = (const Uint8 *)SDL_GetKeyboardState(NULL);

    // Flags to track if we need to restart the render thread after processing input
    bool changed = false;
    bool resChanged = false;

    // Camera movement
    vec3 right = normalize(cross(vec3(0.0f, 1.0f, 0.0f), camera.direction));
    vec3 up = vec3(0.0f, 1.0f, 0.0f); // Default up

    if (keystate[SDL_SCANCODE_W])
    {
        camera.position += camera.direction * cameraSpeed;
        changed = true;
    } // FORWARD
    if (keystate[SDL_SCANCODE_S])
    {
        camera.position -= camera.direction * cameraSpeed;
        changed = true;
    } // BACKWARD
    if (keystate[SDL_SCANCODE_A])
    {
        camera.position -= right * cameraSpeed;
        changed = true;
    } // LEFT
    if (keystate[SDL_SCANCODE_D])
    {
        camera.position += right * cameraSpeed;
        changed = true;
    } // RIGHT
    if (keystate[SDL_SCANCODE_SPACE])
    {
        camera.position -= up * cameraSpeed;
        changed = true;
    } // UP
    if (keystate[SDL_SCANCODE_LSHIFT])
    {
        camera.position += up * cameraSpeed;
        changed = true;
    } // DOWN

    // Camera rotation
    if (keystate[SDL_SCANCODE_UP])
    {
        camera.direction = glm::rotate(camera.direction, rotationSpeed, right);
        changed = true;
    } // LOOK UP
    if (keystate[SDL_SCANCODE_DOWN])
    {
        camera.direction = glm::rotate(camera.direction, -rotationSpeed, right);
        changed = true;
    } // LOOK DOWN
    if (keystate[SDL_SCANCODE_LEFT])
    {
        camera.direction = glm::rotate(camera.direction, -rotationSpeed, up);
        changed = true;
    } // TURN LEFT
    if (keystate[SDL_SCANCODE_RIGHT])
    {
        camera.direction = glm::rotate(camera.direction, rotationSpeed, up);
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
    if ((t - lastShaderSwitchTime > 200) && keystate[SDL_SCANCODE_1])
    {
        // Use dynamic_cast to check if the current shader is WireframeShader
        if (WireframeShader *wfShader = dynamic_cast<WireframeShader *>(shaders[activeShaderIdx].get()))
        {
            if (wfShader->showBVH)
                activeShaderIdx = (++activeShaderIdx) % shaders.size();

            wfShader->showBVH = !wfShader->showBVH;

            changed = true;
            lastShaderSwitchTime = t;
        }
        else
        {
            activeShaderIdx = (++activeShaderIdx) % shaders.size();
            changed = true;
            lastShaderSwitchTime = t;
        }
    }

    // Model switching
    static int lastModelSwitchTime = 0;
    if ((t - lastModelSwitchTime > 200) && keystate[SDL_SCANCODE_2])
    {
        activeModelIdx = (++activeModelIdx) % models.size();
        changed = true;
        lastModelSwitchTime = t;
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
    while (t - SDL_GetTicks() < 16)
        ; // Cap framerate to ~60 fps
}

void ResetCamera()
{
    camera.position = vec3(0, 0, -2);
    camera.direction = vec3(0, 0, 1);
    camera.roll = 0;
}
