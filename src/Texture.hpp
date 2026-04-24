#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <glm/glm.hpp>
#include <string>

/// @brief Texture class for loading and sampling textures from image files.
struct Texture {
    /// @brief Raw pixel data loaded from the image file. Each pixel is represented as 4 bytes (RGBA).
    unsigned char *data = nullptr;

    /// @brief Width of the texture in pixels.
    int width = 0;
    /// @brief Height of the texture in pixels.
    int height = 0;

    /// @brief Number of color channels in the texture (e.g., 3 for RGB, 4 for RGBA).
    int channels = 0;

    /// @brief Whether the texture data was allocated using stb_image allocator.
    bool usesStbAllocator = true;

    /// @brief Construct a Texture by loading an image from the given filepath.
    /// @param filepath Filepath to the image file to load as a texture.
    Texture(const std::string &filepath);
    ~Texture();

    // Overloaded operators and constructors to prevent copying and allow moving of Texture objects.
    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;
    Texture(Texture &&other) noexcept;
    Texture &operator=(Texture &&other) noexcept;

    /// @brief Sample the texture at the given UV coordinates.
    /// @param uv UV coordinates in the range [0, 1]
    /// @return RGB color sampled from the texture at the given UV coordinates.
    glm::vec3 sample(const glm::vec2 &uv) const;
};

#endif