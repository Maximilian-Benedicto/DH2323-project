
#define GLM_ENABLE_EXPERIMENTAL  // Enable experimental features in GLM, needed for rotate() function

#include <algorithm>
#include <atomic>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <memory>
#include <thread>

#include "Camera.hpp"
#include "CornellBox.hpp"
#include "DipoleShader.hpp"
#include "LambertianShader.hpp"
#include "Light.hpp"
#include "ObjModel.hpp"
#include "PlyModel.hpp"
#include "Shader.hpp"
#include "Triangle.hpp"
#include "Window.hpp"
#include "WireframeShader.hpp"

using namespace std;
using glm::vec3;

int screenWidth = 100;
int screenHeight = 100;
Window* window;
int t;

DipoleShader::Mode DIPOLE_MODE = DipoleShader::FULL;
int NUM_THREADS = 25;
int DIPOLE_MULTIPLE_SCATTER_SAMPLES = 100;
int DIPOLE_SINGLE_SCATTER_SAMPLES = 100;

vector<unique_ptr<Shader>> shaders;
vector<unique_ptr<Model>> models;
size_t activeModelIdx;
size_t activeShaderIdx;

std::thread renderThread;
std::atomic<bool> shouldStopRenderThread(false);

vec3 cameraInitialPosition =
    vec3(0, 0, -1) * 555.0f * 0.1f;  // Scale to perfectly fit the Cornell box in view
Light light(glm::vec3(0, -1, 0) * 250.0f * 0.1f, 1.4e4f * glm::vec3(1, 1, 1));
Camera camera(cameraInitialPosition, glm::vec3(0, 0, 1), screenHeight / 2.0f);

float cameraSpeed = 5.0f;
float rotationSpeed = M_PI / 180.0f * 2.0f;  // Rotate 2 degrees per key press
float lightSpeed = 5.0f;

/// @brief Update the scene based on user input and update the camera and light positions accordingly.
void update();

/// @brief Render the current scene to the window.
void draw();

/// @brief Stop the render thread by setting the shouldStopRenderThread flag and joining the thread.
void stopRenderThread();

/// @brief Start the render thread, running until shouldStopRenderThread is set to true.
void startRenderThread();

/// @brief Reset the camera to its initial position and orientation.
void resetCamera();

int main(int argc, char* argv[]) {
    window = new Window(screenWidth, screenHeight, 10, false);
    t = SDL_GetTicks();

    // Initialize the models
    models.push_back(make_unique<CornellBox>(vec3(-1.0f, -1.0f, 1.0f) * 0.1f));
    models.push_back(
        make_unique<PlyModel>(vec3(1.0f, -1.0f, -1.0f) * 0.5e4f, "model/bun_zipper.ply"));
    models.push_back(make_unique<ObjModel>(vec3(1.0f, -1.0f, 1.0f), "model/sponza/sponza.obj"));
    models.push_back(
        make_unique<ObjModel>(vec3(1.0f, -1.0f, -1.0f) * 200.0f, "model/diana/diana.obj"));

    // Initialize the shaders
    shaders.push_back(make_unique<WireframeShader>(NUM_THREADS));
    shaders.push_back(make_unique<DipoleShader>(
        DIPOLE_MODE, NUM_THREADS, DIPOLE_MULTIPLE_SCATTER_SAMPLES, DIPOLE_SINGLE_SCATTER_SAMPLES));
    shaders.push_back(make_unique<LambertianShader>(NUM_THREADS, vec3(0.5f, 0.5f, 0.5f)));

    // Load the models, removing any that fail to load
    for (size_t i = 0; i < models.size(); i++)
        try {
            models[i]->load();
        } catch (const std::exception& e) {
            std::cerr << e.what() << '\n';
            models.erase(models.begin() + i);
            i--;
        }

    // Set the active model and shader indices
    activeModelIdx = 0;
    activeShaderIdx = 0;

    startRenderThread();

    while (!window->quitEvent()) {
        update();
        draw();
    }

    stopRenderThread();

    const string filename = "screenshot_" + to_string(time(nullptr)) + ".bmp";
    window->saveBMP(filename.c_str());

    delete window;

    return 0;
}

void stopRenderThread() {
    if (renderThread.joinable()) {
        shouldStopRenderThread = true;
        renderThread.join();
    }
}

void startRenderThread() {
    shouldStopRenderThread = false;
    int w, h;
    window->getRenderResolution(w, h);
    Uint32* buffer = window->getPixelBuffer();
    renderThread = std::thread([w, h, buffer]() {
        shaders[activeShaderIdx]->render(buffer, w, h, *models[activeModelIdx], light, camera,
                                         shouldStopRenderThread);
    });
}

