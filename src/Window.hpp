#ifndef WINDOW_HPP
#define WINDOW_HPP

// from https://github.com/lemonad/DH2323-Skeleton (heavily modified)

#define SDL_MAIN_HANDLED  // circumvent failure of SDL_Init() when not using SDL_main() as an entry point.

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

/**
 * @brief Simple wrapper around SDL that provides a pixel buffer and rendering functionality for the ray tracer.
 */
class Window {
   private:
    int width;
    int height;
    int scale;
    bool isFullscreen;

    SDL_Renderer *sdlRenderer = nullptr;
    SDL_Texture *sdlTexture = nullptr;
    SDL_Window *sdlWindow = nullptr;

    Uint32 *pixelBuffer = nullptr;

   public:
    /** @brief Release the pixel buffer and SDL-owned resources. */
    ~Window();

    /**
     * @brief Create a window and backing pixel buffer.
     * @param width Logical render width in pixels.
     * @param height Logical render height in pixels.
     * @param scale Integer presentation scale factor.
     * @param isFullscreen Whether to start the window in fullscreen mode.
     */
    Window(int width, int height, int scale = 1, bool isFullscreen = false);

    /** @brief Clear the pixel buffer to black. */
    void clearPixels();

    /** @brief Set a single pixel in the logical buffer. */
    void putPixel(int x, int y, glm::vec3 color);

    /** @brief Present the current pixel buffer to the window. */
    void render();

    /** @brief Save the current pixel buffer to a BMP image. */
    bool saveBMP(const char *filename);

    /** @brief Poll SDL events and report whether the user requested quit. */
    bool quitEvent();

    /** @brief Set the native window title text. */
    void setWindowTitle(const char *title);

    /** @brief Recreate render targets for a new logical resolution. */
    void setRenderResolution(int newWidth, int newHeight);

    /** @brief Get the current logical render resolution. */
    void getRenderResolution(int &outWidth, int &outHeight);

    /** @brief Access the raw ARGB8888 pixel buffer. */
    Uint32 *getPixelBuffer() {
        return pixelBuffer;
    }

   private:
    bool initializeSDL();
    bool createPixelBuffer();
    bool createRenderer();
    bool createTexture();
    bool createWindow();
};
#endif
