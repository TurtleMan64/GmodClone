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
    // The local position (relative to our parent position)
    Vector3f position;

    // The local rotation (relative to our parent rotation)
    Quaternion rotation;

    JointTransform();

    JointTransform(Vector3f position, Quaternion rotation);

    JointTransform(const JointTransform &other);

    void calculateLocalTransform(Matrix4f* mat, bool isRootJoint);

    // Translates and rotates this joint to look in the direction and be located around a new origin.
    // This was designed to be used on the root joint, to move the model to the location of the player
    // and look in the direction that the player is going.
    void lookAtAndTranslate(Vector3f lookDir, Vector3f newOrigin);

    static JointTransform interpolate(JointTransform* frameA, JointTransform* frameB, float progression);

    static Vector3f interpolate(Vector3f* start, Vector3f* end, float progression);
};
#endif
