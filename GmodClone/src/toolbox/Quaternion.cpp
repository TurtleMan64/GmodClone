#include <cmath>

#include "quaternion.hpp"
#include "matrix.hpp"
#include <stdio.h>

Quaternion::Quaternion()
{
    
}

Quaternion::Quaternion(float x, float y, float z, float w)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
    normalize();
}

void Quaternion::normalize()
{
    float mag = sqrtf(x*x + y*y + z*z + w*w);
    x /= mag;
    y /= mag;
    z /= mag;
    w /= mag;
}

Matrix4f Quaternion::toRotationMatrix()
{
    Matrix4f rotationMatrix;

    const float xy = x * y;
	const float xz = x * z;
	const float xw = x * w;
	const float yz = y * z;
	const float yw = y * w;
	const float zw = z * w;
	const float xSquared = x * x;
	const float ySquared = y * y;
	const float zSquared = z * z;
	rotationMatrix.m00 = 1 - 2 * (ySquared + zSquared);
	rotationMatrix.m01 = 2 * (xy - zw);
	rotationMatrix.m02 = 2 * (xz + yw);
	rotationMatrix.m03 = 0;
	rotationMatrix.m10 = 2 * (xy + zw);
	rotationMatrix.m11 = 1 - 2 * (xSquared + zSquared);
	rotationMatrix.m12 = 2 * (yz - xw);
	rotationMatrix.m13 = 0;
	rotationMatrix.m20 = 2 * (xz - yw);
	rotationMatrix.m21 = 2 * (yz + xw);
	rotationMatrix.m22 = 1 - 2 * (xSquared + ySquared);
	rotationMatrix.m23 = 0;
	rotationMatrix.m30 = 0;
	rotationMatrix.m31 = 0;
	rotationMatrix.m32 = 0;
	rotationMatrix.m33 = 1;

    return rotationMatrix;
}

Quaternion Quaternion::fromMatrix(Matrix4f* matrix)
{
    float w, x, y, z;
	float diagonal = matrix->m00 + matrix->m11 + matrix->m22;

	if (diagonal > 0)
    {
		float w4 = (float) (sqrtf(diagonal + 1.0f) * 2.0f);
		w = w4 / 4.0f;
		x = (matrix->m21 - matrix->m12) / w4;
		y = (matrix->m02 - matrix->m20) / w4;
		z = (matrix->m10 - matrix->m01) / w4;
	}
    else if ((matrix->m00 > matrix->m11) && (matrix->m00 > matrix->m22))
    {
		float x4 = (float) (sqrtf(1.0f + matrix->m00 - matrix->m11 - matrix->m22) * 2.0f);
		w = (matrix->m21 - matrix->m12) / x4;
		x = x4 / 4.0f;
		y = (matrix->m01 + matrix->m10) / x4;
		z = (matrix->m02 + matrix->m20) / x4;
	}
    else if (matrix->m11 > matrix->m22)
    {
		float y4 = (float) (sqrtf(1.0f + matrix->m11 - matrix->m00 - matrix->m22) * 2.0f);
		w = (matrix->m02 - matrix->m20) / y4;
		x = (matrix->m01 + matrix->m10) / y4;
		y = y4 / 4.0f;
		z = (matrix->m12 + matrix->m21) / y4;
	}
    else
    {
		float z4 = (float) (sqrtf(1.0f + matrix->m22 - matrix->m00 - matrix->m11) * 2.0f);
		w = (matrix->m10 - matrix->m01) / z4;
		x = (matrix->m02 + matrix->m20) / z4;
		y = (matrix->m12 + matrix->m21) / z4;
		z = z4 / 4.0f;
	}

	return Quaternion(x, y, z, w);
}

//https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
Quaternion Quaternion::fromEulerAngles(float yaw, float pitch, float roll)
{
    // Abbreviations for the various angular functions
    double cy = cos(yaw   * 0.5);
    double sy = sin(yaw   * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);
    double cr = cos(roll  * 0.5);
    double sr = sin(roll  * 0.5);

    Quaternion q;
    q.w = (float)(cr * cp * cy + sr * sp * sy);
    q.x = (float)(sr * cp * cy - cr * sp * sy);
    q.y = (float)(cr * sp * cy + sr * cp * sy);
    q.z = (float)(cr * cp * sy - sr * sp * cy);

    return q;
}

Quaternion Quaternion::interpolate(Quaternion* a, Quaternion* b, float blend)
{
    Quaternion result(0, 0, 0, 1);

	float dot = a->w * b->w + a->x * b->x + a->y * b->y + a->z * b->z;
	float blendI = 1.0f - blend;

	if (dot < 0)
    {
		result.w = blendI * a->w + blend * -b->w;
		result.x = blendI * a->x + blend * -b->x;
		result.y = blendI * a->y + blend * -b->y;
		result.z = blendI * a->z + blend * -b->z;
	}
    else
    {
		result.w = blendI * a->w + blend * b->w;
		result.x = blendI * a->x + blend * b->x;
		result.y = blendI * a->y + blend * b->y;
		result.z = blendI * a->z + blend * b->z;
	}

	result.normalize();
	return result;
}

Quaternion Quaternion::multiply(const Quaternion& qLeft, const Quaternion& qRight)
{
    float x =  qLeft.x * qRight.w + qLeft.y * qRight.z - qLeft.z * qRight.y + qLeft.w * qRight.x;
    float y = -qLeft.x * qRight.z + qLeft.y * qRight.w + qLeft.z * qRight.x + qLeft.w * qRight.y;
    float z =  qLeft.x * qRight.y - qLeft.y * qRight.x + qLeft.z * qRight.w + qLeft.w * qRight.z;
    float w = -qLeft.x * qRight.x - qLeft.y * qRight.y - qLeft.z * qRight.z + qLeft.w * qRight.w;

    return Quaternion(x, y, z, w);
}
