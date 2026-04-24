#ifndef WINDOW_HPP
#define WINDOW_HPP

#define SDL_MAIN_HANDLED

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

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

    Window(int width, int height, int scale = 1, bool isFullscreen = false);

    void clearPixels();

    void putPixel(int x, int y, glm::vec3 color);

    void render();

    bool saveBMP(const char *filename);

    bool quitEvent();

    void setWindowTitle(const char *title);

    void setRenderResolution(int newWidth, int newHeight);

    void getRenderResolution(int &outWidth, int &outHeight);

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
