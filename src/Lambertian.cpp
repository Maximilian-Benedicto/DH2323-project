
#include "include/IRayTracer.hpp"
#include "include/Camera.hpp"
#include "include/Light.hpp"
#include "include/Triangle.hpp"
#include "include/Window.hpp"

using namespace glm;
using namespace std;

class Lambertian : public IRayTracer
{
private:
    struct Intersection
    {
        vec3 position;
        float distance;
        int triangleIndex;
    };

    vec3 indirectLight = vec3(0.5f, 0.5f, 0.5f); // Simple ambient light fallback

    bool ClosestIntersection(vec3 start, vec3 dir, const vector<Triangle> &triangles, Intersection &closestIntersection)
    {
        bool found = false;

        for (size_t i = 0; i < triangles.size(); i++)
        {
            // Get triangle
            Triangle triangle = triangles[i];

            // Compute intersection with ray
            vec3 v0 = triangle.v0;
            vec3 v1 = triangle.v1;
            vec3 v2 = triangle.v2;
            vec3 e1 = v1 - v0;
            vec3 e2 = v2 - v0;
            vec3 b = start - v0;
            mat3 A(-dir, e1, e2);
            vec3 x = inverse(A) * b;

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
            float distance = length(dir * t);

            // Save the intersection if it is the first one, or closer
            if (!found || distance < closestIntersection.distance)
            {
                closestIntersection = {position, distance, (int)i};
                found = true;
            }
        }

        return found;
    }

    vec3 DirectLight(const Intersection &i, const vector<Triangle> &triangles, const Light &light)
    {
        // Intersection to light vector
        vec3 r = light.position - i.position;

        // Intersection normal vector
        vec3 nUnit = triangles[i.triangleIndex].normal;

        // Offset intersection by a small amount (epsilon) to prevent self-intersection
        const float epsilon = 1e-4f;
        vec3 start = i.position + nUnit * epsilon;

        // Find inverse intersection cast towards the light
        Intersection reverse;
        if (ClosestIntersection(start, r, triangles, reverse))
        {
            // No light (shadow) if an intersection is closer than the light source
            if (length(start - reverse.position) < length(r))
                return vec3(0, 0, 0);
        }

        // Power per area
        vec3 B = light.color / (float)(4 * M_PI * pow(length(r), 2));

        // Intersection to light unit vector
        vec3 rUnit = normalize(r);

        // Power per real surface - dot product logic of lambertian shading
        vec3 D = B * glm::max(dot(rUnit, nUnit), 0.0f);

        return D;
    }

    mat3 RotationMatrix(const float pitch, const float roll, const float yaw)
    {
        mat3 Rx = mat3{
            1.0f, 0.0f, 0.0f,
            0.0f, (float)cos(pitch), (float)-sin(pitch),
            0.0f, (float)sin(pitch), (float)cos(pitch)};

        mat3 Ry = mat3{
            (float)cos(yaw), 0.0f, (float)sin(yaw),
            0.0f, 1.0f, 0.0f,
            (float)-sin(yaw), 0.0f, (float)cos(yaw)};

        mat3 Rz = mat3{
            (float)cos(roll), (float)-sin(roll), 0.0f,
            (float)sin(roll), (float)cos(roll), 0.0f,
            0.0f, 0.0f, 1.0f};

        return Ry * Rz * Rx;
    }

public:
    void render(const vector<Triangle> &triangles, const Light &light, const Camera &camera, Window &window) override
    {
        int renderWidth, renderHeight;
        window.getRenderResolution(renderWidth, renderHeight);

        mat3 R = RotationMatrix(camera.pitch, camera.roll, camera.yaw);

        // Loop through window pixels
        for (int y = 0; y < renderHeight; ++y)
        {
            for (int x = 0; x < renderWidth; ++x)
            {
                // Calculate start and direction for ray
                vec3 start = camera.position;
                vec3 dir((float)x - renderWidth / 2.0f, (float)y - renderHeight / 2.0f, camera.focalLength);

                // Apply rotation matrix
                dir = dir * R;

                // Find closest ray intersection
                Intersection closestIntersection;
                bool found = ClosestIntersection(start, dir, triangles, closestIntersection);

                // Draw pixel
                vec3 color;
                if (found)
                {
                    vec3 directL = DirectLight(closestIntersection, triangles, light);
                    color = triangles[closestIntersection.triangleIndex].color * (directL + indirectLight);
                }
                else
                {
                    color = vec3(0, 0, 0); // Background color
                }

                window.putPixel(x, y, color);
            }
        }
    }
};