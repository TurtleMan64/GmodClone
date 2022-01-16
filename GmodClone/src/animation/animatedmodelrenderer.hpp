#ifndef ANIMATED_MODEL_RENDERER_H
#define ANIMATED_MODEL_RENDERER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <unordered_map>

#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"

class AnimatedModelShader;
class AnimatedModel;
class Entity;

class AnimatedModelRenderer
{
public:
    AnimatedModelShader* shader = nullptr;

    AnimatedModelRenderer();

    void render(std::unordered_map<AnimatedModel*, std::vector<Entity*>>* animatedEntitiesMap);

    void cleanUp();

    void prepare();

    void finish();
};
#endif
