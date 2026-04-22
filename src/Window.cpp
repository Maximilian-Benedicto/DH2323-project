// from https://github.com/lemonad/DH2323-Skeleton (heavily modified)
#include <SDL3/SDL.h>
#include <iostream>
#include <glm/glm.hpp>
#include "Window.hpp"

using namespace std;

/*
 * Construct a window with an associated pixel buffer to
 * draw into.
 *
 * If the fullscreen flag is set, the window is maximized
 * to the desktop size (but resolution is not changed).
 * In this case, width and height is only used for the
 * size of the pixel buffer.
 */
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

/*
 * Free allocated pixel buffer. We don't have to destroy SDL
 * objects here because that is taken care of by SDL_Quit.
 */
Window::~Window() {
    if (pixelBuffer != nullptr) {
        free(pixelBuffer);
    }
}

/*
 * Sets up SDL (video and timer) for per pixel drawing.
 *
 * Returns true on success.
 */
bool Window::initializeSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        cout << "Could not initialize SDL: " << SDL_GetError() << endl;
        return false;
    }

    return true;
}

/*
 * Creates a window for rendering to.
 *
 * Returns true on success.
 */
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

/*
 * Creates a renderer which is scaled up so we don't get
 * a stamp-sized output on screen.
 *
 * Returns true on success.
 */
bool Window::createRenderer() {
    // Just pick any renderer available (providing NULL name gives
    // priority to available hardware accelerated renderers.)
    sdlRenderer = SDL_CreateRenderer(sdlWindow, nullptr);
    if (sdlRenderer == nullptr) {
        cout << "Could not create SDL renderer: " << SDL_GetError() << endl;
        return false;
    }

    SDL_SetRenderVSync(sdlRenderer, 1);

    // Make the scaled rendering look smoother.
    SDL_SetRenderLogicalPresentation(sdlRenderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    return true;
}

/*
 * Create the texture which represents the whole screen. The pixel
 * buffer is used to update this texture which is then rendered to
 * screen.
 *
 * Returns true on success.
 */
bool Window::createTexture() {
    // This represents a texture on the GPU.
    // SDL_TEXTUREACCESS_STREAMING tells SDL that this texture's
    // contents is going to change frequently.
    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (sdlTexture == nullptr) {
        cout << "Could not create SDL texture: " << SDL_GetError() << endl;
        return false;
    }

    SDL_SetTextureScaleMode(sdlTexture, SDL_SCALEMODE_LINEAR);

    return true;
}

/*
 * Create the pixel buffer for per-pixel drawing into.
 *
 * Returns true on success.
 */
bool Window::createPixelBuffer() {
    pixelBuffer = (Uint32 *)calloc(width * height, sizeof(Uint32));
    if (pixelBuffer == nullptr) {
        cout << "Could not create SDL pixel buffer." << endl;
        return false;
    }

    return true;
}

/*
 * Clears the pixel buffer (i.e. sets it to black).
 */
void Window::clearPixels() {
    memset(pixelBuffer, 0, width * height * sizeof(Uint32));
}

/*
 * Update a pixel in the pixel buffer. The color is represented
 * by a glm:vec3 which specifies the red, green and blue components
 * with numbers between 0.0 and 1.0 (inclusive).
 */
void Window::putPixel(int x, int y, glm::vec3 color) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    Uint8 red = Uint8(glm::clamp(255 * color.r, 0.f, 255.f));
    Uint8 green = Uint8(glm::clamp(255 * color.g, 0.f, 255.f));
    Uint8 blue = Uint8(glm::clamp(255 * color.b, 0.f, 255.f));
    Uint8 alpha = 255;

    // Each pixel in the pixel buffer consists of four bytes: 0xAARRGGBB.
    Uint32 rgba = (alpha << 24) + (red << 16) + (green << 8) + blue;

    // Calculate the address of the pixel we want to set.
    Uint32 *pixel = (Uint32 *)pixelBuffer + y * width + x;

    // TODO big endian support?
    // #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    // #else
    // #endif

    *pixel = rgba;
}

/*
 * Use the pixel buffer to update the texture, then render the
 * texture into the window/screen.
 */
void Window::render() {
    SDL_UpdateTexture(sdlTexture, nullptr, pixelBuffer, width * sizeof(Uint32));

    SDL_RenderClear(sdlRenderer);
    SDL_RenderTexture(sdlRenderer, sdlTexture, nullptr, nullptr);
    SDL_RenderPresent(sdlRenderer);
}

/*
 * Save pixel buffer as a bitmap file.
 *
 * Returns true on success.
 */
bool Window::saveBMP(const char *filename) {
    // TODO big endian support?
    SDL_Surface *surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_ARGB8888, pixelBuffer, width * 4);
    if (surface == nullptr) {
        cout << "Could not create SDL surface for bitmap: " << SDL_GetError() << endl;
        return false;
    }

    SDL_SaveBMP(surface, filename);
    SDL_DestroySurface(surface);
    return true;
}

/*
 * Goes through the SDL event queue looking for events corresponding
 * to the user wanting to quit/exit.
 *
 * Returns true if a quit event was received.
 */
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

/*
 * Updates the window title.
 */
void Window::setWindowTitle(const char *title) {
    SDL_SetWindowTitle(sdlWindow, title);
}

/*
 * Updates the logical render resolution
 */
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

/*
 * Gets the current logical render resolution
 */
void Window::getRenderResolution(int &outWidth, int &outHeight) {
    outWidth = width;
    outHeight = height;
}