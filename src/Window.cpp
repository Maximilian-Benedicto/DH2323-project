
#include <SDL3/SDL.h>
#include <iostream>
#include <glm/glm.hpp>
#include "Window.hpp"

using namespace std;

Window::Window(int width, int height, int scale, bool isFullscreen) {
    this->width = width;
    this->height = height;
    this->scale = scale;
    this->isFullscreen = isFullscreen;

    if (!initializeSDL() || !createWindow() || !createRenderer() || !createTexture() || !createPixelBuffer()) {
        cout << "Could not initialize Window. Exiting." << endl;
        exit(1);
    }

    atexit(SDL_Quit);
}

Window::~Window() {
    if (pixelBuffer != nullptr) {
        free(pixelBuffer);
    }
}

bool Window::initializeSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        cout << "Could not initialize SDL: " << SDL_GetError() << endl;
        return false;
    }

    return true;
}

bool Window::createWindow() {
    SDL_WindowFlags flags = (SDL_WindowFlags)(SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE);

    sdlWindow = SDL_CreateWindow("SDL", width * scale, height * scale, flags);
    if (sdlWindow == nullptr) {
        cout << "Could not create SDL window: " << SDL_GetError() << endl;
        return false;
    }

    if (isFullscreen) {
        SDL_SetWindowFullscreen(sdlWindow, true);
    }

    return true;
}

bool Window::createRenderer() {
    sdlRenderer = SDL_CreateRenderer(sdlWindow, nullptr);
    if (sdlRenderer == nullptr) {
        cout << "Could not create SDL renderer: " << SDL_GetError() << endl;
        return false;
    }

    SDL_SetRenderVSync(sdlRenderer, 1);

    SDL_SetRenderLogicalPresentation(sdlRenderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    return true;
}

bool Window::createTexture() {
    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (sdlTexture == nullptr) {
        cout << "Could not create SDL texture: " << SDL_GetError() << endl;
        return false;
    }

    SDL_SetTextureScaleMode(sdlTexture, SDL_SCALEMODE_LINEAR);

    return true;
}

bool Window::createPixelBuffer() {
    pixelBuffer = (Uint32 *)calloc(width * height, sizeof(Uint32));
    if (pixelBuffer == nullptr) {
        cout << "Could not create SDL pixel buffer." << endl;
        return false;
    }

    return true;
}

void Window::clearPixels() {
    memset(pixelBuffer, 0, width * height * sizeof(Uint32));
}

void Window::putPixel(int x, int y, glm::vec3 color) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    Uint8 red = Uint8(glm::clamp(255 * color.r, 0.f, 255.f));
    Uint8 green = Uint8(glm::clamp(255 * color.g, 0.f, 255.f));
    Uint8 blue = Uint8(glm::clamp(255 * color.b, 0.f, 255.f));
    Uint8 alpha = 255;

    Uint32 rgba = (alpha << 24) + (red << 16) + (green << 8) + blue;

    Uint32 *pixel = (Uint32 *)pixelBuffer + y * width + x;

    *pixel = rgba;
}

void Window::render() {
    SDL_UpdateTexture(sdlTexture, nullptr, pixelBuffer, width * sizeof(Uint32));

    SDL_RenderClear(sdlRenderer);
    SDL_RenderTexture(sdlRenderer, sdlTexture, nullptr, nullptr);
    SDL_RenderPresent(sdlRenderer);
}

bool Window::saveBMP(const char *filename) {
    SDL_Surface *surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_ARGB8888, pixelBuffer, width * 4);
    if (surface == nullptr) {
        cout << "Could not create SDL surface for bitmap: " << SDL_GetError() << endl;
        return false;
    }

    SDL_SaveBMP(surface, filename);
    SDL_DestroySurface(surface);
    return true;
}

bool Window::quitEvent() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return true;
        }

        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                return true;
            }
        }
    }

    return false;
}

void Window::setWindowTitle(const char *title) {
    SDL_SetWindowTitle(sdlWindow, title);
}

void Window::setRenderResolution(int newWidth, int newHeight) {
    this->width = newWidth;
    this->height = newHeight;

    if (sdlTexture != nullptr) {
        SDL_DestroyTexture(sdlTexture);
        sdlTexture = nullptr;
    }
    if (pixelBuffer != nullptr) {
        free(pixelBuffer);
        pixelBuffer = nullptr;
    }

    SDL_SetRenderLogicalPresentation(sdlRenderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    createTexture();
    createPixelBuffer();
}

void Window::getRenderResolution(int &outWidth, int &outHeight) {
    outWidth = width;
    outHeight = height;
}