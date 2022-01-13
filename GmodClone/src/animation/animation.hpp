#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>

#include "keyframe.hpp"

class Animation
{
public:
    float length;
    std::vector<Keyframe> keyframes;

    Animation(float lengthInSeconds, std::vector<Keyframe> keyframes);

};
#endif
