#include "skeletondata.hpp"

SkeletonData::SkeletonData()
{

}

SkeletonData::SkeletonData(int jointCount, JointData* headJoint)
{
    this->jointCount = jointCount;
    this->headJoint = headJoint;
}
