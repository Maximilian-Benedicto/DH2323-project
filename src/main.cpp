#include <iostream>
#include <glm/glm.hpp>
#include "Window.cpp"

using glm::vec3;

int main()
{
    std::cout << "DH2323 Project C++ Application Started" << std::endl;

    vec3 position(1.0f, 2.0f, 3.0f);
    std::cout << "Initial Position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;

    // Create a window with a pixel buffer to draw into.
    Window window(800, 600);

    return 0;
}