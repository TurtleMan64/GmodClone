#ifndef ANIMATED_MODEL_LOADER_H
#define ANIMATED_MODEL_LOADER_H

#include "animatedmodel.hpp"
#include "meshdata.hpp"
#include "jointdata.hpp"
#include "joint.hpp"
#include "skeletondata.hpp"
#include "../openglObjects/vao.hpp"

class AnimatedModelLoader
{
public:
    static AnimatedModel* loadAnimatedModel(const char* folder, const char* filename);

    static AnimatedModel* loadAnimatedModel(char* folder, char* filename);

    static Vao* createVao(MeshData* data);
};
#endif
