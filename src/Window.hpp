#ifndef WINDOW_HPP
#define WINDOW_HPP

#define SDL_MAIN_HANDLED

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

/// @brief Window class for creating an SDL window, managing a pixel buffer, and handling rendering and events.
/// Provided by DH2323 course staff in the labs.
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
    ~Window();

    /// @brief Construct a Window with the given width, height, scale factor, and fullscreen mode.
    /// @param width
    /// @param height
    /// @param scale
    /// @param isFullscreen
    Window(int width, int height, int scale = 1, bool isFullscreen = false);

    /// @brief Clear the pixel buffer by setting all pixels to black.
    void clearPixels();

    /// @brief Set the color of a pixel in the pixel buffer.
    /// @param x
    /// @param y
    /// @param color
    void putPixel(int x, int y, glm::vec3 color);

    /// @brief Update the SDL texture with the current contents of the pixel buffer and present it on the window.
    void render();

    /// @brief Save the current contents of the pixel buffer to a BMP image file with the given filename.
    /// @param filename
    /// @return true if the image was saved successfully, false otherwise.
    bool saveBMP(const char *filename);

    /// @brief Poll SDL events and return true if a quit event is received (e.g. window close or escape key press).
    /// @return true if a quit event is received, false otherwise.
    bool quitEvent();

    /// @brief Set the title of the SDL window.
    /// @param title New title for the window.
    void setWindowTitle(const char *title);

    /// @brief Set the resolution of the pixel buffer used for rendering.
    /// @param newWidth
    /// @param newHeight
    void setRenderResolution(int newWidth, int newHeight);

    /// @brief Get the current resolution of the pixel buffer used for rendering.
    /// @param outWidth
    /// @param outHeight
    void getRenderResolution(int &outWidth, int &outHeight);

    /// @brief Get a pointer to the pixel buffer used for rendering, which is a flat array of 32-bit RGBA values.
    /// @return Pointer to the pixel buffer.
    Uint32 *getPixelBuffer() {
        return pixelBuffer;
    }

   private:
    /// @brief Initialize SDL.
    /// @return true if initialization is successful, false otherwise.
    bool initializeSDL();

    /// @brief Create the pixel buffer for rendering.
    /// @return true if the pixel buffer was created successfully, false otherwise.
    bool createPixelBuffer();

    /// @brief Create the SDL window.
    /// @return true if the window was created successfully, false otherwise.
    bool createRenderer();

    /// @brief Create the SDL texture for rendering.
    /// @return true if the texture was created successfully, false otherwise.
    bool createTexture();

    /// @brief Create the SDL window.
    /// @return true if the window was created successfully, false otherwise.
    bool createWindow();
};
#endif
