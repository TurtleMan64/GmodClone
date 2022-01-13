#ifndef SKELETON_DATA_H
#define SKELETON_DATA_H

#include "jointdata.hpp"

class SkeletonData
{
public:
    int jointCount;
    JointData* headJoint = nullptr;

    SkeletonData();

    SkeletonData(int jointCount, JointData* headJoint);
};
#endif
