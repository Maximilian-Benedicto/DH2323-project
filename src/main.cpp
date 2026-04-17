#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include "Window.h"
#include "Model.h"
#include <algorithm>

using namespace std;
using glm::mat3;
using glm::vec3;

// ----------------------------------------------------------------------------
// STRUCTS

struct Intersection
{
    vec3 position;
    float distance;
    int triangleIndex;
};

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 100;
const int SCREEN_HEIGHT = 100;
const vec3 BLACK(0.0f, 0.0f, 0.0f);
Window *window;
int t;

// Model variables
vector<Triangle> triangles;

// Camera variables
float focalLength = SCREEN_HEIGHT / 2;
vec3 cameraPos(0, 0, -2);
float cameraSpeed = 0.1;

// Rotation variables
float rotationSpeed = M_PI / 24;
float yaw;
mat3 R;

// Omni-light
vec3 lightPos(0, -0.5, -0.7);
vec3 lightColor = 14.f * vec3(1, 1, 1);
float lightSpeed = 0.05;
vec3 indirectLight = 0.5f * vec3(1, 1, 1);

// ----------------------------------------------------------------------------
// FUNCTION DECLARATIONS

void Update(void);
void Draw(void);
bool ClosestIntersection(vec3 start, vec3 dir, const vector<Triangle> &triangles, Intersection &closestIntersection);
vec3 DirectLight(const Intersection &i);

// ----------------------------------------------------------------------------
// MAIN LOOP

int main(int argc, char *argv[])
{
    window = new Window(SCREEN_WIDTH, SCREEN_HEIGHT, 10, false);
    t = SDL_GetTicks(); // Set start value for timer.

    Load(triangles);

    while (!window->quitEvent())
    {
        Update();
        Draw();
    }
    window->saveBMP("screenshot.bmp");
    return 0;
}

// ----------------------------------------------------------------------------
// FUNCTION DEFINITIONS

void Update(void)
{
    // Update rotation matrix
    R = mat3{
        (float)cos(yaw), 0.0f, (float)sin(yaw), // Column 0 (top to bottom)
        0.0f, 1.0f, 0.0f,                       // Column 1
        (float)-sin(yaw), 0.0f, (float)cos(yaw) // Column 2
    };

    // Compute frame time:
    int t2 = SDL_GetTicks();
    float dt = float(t2 - t);
    t = t2;
    cout << "Rendertime: " << dt << " ms." << endl;

    // Get keystrokes
    const Uint8 *keystate = (const Uint8 *)SDL_GetKeyboardState(NULL);

    // Camera controls
    if (keystate[SDL_SCANCODE_UP])
        cameraPos.z += cameraSpeed; // CAMERA MOVE FORWARD
    if (keystate[SDL_SCANCODE_DOWN])
        cameraPos.z -= cameraSpeed; // CAMERA MOVE BACK
    if (keystate[SDL_SCANCODE_SPACE])
        cameraPos.y -= cameraSpeed; // CAMERA MOVE UP
    if (keystate[SDL_SCANCODE_LCTRL])
        cameraPos.y += cameraSpeed; // CAMERA MOVE DOWN
    if (keystate[SDL_SCANCODE_LEFT])
        yaw += rotationSpeed; // CAMERA ROTATE LEFT
    if (keystate[SDL_SCANCODE_RIGHT])
        yaw -= rotationSpeed; // CAMERA ROTATE RIGHT

    // Light controls
    if (keystate[SDL_SCANCODE_W])
        lightPos.z += lightSpeed; // LIGHT MOVE FORWARD
    if (keystate[SDL_SCANCODE_S])
        lightPos.z -= lightSpeed; // LIGHT MOVE BACK
    if (keystate[SDL_SCANCODE_A])
        lightPos.x -= lightSpeed; // LIGHT MOVE LEFT
    if (keystate[SDL_SCANCODE_D])
        lightPos.x += lightSpeed; // LIGHT MOVE RIGHT
    if (keystate[SDL_SCANCODE_Q])
        lightPos.y += lightSpeed; // LIGHT MOVE UP
    if (keystate[SDL_SCANCODE_E])
        lightPos.y -= lightSpeed; // LIGHT MOVE DOWN
}

void Draw()
{
    window->clearPixels();

    for (int y = 0; y < SCREEN_HEIGHT; ++y)
    {
        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {
            // Calculate start and direction for ray
            vec3 start = cameraPos;
            vec3 dir((float)x - SCREEN_WIDTH / 2.0, (float)y - SCREEN_HEIGHT / 2.0, focalLength);

            // Apply rotation matrix
            start = start * R;
            dir = dir * R;

            // Find closest ray intersection
            Intersection closestIntersection;
            bool found = ClosestIntersection(start, dir, triangles, closestIntersection);

            // Draw pixel
            vec3 color;
            if (found)
                // color = triangles[closestIntersection.triangleIndex].color * DirectLight(closestIntersection);
                color = triangles[closestIntersection.triangleIndex].color * (DirectLight(closestIntersection) + indirectLight);
            else
                color = BLACK;
            window->putPixel(x, y, color);
        }
    }
    window->render();
}

bool ClosestIntersection(vec3 start, vec3 dir, const vector<Triangle> &triangles, Intersection &closestIntersection)
{
    bool found = false;

    for (size_t i = 0; i < triangles.size(); i++)
    {
        // Get triangle
        Triangle triangle = triangles[i];

        // Compute intersection with ray
        using glm::mat3;
        using glm::vec3;
        vec3 v0 = triangle.v0;
        vec3 v1 = triangle.v1;
        vec3 v2 = triangle.v2;
        vec3 e1 = v1 - v0;
        vec3 e2 = v2 - v0;
        vec3 b = start - v0;
        mat3 A(-dir, e1, e2);
        vec3 x = glm::inverse(A) * b;

        // Extract t, u, v values
        float t = x.x;
        float u = x.y;
        float v = x.z;

        // Check if t, u, v are within the triangle
        bool tCheck = 0 <= t;
        bool uCheck = 0 <= u;
        bool vCheck = 0 <= v;
        bool uvCheck = u + v <= 1;

        // Continue to next triangle if the ray does not intersect
        if (!(tCheck && uCheck && vCheck && uvCheck))
            continue;

        // Compute intersection position and distance
        vec3 position = start + dir * t;
        float distance = glm::length(dir * t);

        // Save the intersection if it is the first one, or closer
        if (!found || distance < closestIntersection.distance)
        {
            closestIntersection = {position, distance, (int)i};
            found = true;
        }
    }

    return found;
}

vec3 DirectLight(const Intersection &i)
{
    // Intersection to light vector
    vec3 r = lightPos - i.position;

    // Intersection normal vector
    vec3 nUnit = triangles[i.triangleIndex].normal;

    // Offset intersection by a small amount (epsilon) to prevent self-intersection
    const float epsilon = 1e-4f;
    vec3 start = i.position + nUnit * epsilon;

    // Find inverse intersection
    Intersection reverse;
    if (ClosestIntersection(start, r, triangles, reverse))
        // No light (shadow) if intersection is closer than light source
        if (glm::length(start - reverse.position) < glm::length(r))
            return vec3(0, 0, 0);

    // Power per area
    vec3 B = lightColor / (float)(4 * M_PI * pow(glm::length(r), 2));

    // Intersection to light unit vector
    vec3 rUnit = glm::normalize(r);

    // Power per real surface
    vec3 D = B * glm::max(glm::dot(rUnit, nUnit), 0.0f);

    return D;
}