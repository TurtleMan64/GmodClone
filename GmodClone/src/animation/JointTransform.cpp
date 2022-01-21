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
