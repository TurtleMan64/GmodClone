#include <vector>

#include "keyframedata.hpp"
#include "jointtransform.hpp"

KeyframeData::KeyframeData(float time)
{
    this->time = time;
}

void KeyframeData::addJointTransform(JointTransformData transform)
{
    jointTransforms.push_back(transform);
}
