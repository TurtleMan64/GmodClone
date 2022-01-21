#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>

#include "keyframe.hpp"

class Animation
{
public:
    float length;
    std::vector<Keyframe> keyframes;
    //0 for looping, 1 for clamp
    int timeWrappingType;

    Animation();

    Animation(float lengthInSeconds, std::vector<Keyframe> keyframes, int timeWrappingType);

};
#endif
