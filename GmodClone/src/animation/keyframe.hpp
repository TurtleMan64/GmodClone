#ifndef KEYFRAME_H
#define KEYFRAME_H

#include <string>
#include <vector>
#include <unordered_map>

#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"
#include "jointtransform.hpp"

class Keyframe
{
public:
    float timeStamp;
    std::unordered_map<std::string, JointTransform> pose;

    Keyframe();

    Keyframe(float timeStamp, std::unordered_map<std::string, JointTransform> jointKeyframes);
};
#endif
