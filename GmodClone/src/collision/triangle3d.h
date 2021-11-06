#ifndef TRIANGLE3D_H
#define TRIANGLE3D_H

#include <math.h>
#include "../toolbox/vector.h"


class Triangle3D
{
public:
    float p1X;
    float p1Y;
    float p1Z;

    float p2X;
    float p2Y;
    float p2Z;

    float p3X;
    float p3Y;
    float p3Z;

    Vector3f normal;

    float A;
    float B;
    float C;
    float D;

    char type; //0 = normal, 1 = diggable, 2 = wall, 3 = death

    char sound;

    char particle;

    float maxX;
    float minX;
    float maxY;
    float minY;
    float maxZ;
    float minZ;

    //Loads in all data from 91 bytes
    Triangle3D(char* buf);

    Triangle3D(Vector3f* newP1, Vector3f* newP2, Vector3f* newP3, char type, char sound, char particle);

    void generateValues();
};

#endif
