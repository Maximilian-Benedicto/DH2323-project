
#define GLM_ENABLE_EXPERIMENTAL  // Enable experimental features in GLM, needed for rotate() function

#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
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

int screenWidth = 100;
int screenHeight = 100;
Window *window;
int t;

vector<unique_ptr<Shader>> shaders;
vector<unique_ptr<Model>> models;
size_t activeModelIdx;
size_t activeShaderIdx;

std::thread renderThread;
std::atomic<bool> shouldStopRenderThread(false);

Light light(glm::vec3(0, -0.5, -0.7), 14.f * glm::vec3(1, 1, 1));
Camera camera(glm::vec3(0, 0, -2), glm::vec3(0, 0, 1), screenHeight / 2.0f);

float cameraSpeed = 0.05;
float rotationSpeed = M_PI / 48;
float lightSpeed = 0.05;

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

int main(int argc, char *argv[]) {
    window = new Window(screenWidth, screenHeight, 10, false);
    t = SDL_GetTicks();

    models = vector<unique_ptr<Model>>();
    models.push_back(make_unique<CornellBox>());
    models.push_back(make_unique<PlyModel>("model/bun_zipper.ply"));
    models.push_back(make_unique<ObjModel>("model/sponza/sponza.obj"));
    for (size_t i = 0; i < models.size(); i++) try {
            models[i]->load();
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
            models.erase(models.begin() + i);
            i--;
        }

    shaders = vector<unique_ptr<Shader>>();
    shaders.push_back(make_unique<WireframeShader>());
    shaders.push_back(make_unique<DipoleShader>());
    shaders.push_back(make_unique<LambertianShader>());

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
    Uint32 *buffer = window->getPixelBuffer();
    renderThread = std::thread([w, h, buffer]() {
        shaders[activeShaderIdx]->render(buffer, w, h, *models[activeModelIdx], light, camera, shouldStopRenderThread);
    });
}

void update() {
    int t2 = SDL_GetTicks();
    t = t2;

    const Uint8 *keystate = (const Uint8 *)SDL_GetKeyboardState(NULL);

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

    static int lastShaderSwitchTime = 0;
    if ((t - lastShaderSwitchTime > 200) && keystate[SDL_SCANCODE_1]) {
        if (WireframeShader *wfShader = dynamic_cast<WireframeShader *>(shaders[activeShaderIdx].get())) {
            if (wfShader->isShowingBvh)
                activeShaderIdx = (++activeShaderIdx) % shaders.size();

            wfShader->isShowingBvh = !wfShader->isShowingBvh;

            hasSceneChanged = true;
            lastShaderSwitchTime = t;
        } else {
            activeShaderIdx = (++activeShaderIdx) % shaders.size();
            hasSceneChanged = true;
            lastShaderSwitchTime = t;
        }
    }

    static int lastModelSwitchTime = 0;
    if ((t - lastModelSwitchTime > 200) && keystate[SDL_SCANCODE_2]) {
        activeModelIdx = (++activeModelIdx) % models.size();
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
    camera.position = vec3(0, 0, -2);
    camera.direction = vec3(0, 0, 1);
    camera.roll = 0;
}
