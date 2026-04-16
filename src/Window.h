#ifndef WINDOW_H
#define WINDOW_H

// from https://github.com/lemonad/DH2323-Skeleton

#define SDL_MAIN_HANDLED // circumvent failure of SDL_Init() when not using SDL_main() as an entry point.

#include <SDL.h>
#include <glm/glm.hpp>

class Window
{
private:
    int width;
    int height;
    bool fullscreen;

    SDL_Renderer *sdl_renderer = NULL;
    SDL_Texture *sdl_texture = NULL;
    SDL_Window *sdl_window = NULL;

    Uint32 *pixel_buffer = NULL;

public:
    ~Window();
    Window(int width, int height, bool fullscreen = false);
    void clearPixels();
    void putPixel(int x, int y, glm::vec3 color);
    void render();
    bool saveBMP(const char *filename);
    bool quitEvent();
    void setWindowTitle(const char *title);

private:
    bool initializeSDL();
    bool createPixelBuffer();
    bool createRenderer();
    bool createTexture();
    bool createWindow();
};
#endif
