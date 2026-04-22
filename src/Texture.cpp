
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Texture.hpp"

/**
 * @brief Load image data into a move-only texture object.
 * @details Missing files use a 2x2 magenta/black checkerboard fallback allocated with new[].
 */
Texture::Texture(const std::string &filepath) {
    // Force 3 channels (RGB)
    data = stbi_load(filepath.c_str(), &width, &height, &channels, 3);
    channels = 3;
    usesStbAllocator = true;

    if (!data) {
        // If no texture, magenta/black checkerboard pattern will be used to indicate missing texture
        width = 2;
        height = 2;
        channels = 3;
        usesStbAllocator = false;
        data = new unsigned char[width * height * channels]{255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255};
    }
}

/**
 * @brief Transfer ownership of texture storage from another texture.
 */
Texture::Texture(Texture &&other) noexcept
    : data(other.data),
      width(other.width),
      height(other.height),
      channels(other.channels),
      usesStbAllocator(other.usesStbAllocator) {
    other.data = nullptr;
    other.width = 0;
    other.height = 0;
    other.channels = 0;
    other.usesStbAllocator = true;
}

/**
 * @brief Move-assign texture storage with correct allocator-aware cleanup.
 */
Texture &Texture::operator=(Texture &&other) noexcept {
    if (this == &other)
        return *this;

    if (data) {
        if (usesStbAllocator)
            stbi_image_free(data);
        else
            delete[] data;
    }

    data = other.data;
    width = other.width;
    height = other.height;
    channels = other.channels;
    usesStbAllocator = other.usesStbAllocator;

    other.data = nullptr;
    other.width = 0;
    other.height = 0;
    other.channels = 0;
    other.usesStbAllocator = true;
    return *this;
}

/**
 * @brief Destroy texture storage using the allocator that created it.
 */
Texture::~Texture() {
    if (data) {
        if (usesStbAllocator)
            stbi_image_free(data);
        else
            delete[] data;
    }
}

/**
 * @brief Sample RGB texture data using UV coordinates.
 * @details UVs are clamped to [0,1] and V is flipped to map OBJ bottom-left UV space to image top-left memory layout.
 */
glm::vec3 Texture::sample(const glm::vec2 &uv) const {
    if (width <= 0 || height <= 0 || channels <= 0 || data == nullptr)
        return glm::vec3(1.0f, 0.0f, 1.0f);

    // Clamp UV coordinates at texture borders.
    float u = glm::clamp(uv.x, 0.0f, 1.0f);
    float v = glm::clamp(uv.y, 0.0f, 1.0f);

    // OBJ UVs are typically bottom-left origin while image memory is top-left.
    v = 1.0f - v;

    // Convert to pixel coordinates
    int x = static_cast<int>(u * static_cast<float>(width - 1));
    int y = static_cast<int>(v * static_cast<float>(height - 1));

    // Get pixel color
    int index = (y * width + x) * channels;
    unsigned char r = data[index + 0];
    unsigned char g = data[index + 1];
    unsigned char b = data[index + 2];

    return glm::vec3(r, g, b) / 255.0f;
}
