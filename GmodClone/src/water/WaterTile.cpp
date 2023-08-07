#include "watertile.hpp"

#include "../toolbox/vector.hpp"

WaterTile::WaterTile(Vector3f pos, Vector3f scale, float rotY, Vector3f color, float murkiness)
{
    this->pos = pos;
    this->scale = scale;
    this->rotY = rotY;
    this->color = color;
    this->murkiness = murkiness;
}
