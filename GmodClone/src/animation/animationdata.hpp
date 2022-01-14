#ifndef ANIMATION_DATA_H
#define ANIMATION_DATA_H

#include <vector>

#include "keyframedata.hpp"

class AnimationData
{
public:
    float lengthSeconds;
    std::vector<KeyframeData> keyframes;

    AnimationData(float lengthSeconds, std::vector<KeyframeData> keyframes);
};
#endif
