#ifndef JOINT_TRANSFORM_DATA_H
#define JOINT_TRANSFORM_DATA_H

#include <string>

#include "../toolbox/matrix.hpp"

class JointTransformData
{
public:
    std::string jointNameId;
    Matrix4f jointLocalTransform;

    JointTransformData(std::string jointNameId, Matrix4f jointLocalTransform);
};
#endif
