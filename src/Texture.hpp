#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <glm/glm.hpp>
#include <string>

struct Texture
{
    // Texture data is stored as a flat array of unsigned chars (RGBA format)
    unsigned char *data = nullptr;

    // Texture dimensions are used navigate the flat data array
    int width = 0;
    int height = 0;

    // Number of color channels
    int channels = 0;

    // True when data was allocated by stb_image and must be released with stbi_image_free.
    bool usesStbAllocator = true;

    /**
     * @brief Load a texture from an image file.
     * @param filepath The path to the image file to load as a texture.
     */
    Texture(const std::string &filepath);
    ~Texture();

    // Delete copy constructor and copy assignment operator to prevent accidental copying of the texture data
    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;
    Texture(Texture &&other) noexcept;
    Texture &operator=(Texture &&other) noexcept;

    /**
     * @brief Sample the texture at given UV coordinates.
     * @param uv The UV coordinates to sample
     */
    glm::vec3 Sample(const glm::vec2 &uv) const;
};

#endif