#ifndef WATERTILE_H
#define WATERTILE_H

#include "../toolbox/vector.hpp"

class WaterTile
{
public:
    Vector3f pos;
    Vector3f scale;
    float rotY;
    Vector3f color;
    float murkiness;

    WaterTile(Vector3f pos, Vector3f scale, float rotY, Vector3f color, float murkiness);
};
#endif
