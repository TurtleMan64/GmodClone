#ifndef ANIMATED_MODEL_RENDERER_H
#define ANIMATED_MODEL_RENDERER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"

class AnimatedModelShader;
class AnimatedModel;

class AnimatedModelRenderer
{
public:
    AnimatedModelShader* shader = nullptr;

    AnimatedModelRenderer();

    void render(AnimatedModel* entity);

    void cleanUp();

    void prepare();

    void finish();
};
#endif
