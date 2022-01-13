#include <vector>

#include "animation.hpp"
#include "keyframe.hpp"

Animation::Animation(float lengthInSeconds, std::vector<Keyframe> keyframes)
{
    this->length = lengthInSeconds;
    this->keyframes = keyframes;
}
