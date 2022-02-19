#include <string>

#include "jointtransform.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"
#include "../toolbox/quaternion.hpp"

JointTransform::JointTransform()
{

}

JointTransform::JointTransform(Vector3f position, Quaternion rotation)
{
    this->position = position;
    this->rotation = rotation;
}

JointTransform::JointTransform(const JointTransform &other)
{
    this->position = other.position;
    this->rotation = other.rotation;
}

void JointTransform::calculateLocalTransform(Matrix4f* localTransform, bool /*isRootJoint*/)
{
    //if (isRootJoint)
    //{
    //    Matrix4f positionMatrix;
    //    positionMatrix.translate(&position);
    //
    //    Matrix4f rotationMatrix = rotation.toRotationMatrix();
    //
    //    rotationMatrix.multiply(&positionMatrix, localTransform);
    //}
    //else
    {
        Matrix4f positionMatrix;
        positionMatrix.translate(&position);
    
        Matrix4f rotationMatrix = rotation.toRotationMatrix();
    
        positionMatrix.multiply(&rotationMatrix, localTransform);
    }
}

void JointTransform::lookAtAndTranslate(Vector3f lookDir, Vector3f newOrigin)
{
    float rotY = atan2f(lookDir.z, lookDir.x) - Maths::PI/2;
    float rotX = atan2f(lookDir.y, sqrtf(lookDir.x*lookDir.x + lookDir.z*lookDir.z));

    //printf("rotY = %f, position = %f %f %f\n", rotY, position.x, position.y, position.z);


    Vector3f yAxis(0, -1, 0);
    Vector3f xAxis(-1,  0, 0);

    //works
    Vector3f toAdd = position;
    toAdd = Maths::rotatePoint(&toAdd, &xAxis, rotX);
    toAdd = Maths::rotatePoint(&toAdd, &yAxis, rotY);
    position = toAdd;
    rotation = Quaternion::multiply(rotation, Quaternion::fromEulerAngles(0, 0, rotX));
    rotation = Quaternion::multiply(rotation, Quaternion::fromEulerAngles(0, rotY, 0));


    //hard coded 180 angle, works
    //rotate position by 180 degrees around y axis
    //position.x = -position.x;
    //position.z = -position.z;
    // rotate rotation by 180 degrees around y axis
    //rotation = Quaternion::multiply(rotation, Quaternion::fromEulerAngles(0, Maths::PI, 0));




    //position = Maths::rotatePoint(&position, &zAxis, rotZ);
    //position = Maths::rotatePoint(&position, &yAxis, rotY);
    position = position + newOrigin;
    //position.x += 10;

    //position = position + toAdd;

    //rotation = Quaternion::multiply(rotation, Quaternion::fromEulerAngles(0, 0, rotZ));
    //rotation = Quaternion::multiply(rotation, Quaternion::fromEulerAngles(0, rotY - Maths::PI/2, 0));
}

JointTransform JointTransform::interpolate(JointTransform* frameA, JointTransform* frameB, float progression)
{
    Vector3f pos = JointTransform::interpolate(&frameA->position, &frameB->position, progression);
    Quaternion rot = Quaternion::interpolate(&frameA->rotation, &frameB->rotation, progression);
    return JointTransform(pos, rot);
}

Vector3f JointTransform::interpolate(Vector3f* start, Vector3f* end, float progression)
{
    float x = start->x + (end->x - start->x) * progression;
	float y = start->y + (end->y - start->y) * progression;
	float z = start->z + (end->z - start->z) * progression;
	return Vector3f(x, y, z);
}
