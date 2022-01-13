#ifndef QUATERNION_H
#define QUATERNION_H

#include "matrix.hpp"

class Quaternion
{
public:
    float x;
    float y;
    float z;
    float w;

    Quaternion();

    Quaternion(float x, float y, float z, float w);

    void normalize();

    Matrix4f toRotationMatrix();

    static Quaternion fromMatrix(Matrix4f* matrix);

    static Quaternion interpolate(Quaternion* a, Quaternion* b, float blend);
};
#endif
