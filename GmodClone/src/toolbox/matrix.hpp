#ifndef MATRIX_H
#define MATRIX_H

class Vector3f;

#include <math.h>
#include "vector.hpp"

class Matrix4f
{
public:
    float m00;
    float m01;
    float m02;
    float m03;
    float m10;
    float m11;
    float m12;
    float m13;
    float m20;
    float m21;
    float m22;
    float m23;
    float m30;
    float m31;
    float m32;
    float m33;

    Matrix4f();

    Matrix4f(Matrix4f*);

    Matrix4f(const Matrix4f &other);

    void setZero();

    void setIdentity();

    /**
    * Copy the buffered matrix into this matrix
    * @param buf The buffered matrix data
    */
    void load(float[]);

    // loads the data sorted by column first.
    // example:
    // data = 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
    //
    // mat4 =
    //        1 5  9 13
    //        2 6 10 14
    //        3 7 11 15
    //        4 8 12 16
    void loadColumnFirst(float* data);

    //loads from a string of 16 floats
    // example: 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1
    void load(char* line);

    /**
    *
    * @param buf Buffer to put this matrix data into
    */
    void store(float[]);

    /**
    * Multiply the right matrix by this matrix and place the result in a dest matrix.
    * @param right The right source matrix
    * @param dest The destination matrix
    */
    void multiply(Matrix4f* right, Matrix4f* dest);

    void translate(Vector3f*);

    void translate(Vector2f*);

    void scale(Vector3f*);

    void scale(Vector2f*);

    void set(Matrix4f* other);

    //angle in radians
    void rotate(float, Vector3f*);

    Vector4f transform(Vector4f* vec);

    void invert();

    float determinant();

    float determinant3x3(
        float t00, float t01, float t02,
        float t10, float t11, float t12,
        float t20, float t21, float t22);

    //bool operator <(const Matrix4f& rhs) const;
};
#endif
