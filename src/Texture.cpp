
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Texture.hpp"

Texture::Texture(const std::string &filepath)
{

    // Force 3 channels (RGB)
    data = stbi_load(filepath.c_str(), &width, &height, &channels, 3);
    channels = 3;
    usesStbAllocator = true;

    if (!data)
    {
        // If no texture, magenta/black checkerboard pattern will be used to indicate missing texture
        width = 2;
        height = 2;
        channels = 3;
        usesStbAllocator = false;
        data = new unsigned char[width * height * channels]{
            255, 0, 255, 0, 0, 0,
            0, 0, 0, 255, 0, 255};
    }
}

Texture::Texture(Texture &&other) noexcept
    : data(other.data), width(other.width), height(other.height), channels(other.channels), usesStbAllocator(other.usesStbAllocator)
{
    other.data = nullptr;
    other.width = 0;
    other.height = 0;
    other.channels = 0;
    other.usesStbAllocator = true;
}

Texture &Texture::operator=(Texture &&other) noexcept
{
    if (this == &other)
        return *this;

    if (data)
    {
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

Texture::~Texture()
{
    if (data)
    {
        if (usesStbAllocator)
            stbi_image_free(data);
        else
            delete[] data;
    }
}

glm::vec3 Texture::Sample(const glm::vec2 &uv) const
{
    // Wrap UV coordinates
    float u = uv.x - floor(uv.x);
    float v = uv.y - floor(uv.y);

    // Convert to pixel coordinates
    int x = static_cast<int>(u * width);
    int y = static_cast<int>(v * height);

    // Get pixel color
    int index = (y * width + x) * channels;
    unsigned char r = data[index + 0];
    unsigned char g = data[index + 1];
    unsigned char b = data[index + 2];

    return glm::vec3(r, g, b) / 255.0f;
}
