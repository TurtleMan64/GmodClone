#include <vector>

#include "animation.hpp"
#include "keyframe.hpp"

Animation::Animation()
{

}

Animation::Animation(float lengthInSeconds, std::vector<Keyframe> keyframes, int timeWrappingType)
{
    this->length = lengthInSeconds;
    this->keyframes = keyframes;
    this->timeWrappingType = timeWrappingType;
}
