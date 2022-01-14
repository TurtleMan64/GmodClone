#include <vector>

#include "keyframedata.hpp"
#include "animationdata.hpp"

AnimationData::AnimationData(float lengthSeconds, std::vector<KeyframeData> keyframes)
{
    this->lengthSeconds = lengthSeconds;
    this->keyframes = keyframes;
}
