#ifndef STANFORDBUNNY_HPP
#define STANFORDBUNNY_HPP

#include "Model.hpp"

/**
 * @brief class that can load the Stanford Bunny as a list of triangles.
 */
class StanfordBunny : public Model
{
private:
    // Scale the bunny to fit in the volume [-1,1]^3

    void ScaleToUnitCube();

public:
    enum Resolution
    {
        LOW,
        MEDIUM,
        HIGH,
        FULL
    };
    Resolution resolution;
    void Load() override;
};

#endif