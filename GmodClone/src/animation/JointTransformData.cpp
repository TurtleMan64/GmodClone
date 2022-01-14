#include <string>

#include "../toolbox/matrix.hpp"
#include "jointtransformdata.hpp"

JointTransformData::JointTransformData(std::string jointNameId, Matrix4f jointLocalTransform)
{
    this->jointNameId = jointNameId;
    this->jointLocalTransform = jointLocalTransform;
}
