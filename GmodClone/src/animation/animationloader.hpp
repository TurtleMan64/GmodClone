#ifndef ANIMATION_LOADER_H
#define ANIMATION_LOADER_H

#include "animatedmodel.hpp"
#include "meshdata.hpp"
#include "jointdata.hpp"
#include "joint.hpp"
#include "skeletondata.hpp"
#include "keyframe.hpp"
#include "jointtransformdata.hpp"
#include "keyframedata.hpp"

class AnimationLoader
{
public:
    static Animation* loadAnimation(const char* filename);

    static Animation* loadAnimation(char* filename);
};
#endif
