#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <glm/glm.hpp>
#include <string>

struct Texture {
    unsigned char *data = nullptr;

    int width = 0;
    int height = 0;

    int channels = 0;

    bool usesStbAllocator = true;

    Texture(const std::string &filepath);
    ~Texture();

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;
    Texture(Texture &&other) noexcept;
    Texture &operator=(Texture &&other) noexcept;

    glm::vec3 sample(const glm::vec2 &uv) const;
};

#endif