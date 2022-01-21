#ifndef JOINTTRANSFORM_H
#define JOINTTRANSFORM_H

#include <string>
#include <vector>

#include "../toolbox/vector.hpp"
#include "../toolbox/quaternion.hpp"

class Matrix4f;

class JointTransform
{
public:
    Vector3f position;
    Quaternion rotation;

    JointTransform();

    JointTransform(Vector3f position, Quaternion rotation);

    JointTransform(const JointTransform &other);

    void calculateLocalTransform(Matrix4f* mat, bool isRootJoint);

    static JointTransform interpolate(JointTransform* frameA, JointTransform* frameB, float progression);

    static Vector3f interpolate(Vector3f* start, Vector3f* end, float progression);
};
#endif
