#ifndef KEYFRAME_DATA_H
#define KEYFRAME_DATA_H

#include <vector>

#include "../toolbox/matrix.hpp"
#include "jointtransformdata.hpp"

class KeyframeData
{
public:
    float time;
    std::vector<JointTransformData> jointTransforms;

    KeyframeData(float time);

    void addJointTransform(JointTransformData transform);
};
#endif
