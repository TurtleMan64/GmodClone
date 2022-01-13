#ifndef ANIMATED_MODEL_DATA_H
#define ANIMATED_MODEL_DATA_H

#include "skeletondata.hpp"
#include "meshdata.hpp"

class AnimatedModelData
{
public:
    SkeletonData joints;
    MeshData mesh;

    AnimatedModelData(MeshData mesh, SkeletonData joints);
};
#endif
