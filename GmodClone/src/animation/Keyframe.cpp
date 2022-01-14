#include <string>
#include <unordered_map>

#include "keyframe.hpp"
#include "jointtransform.hpp"

Keyframe::Keyframe()
{

}

Keyframe::Keyframe(float timeStamp, std::unordered_map<std::string, JointTransform> jointKeyframes)
{
    this->timeStamp = timeStamp;
    this->pose = jointKeyframes;
}
