#include "animatedmodeldata.hpp"
#include "meshdata.hpp"
#include "skeletondata.hpp"

AnimatedModelData::AnimatedModelData(MeshData mesh, SkeletonData joints)
{
    this->joints = joints;
    this->mesh = mesh;
}
