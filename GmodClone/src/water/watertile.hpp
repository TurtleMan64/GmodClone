#ifndef WATERTILE_H
#define WATERTILE_H

class WaterTile
{
public:
    float centerX;
    float centerZ;

    static constexpr float TILE_SIZE = 30.0f;

    WaterTile(float centerX, float centerZ);
};
#endif