void update() {
    int t2 = SDL_GetTicks();
    t = t2;

    const Uint8* keystate = (const Uint8*)SDL_GetKeyboardState(NULL);

    bool hasSceneChanged = false;
    bool hasResolutionChanged = false;

    vec3 right = normalize(cross(vec3(0.0f, 1.0f, 0.0f), camera.direction));
    vec3 up = vec3(0.0f, 1.0f, 0.0f);

    if (keystate[SDL_SCANCODE_W]) {
        camera.position += camera.direction * cameraSpeed;
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_S]) {
        camera.position -= camera.direction * cameraSpeed;
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_A]) {
        camera.position -= right * cameraSpeed;
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_D]) {
        camera.position += right * cameraSpeed;
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_SPACE]) {
        camera.position -= up * cameraSpeed;
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_LSHIFT]) {
        camera.position += up * cameraSpeed;
        hasSceneChanged = true;
    }

    if (keystate[SDL_SCANCODE_UP]) {
        camera.direction = glm::rotate(camera.direction, rotationSpeed, right);
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_DOWN]) {
        camera.direction = glm::rotate(camera.direction, -rotationSpeed, right);
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_LEFT]) {
        camera.direction = glm::rotate(camera.direction, -rotationSpeed, up);
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_RIGHT]) {
        camera.direction = glm::rotate(camera.direction, rotationSpeed, up);
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_Q]) {
        camera.roll -= rotationSpeed;
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_E]) {
        camera.roll += rotationSpeed;
        hasSceneChanged = true;
    }

    if (keystate[SDL_SCANCODE_BACKSPACE]) {
        resetCamera();
        hasSceneChanged = true;
    }

    if (keystate[SDL_SCANCODE_J]) {
        light.position.x -= lightSpeed;
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_L]) {
        light.position.x += lightSpeed;
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_U]) {
        light.position.y += lightSpeed;
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_O]) {
        light.position.y -= lightSpeed;
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_I]) {
        light.position.z += lightSpeed;
        hasSceneChanged = true;
    }
    if (keystate[SDL_SCANCODE_K]) {
        light.position.z -= lightSpeed;
        hasSceneChanged = true;
    }

    if (keystate[SDL_SCANCODE_LALT]) {
        const string filename = "screenshot_" + to_string(time(nullptr)) + ".bmp";
        window->saveBMP(filename.c_str());
    }

    static int lastShaderSwitchTime = 0;
    if ((t - lastShaderSwitchTime > 200) && keystate[SDL_SCANCODE_1]) {
        if (WireframeShader* wfShader =
                dynamic_cast<WireframeShader*>(shaders[activeShaderIdx].get())) {
            if (wfShader->isShowingBvh) {
                ++activeShaderIdx;
                activeShaderIdx %= shaders.size();
            }

            wfShader->isShowingBvh = !wfShader->isShowingBvh;

            hasSceneChanged = true;
            lastShaderSwitchTime = t;
        } else if (DipoleShader* dShader =
                       dynamic_cast<DipoleShader*>(shaders[activeShaderIdx].get())) {
            // Rotate modes, or switch shader if at the end of the modes
            if (dShader->mode == DipoleShader::FULL)
                dShader->mode = DipoleShader::SINGLE_SCATTER;
            else if (dShader->mode == DipoleShader::SINGLE_SCATTER)
                dShader->mode = DipoleShader::MULTIPLE_SCATTER;
            else if (dShader->mode == DipoleShader::MULTIPLE_SCATTER) {
                dShader->mode = DipoleShader::FULL;
                ++activeShaderIdx;
                activeShaderIdx %= shaders.size();
            }

            hasSceneChanged = true;
            lastShaderSwitchTime = t;
        } else {
            ++activeShaderIdx;
            activeShaderIdx %= shaders.size();
            hasSceneChanged = true;
            lastShaderSwitchTime = t;
        }
    }

    static int lastModelSwitchTime = 0;
    if ((t - lastModelSwitchTime > 200) && keystate[SDL_SCANCODE_2]) {
        ++activeModelIdx;
        activeModelIdx %= models.size();
        hasSceneChanged = true;
        lastModelSwitchTime = t;
    }

    static int lastResChangeTime = 0;
    if (t - lastResChangeTime > 200) {
        if (keystate[SDL_SCANCODE_EQUALS] || keystate[SDL_SCANCODE_KP_PLUS]) {
            stopRenderThread();
            screenWidth = std::min(screenWidth + 100, 2000);
            screenHeight = std::min(screenHeight + 100, 2000);
            camera.focalLength = screenHeight / 2.0f;
            window->setRenderResolution(screenWidth, screenHeight);
            lastResChangeTime = t;
            hasResolutionChanged = true;
            hasSceneChanged = false;
        } else if (keystate[SDL_SCANCODE_MINUS] || keystate[SDL_SCANCODE_KP_MINUS]) {
            stopRenderThread();
            screenWidth = std::max(screenWidth - 100, 100);
            screenHeight = std::max(screenHeight - 100, 100);
            camera.focalLength = screenHeight / 2.0f;
            window->setRenderResolution(screenWidth, screenHeight);
            lastResChangeTime = t;
            hasResolutionChanged = true;
            hasSceneChanged = false;
        }
    }

    if (hasResolutionChanged) {
        startRenderThread();
    } else if (hasSceneChanged) {
        stopRenderThread();
        startRenderThread();
    }
}

void draw() {
    window->render();
}

void resetCamera() {
    camera.position = cameraInitialPosition;
    camera.direction = vec3(0, 0, 1);
    camera.roll = 0;
}
