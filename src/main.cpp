#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL // For rotate_vector
#include <glm/gtx/rotate_vector.hpp>
#include <algorithm>
#include <thread>
#include <atomic>

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
#include "BVH.hpp"

using namespace std;
using glm::vec3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

// Window variables
int screenWidth = 100;
int screenHeight = 100;
Window *window;
int t;

// Scene mesh triangles
vector<Triangle> triangles;

// Shaders
Shader *activeShader;
LambertianShader *lambertian;
DipoleShader *dipole;
WireframeShader *wireframe;

// Models
Model *activeModel;
CornellBox *cornellBox;
PlyModel *plyModel;
ObjModel *objModel;

// BVHs for each model
BVH *cornellBVH;
BVH *plyBVH;
BVH *objBVH;
BVH *activeBVH;

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
    cornellBox = new CornellBox();
    plyModel = new PlyModel("../model/bun_zipper.ply");
    objModel = new ObjModel("../model/sponza/sponza.obj");
    cornellBox->Load();
    plyModel->Load();
    objModel->Load();

    // Build BVHs for each model
    cornellBVH = new BVH(cornellBox->triangles);
    plyBVH = new BVH(plyModel->triangles);
    objBVH = new BVH(objModel->triangles);

    // Initialize shaders
    lambertian = new LambertianShader();
    dipole = new DipoleShader();
    wireframe = new WireframeShader();

    // Set the active model, BVH, and shader
    activeBVH = cornellBVH;
    activeModel = cornellBox;
    activeShader = wireframe;

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

    delete cornellBVH;
    delete plyBVH;
    delete objBVH;

    delete cornellBox;
    delete plyModel;
    delete objModel;

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
    triangles = activeModel->triangles;
    killFlag = false;
    int w, h;
    window->getRenderResolution(w, h);
    Uint32 *buffer = window->getPixelBuffer();
    renderThread = std::thread([w, h, buffer]()
                               { activeShader->render(buffer, w, h, *activeBVH, triangles, light, camera, killFlag); });
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
            if (activeShader == wireframe)
                wireframe->showBVH = !wireframe->showBVH;

            activeShader = wireframe;
            changed = true;
            lastShaderSwitchTime = t;
        }
    }

    // Model switching
    static int lastModelSwitchTime = 0;
    if (t - lastModelSwitchTime > 200)
    {
        if (keystate[SDL_SCANCODE_4])
        {
            activeModel = cornellBox;
            activeBVH = cornellBVH;
            changed = true;
            lastModelSwitchTime = t;
        }
        if (keystate[SDL_SCANCODE_5])
        {
            activeModel = plyModel;
            activeBVH = plyBVH;
            changed = true;
            lastModelSwitchTime = t;
        }
        if (keystate[SDL_SCANCODE_6])
        {
            activeModel = objModel;
            activeBVH = objBVH;
            changed = true;
            lastModelSwitchTime = t;
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
    while (t - SDL_GetTicks() < 16)
        ; // Cap framerate to ~60 fps
}

void ResetCamera()
{
    camera.position = vec3(0, 0, -2);
    camera.direction = vec3(0, 0, 1);
    camera.roll = 0;
}
